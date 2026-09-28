"""Align a local original/port WAV pair and inspect port low-energy intervals.

Requires tools/requirements-parity.txt. Results are diagnostics, not original
phone labels or proof that a join caused a perceived break. Raw WAVs and text
stay local; the output JSON contains only positions, scalar values and hashes.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import librosa
import numpy as np

from audit_talker_joins import low_energy_intervals, parse_joins, summarize
from measure_parity import active_bounds, read_pcm


SR = 16000
HOP = 160


def features(samples: np.ndarray) -> tuple[np.ndarray, tuple[int, int]]:
    bounds = active_bounds(samples, SR)
    if bounds[1] <= bounds[0]:
        raise ValueError("no active audio")
    audio = samples[bounds[0]:bounds[1]] / 32768.0
    rms = float(np.sqrt(np.mean(audio * audio)))
    if rms <= 0:
        raise ValueError("silent audio")
    shape = librosa.feature.mfcc(
        y=(audio / rms).astype(np.float32), sr=SR, n_mfcc=13,
        n_fft=512, hop_length=HOP, center=True,
    )[1:]
    return shape, bounds


def align(reference: np.ndarray, portable: np.ndarray) -> dict:
    reference_shape, reference_bounds = features(reference)
    portable_shape, portable_bounds = features(portable)
    cost, reverse_path = librosa.sequence.dtw(
        X=reference_shape, Y=portable_shape, metric="euclidean",
        global_constraints=True, band_rad=0.20,
    )
    path = reverse_path[::-1]
    if tuple(path[0]) != (0, 0) or tuple(path[-1]) != (
        reference_shape.shape[1] - 1, portable_shape.shape[1] - 1
    ):
        raise ValueError("incomplete DTW path")
    return {
        "path": path,
        "reference_bounds": reference_bounds,
        "portable_bounds": portable_bounds,
        "mean_shape_cost": float(cost[-1, -1] / len(path)),
        "path_stretch": float(len(path) / max(reference_shape.shape[1], portable_shape.shape[1])),
    }


def reference_quiet_ms(reference: np.ndarray, begin: int, end: int) -> float:
    """Apply the M37 quiet rule to 10-ms frames within a mapped reference span."""
    frame = HOP
    last = len(reference) // frame
    levels = np.sqrt(np.mean(reference[:last * frame].reshape(last, frame) ** 2, axis=1))
    gate = max(60.0, float(np.max(levels)) * 0.02)
    first_frame = max(0, begin // frame)
    last_frame = min(last, (end + frame - 1) // frame)
    frames = np.arange(first_frame, last_frame)
    covered = np.maximum(0, np.minimum(end, (frames + 1) * frame)
                         - np.maximum(begin, frames * frame))
    return round(float(np.sum(covered[levels[first_frame:last_frame] < gate]) * 1000 / SR), 1)


def inspect_gap(
    reference: np.ndarray, alignment: dict,
    begin: int, end: int,
) -> dict:
    path = alignment["path"]
    reference_begin = alignment["reference_bounds"][0]
    portable_begin = alignment["portable_bounds"][0]
    portable_first = max(0, (begin - portable_begin) // HOP)
    portable_last = min(int(path[-1, 1]), (end - portable_begin + HOP - 1) // HOP)
    selected = path[(path[:, 1] >= portable_first) & (path[:, 1] <= portable_last)]
    if not len(selected):
        return {"aligned": False}
    mapped_begin = max(0, reference_begin + int(np.min(selected[:, 0])) * HOP)
    mapped_end = min(len(reference), reference_begin + (int(np.max(selected[:, 0])) + 1) * HOP)
    quiet_ms = reference_quiet_ms(reference, mapped_begin, mapped_end)
    return {
        "aligned": True,
        "reference_start_ms": round(1000 * mapped_begin / SR, 1),
        "reference_end_ms": round(1000 * mapped_end / SR, 1),
        "reference_span_ms": round(1000 * (mapped_end - mapped_begin) / SR, 1),
        "reference_quiet_ms_in_span": quiet_ms,
        "portable_quiet_ms": round(1000 * (end - begin) / SR, 1),
        "reference_frames_on_path": int(len(np.unique(selected[:, 0]))),
        "portable_frames_on_path": int(len(np.unique(selected[:, 1]))),
    }


def read_joins(log: str, sample_count: int, phrase_id: str | None) -> list[dict]:
    if phrase_id is None:
        return parse_joins(log, sample_count)
    converted = []
    for line in log.splitlines():
        fields = line.split("\t")
        if len(fields) == 9 and fields[:2] == ["J", phrase_id]:
            converted.append("join=" + ",".join(fields[2:]))
    return parse_joins("\n".join(converted), sample_count)


def audit(original_path: Path, port_path: Path, join_log: Path,
          phrase_id: str | None = None) -> dict:
    original_rate, original = read_pcm(original_path)
    port_rate, port = read_pcm(port_path)
    if (original_rate, port_rate) != (SR, SR):
        raise ValueError("expected 16-kHz original and portable WAVs")
    joins = read_joins(join_log.read_text(encoding="utf-8"), len(port), phrase_id)
    aligned = align(original, port)
    port_gaps = low_energy_intervals(port, SR)
    gap_rows = []
    for begin, end in port_gaps:
        row = {
            "portable_start_ms": round(1000 * begin / SR, 1),
            "portable_end_ms": round(1000 * end / SR, 1),
            **inspect_gap(original, aligned, begin, end),
        }
        nearest = min(joins, key=lambda j: max(begin - j["center_sample"], 0, j["center_sample"] - end))
        row["nearest_join"] = {
            "phone_index": nearest["phone_index"],
            "phone": nearest["phone"],
            "distance_ms": round(1000 * max(begin - nearest["center_sample"], 0,
                                            nearest["center_sample"] - end) / SR, 1),
        }
        gap_rows.append(row)
    return {
        "schema": "nicolai-talker-pair-alignment-v1",
        "method": "RMS-normalized MFCC 1..12; 10-ms hop; 20% constrained DTW. Quiet rule from M37: 10-ms RMS below max(60 PCM units, 2% peak frame RMS).",
        "caveat": "DTW may align wrong phones or compress a silence. Small unannotated pairs cannot establish join causality, pitch truth, or perceptual improvement.",
        "original_sha256": hashlib.sha256(original_path.read_bytes()).hexdigest(),
        "portable_sha256": hashlib.sha256(port_path.read_bytes()).hexdigest(),
        "original": {key: value for key, value in summarize(SR, original).items() if key != "gaps"},
        "stable_port": {key: value for key, value in summarize(SR, port, joins).items() if key != "gaps"},
        "mean_shape_cost": round(aligned["mean_shape_cost"], 3),
        "path_stretch": round(aligned["path_stretch"], 3),
        "portable_gaps": gap_rows,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original", type=Path, required=True)
    parser.add_argument("--portable", type=Path, required=True)
    parser.add_argument("--join-log", type=Path, required=True)
    parser.add_argument("--phrase-id", help="select J records from a batch render log")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError("refusing to overwrite existing audit")
    result = audit(args.original, args.portable, args.join_log, args.phrase_id)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(json.dumps({"gaps": len(result["portable_gaps"]),
                      "mean_shape_cost": result["mean_shape_cost"]}))


if __name__ == "__main__":
    main()
