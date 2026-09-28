"""Locate low-energy intervals near stable-port diphone joins.

This is diagnostic evidence, not a proof that a join caused a gap. The log is
produced by NicolaiTalker --render and the original/portable WAVs stay local.
"""
from __future__ import annotations

import argparse
from array import array
import json
import math
from pathlib import Path
import sys
import wave


def read_pcm(path: Path) -> tuple[int, array]:
    with wave.open(str(path), "rb") as wav:
        if (wav.getnchannels(), wav.getsampwidth()) != (1, 2):
            raise ValueError(f"{path}: expected mono PCM16")
        rate = wav.getframerate()
        samples = array("h")
        samples.frombytes(wav.readframes(wav.getnframes()))
    if sys.byteorder != "little":
        samples.byteswap()
    if rate <= 0 or not samples:
        raise ValueError(f"{path}: empty or invalid audio")
    return rate, samples


def low_energy_intervals(samples: array, rate: int, minimum_ms: int = 30) -> list[tuple[int, int]]:
    """Return intervals within speech, using the same gate rule for both WAVs."""
    frame = max(1, rate // 100)  # 10 ms at 16 kHz
    peak = max(abs(value) for value in samples)
    active_gate = max(32, peak * 0.005)
    active = [i for i, value in enumerate(samples) if abs(value) > active_gate]
    if not active:
        return []
    first = active[0] // frame
    last = min(len(samples) // frame, (active[-1] + frame) // frame)
    levels = []
    for begin in range(0, len(samples) - frame + 1, frame):
        chunk = samples[begin:begin + frame]
        levels.append(math.sqrt(sum(value * value for value in chunk) / frame))
    threshold = max(60.0, max(levels) * 0.02)
    runs = []
    start = None
    for index in range(first, last + 1):
        quiet = index < last and levels[index] < threshold
        if quiet and start is None:
            start = index
        if not quiet and start is not None:
            if (index - start) * frame * 1000 >= minimum_ms * rate:
                runs.append((start * frame, index * frame))
            start = None
    return runs


def parse_joins(log: str, sample_count: int) -> list[dict]:
    joins = []
    for line in log.splitlines():
        if not line.startswith("join="):
            continue
        fields = line[5:].split(",")
        if len(fields) != 7:
            raise ValueError(f"malformed join telemetry: {line}")
        index, phone, center, overlap, left_trim, right_trim = fields[:6]
        entry = {
            "phone_index": int(index),
            "phone": phone,
            "center_sample": int(center),
            "overlap_samples": int(overlap),
            "left_trim": int(left_trim),
            "right_trim": int(right_trim),
            "correlation": float(fields[6]),
        }
        if not (0 <= entry["center_sample"] < sample_count) or not math.isfinite(entry["correlation"]):
            raise ValueError("join coordinate or correlation invalid")
        if joins and entry["center_sample"] < joins[-1]["center_sample"]:
            raise ValueError("join coordinates are not monotone")
        joins.append(entry)
    if not joins:
        raise ValueError("stable-port join telemetry not found")
    return joins


def summarize(rate: int, samples: array, joins: list[dict] | None = None) -> dict:
    gaps = []
    for begin, end in low_energy_intervals(samples, rate):
        row = {
            "start_ms": round(1000 * begin / rate, 1),
            "end_ms": round(1000 * end / rate, 1),
            "duration_ms": round(1000 * (end - begin) / rate, 1),
        }
        if joins is not None:
            def distance(join: dict) -> int:
                center = join["center_sample"]
                return max(begin - center, 0, center - end)
            closest = min(joins, key=distance)
            row["nearest_join"] = {
                "phone_index": closest["phone_index"],
                "phone": closest["phone"],
                "distance_ms": round(1000 * distance(closest) / rate, 1),
                "correlation": closest["correlation"],
            }
        gaps.append(row)
    return {
        "duration_ms": round(1000 * len(samples) / rate, 1),
        "low_energy_interval_count": len(gaps),
        "low_energy_total_ms": round(sum(gap["duration_ms"] for gap in gaps), 1),
        "gaps": gaps,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original", type=Path, required=True)
    parser.add_argument("--portable", type=Path, required=True)
    parser.add_argument("--join-log", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError("refusing to overwrite an existing audit")
    original_rate, original = read_pcm(args.original)
    portable_rate, portable = read_pcm(args.portable)
    if original_rate != portable_rate:
        raise ValueError("original/portable sample rates differ")
    joins = parse_joins(args.join_log.read_text(encoding="utf-8"), len(portable))
    reference = summarize(original_rate, original)
    candidate = summarize(portable_rate, portable, joins)
    result = {
        "schema": "nicolai-talker-join-audit-v1",
        "sample_rate": original_rate,
        "method": "10-ms RMS; quiet below max(60 PCM units, 2% of maximum frame RMS); within active samples above max(32 PCM units, 0.5% peak); intervals >=30 ms",
        "caveat": "Small, unaligned phrases; low energy is not necessarily a defect or proof of join causality. Phone-boundary original capture is still needed.",
        "original": reference,
        "stable_port": candidate,
        "join_count": len(joins),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(json.dumps({"original_gaps": reference["low_energy_interval_count"],
                      "port_gaps": candidate["low_energy_interval_count"],
                      "join_count": len(joins)}))


if __name__ == "__main__":
    main()
