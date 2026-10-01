"""Screen pitch-period discontinuities around native diphone joins.

JP records are authored SEG-derived hints, not measured F0. PCM estimates
use a conservative local autocorrelation gate. Original positions use MFCC
DTW, not phone labels: this cannot prove perceived intonation or causality.
Only scalar diagnostics are exported; private audio remains local.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np

from audit_talker_pair_alignment import HOP, SR, align, read_joins
from measure_parity import read_pcm


def cents_gap(left: float | None, right: float | None) -> float | None:
    if left is None or right is None or left <= 0 or right <= 0:
        return None
    if not math.isfinite(left) or not math.isfinite(right):
        return None
    return float(abs(1200 * math.log2(left / right)))


def estimate_period(samples: np.ndarray) -> dict:
    """50..300 Hz; 40 ms minimum; RMS >=100 and periodic correlation >=0.75."""
    missing = {"period": None, "confidence": None}
    x = np.asarray(samples, dtype=np.float64)
    if x.ndim != 1 or len(x) < SR // 25 or not np.all(np.isfinite(x)):
        return missing
    x = x - x.mean()
    if np.sqrt(np.mean(x * x)) < 100:
        return missing
    lags = np.arange(math.ceil(SR / 300), math.floor(SR / 50) + 1)
    scores = []
    for lag in lags:
        a, b = x[:-lag], x[lag:]
        a, b = a - a.mean(), b - b.mean()
        denominator = np.linalg.norm(a) * np.linalg.norm(b)
        scores.append(float(np.dot(a, b) / denominator) if denominator > 0 else 0)
    scores = np.asarray(scores)
    peaks = [i for i in range(1, len(scores) - 1)
             if scores[i] > scores[i - 1] and scores[i] >= scores[i + 1]]
    if not peaks:
        return missing
    best = max(scores[i] for i in peaks)
    if best < 0.75:
        return missing
    # Prefer the shortest equally strong recurrence, not a doubled period.
    index = next(i for i in peaks if scores[i] >= best - 0.04)
    a, b, c = scores[index - 1:index + 2]
    denominator = a - 2 * b + c
    delta = float(np.clip(0.5 * (a - c) / denominator, -0.5, 0.5)) if abs(denominator) > 1e-12 else 0.0
    return {"period": float(lags[index] + delta), "confidence": float(scores[index])}


def side_periods(pcm: np.ndarray, center: int, guard: int) -> dict:
    window = SR // 25
    left = estimate_period(pcm[max(0, center - guard - window):max(0, center - guard)])
    right = estimate_period(pcm[min(len(pcm), center + guard):min(len(pcm), center + guard + window)])
    return {"left": left, "right": right,
            "gap_cents": cents_gap(left["period"], right["period"])}


def parse_periods(log: str) -> dict[tuple[str, int], dict]:
    result = {}
    for line in log.splitlines():
        if not line.startswith("JP\t"):
            continue
        fields = line.split("\t")
        if len(fields) != 6:
            raise ValueError("malformed JP record")
        _, phrase, index, phone, left, right = fields
        key = (phrase, int(index))
        a, b = float(left), float(right)
        if key in result or int(index) < 1 or not phrase or not phone or any(
            not math.isfinite(value) or value < 0 for value in (a, b)
        ):
            raise ValueError("invalid/duplicate JP record")
        result[key] = {"phone": phone, "left": a, "right": b,
                       "gap_cents": cents_gap(a, b)}
    if not result:
        raise ValueError("no JP records; enable NICOLAI_AUDIT_JOIN_PERIODS=1")
    return result


def summarize(rows: list[dict]) -> dict:
    def group(values):
        valid = [x for x in values if x is not None]
        return {"eligible": len(valid), "missing": len(values) - len(valid),
                "mean_gap_cents": float(np.mean(valid)) if valid else None,
                "over_100_cents": sum(x > 100 for x in valid),
                "over_200_cents": sum(x > 200 for x in valid)}
    result = {"joins": len(rows), "phrases": len({row["id"] for row in rows})}
    for field in ("authored", "portable", "original_dtw"):
        result[field] = group([row[field]["gap_cents"] if field in row else None for row in rows])
    return result


def matched_comparison(before: list[dict], after: list[dict]) -> dict:
    def keyed(rows):
        result = {(row["id"], row["phone_index"]): row for row in rows}
        if len(result) != len(rows):
            raise ValueError("duplicate join in paired reports")
        return result
    baseline, trial = keyed(before), keyed(after)
    if baseline.keys() != trial.keys():
        raise ValueError("paired reports have different join sets")
    output = {}
    for field in ("authored", "portable"):
        values = []
        for key, a in baseline.items():
            b = trial[key]
            if a["phone"] != b["phone"]:
                raise ValueError("paired phone labels differ")
            x, y = a[field]["gap_cents"], b[field]["gap_cents"]
            if x is not None and y is not None:
                values.append((x, y))
        output[field] = {
            "matched_eligible": len(values), "excluded": len(before) - len(values),
            "baseline_mean_gap_cents": float(np.mean([x for x, y in values])) if values else None,
            "trial_mean_gap_cents": float(np.mean([y for x, y in values])) if values else None,
            "lower_gap": sum(y < x - 1 for x, y in values),
            "higher_gap": sum(y > x + 1 for x, y in values),
        }
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--portable", type=Path, required=True)
    parser.add_argument("--join-log", type=Path, required=True)
    parser.add_argument("--original", type=Path)
    parser.add_argument("--baseline-audit", type=Path, help="compare only mutually eligible join estimates")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error("refusing to overwrite an existing audit")
    log = args.join_log.read_text(encoding="utf-8")
    periods = parse_periods(log)
    rows = []
    for phrase in sorted({key[0] for key in periods}):
        rate, pcm = read_pcm(args.portable / f"{phrase}.wav")
        if rate != SR:
            raise ValueError("expected mono PCM16 at 16 kHz")
        joins = read_joins(log, len(pcm), phrase)
        keys = {(phrase, join["phone_index"]) for join in joins}
        if keys != {key for key in periods if key[0] == phrase}:
            raise ValueError("J/JP record sets differ")
        alignment, original = None, None
        if args.original:
            original_rate, original = read_pcm(args.original / f"{phrase}.wav")
            if original_rate != SR:
                raise ValueError("original sample rate differs")
            alignment = align(original, pcm)
        for join in joins:
            authored = periods[(phrase, join["phone_index"])]
            if authored["phone"] != join["phone"]:
                raise ValueError("J/JP phone labels differ")
            center = join["center_sample"]
            row = {"id": phrase, "phone_index": join["phone_index"], "phone": join["phone"],
                   "center_sample": center, "correlation": join["correlation"],
                   "authored": authored,
                   "portable": side_periods(pcm, center, max(80, join["overlap_samples"] // 2 + 32))}
            if alignment is not None:
                frame = round((center - alignment["portable_bounds"][0]) / HOP)
                path = alignment["path"]
                nearest = np.abs(path[:, 1] - frame)
                mapped = round(float(np.median(path[nearest == nearest.min(), 0])))
                original_center = alignment["reference_bounds"][0] + mapped * HOP
                row["original_dtw"] = {"center_sample": original_center,
                                       **side_periods(original, original_center, 80)}
            rows.append(row)
    report = {"schema": "nicolai-join-period-screen-m42-v1",
              "caveat": "Hints are not measured F0. Autocorrelation can confuse harmonics; missing tracks are reported. Original DTW positions are not phone-aligned ground truth or a listening score.",
              "summary": summarize(rows), "rows": rows}
    if args.baseline_audit:
        previous = json.loads(args.baseline_audit.read_text(encoding="utf-8"))
        report["matched_comparison"] = matched_comparison(previous["rows"], rows)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(report, stream, indent=2, allow_nan=False)
        stream.write("\n")
    print(json.dumps(report["summary"], indent=2))
    if "matched_comparison" in report:
        print(json.dumps(report["matched_comparison"], indent=2))


if __name__ == "__main__":
    main()
