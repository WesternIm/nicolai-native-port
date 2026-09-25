#!/usr/bin/env python3
"""Compare two Nicolai parity JSON reports phrase by phrase."""
from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path


METRICS = {
    "active_correlation": True,
    "active_duration_error": False,
    "total_duration_error": False,
    "f0_contour_relative_mae": False,
    "mfcc_dtw": False,
    "rms_ratio_error": False,
}


def load(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("baseline", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("--json", dest="json_out", type=Path)
    parser.add_argument("--csv", dest="csv_out", type=Path)
    args = parser.parse_args()

    baseline = load(args.baseline)
    candidate = load(args.candidate)
    baseline_rows = {row["id"]: row for row in baseline["rows"] if row["status"] == "ok"}
    candidate_rows = {row["id"]: row for row in candidate["rows"] if row["status"] == "ok"}
    phrase_ids = sorted(set(baseline_rows) & set(candidate_rows))

    rows: list[dict[str, object]] = []
    metric_summary: dict[str, dict[str, object]] = {}
    for metric, higher_is_better in METRICS.items():
        comparisons: list[tuple[str, float]] = []
        for phrase_id in phrase_ids:
            delta = float(candidate_rows[phrase_id][metric]) - float(baseline_rows[phrase_id][metric])
            comparisons.append((phrase_id, delta if higher_is_better else -delta))
        improved = sum(delta > 1e-12 for _, delta in comparisons)
        regressed = sum(delta < -1e-12 for _, delta in comparisons)
        tied = len(comparisons) - improved - regressed
        best_id, best_delta = max(comparisons, key=lambda item: item[1])
        worst_id, worst_delta = min(comparisons, key=lambda item: item[1])
        metric_summary[metric] = {
            "higher_is_better": higher_is_better,
            "improved": improved,
            "regressed": regressed,
            "tied": tied,
            "best_phrase": best_id,
            "best_directional_delta": best_delta,
            "worst_phrase": worst_id,
            "worst_directional_delta": worst_delta,
        }

    for phrase_id in phrase_ids:
        base = baseline_rows[phrase_id]
        cand = candidate_rows[phrase_id]
        row: dict[str, object] = {"id": phrase_id, "text": base.get("text", "")}
        for metric, higher_is_better in METRICS.items():
            delta = float(cand[metric]) - float(base[metric])
            row[f"{metric}_delta"] = delta
            row[f"{metric}_improved"] = delta > 1e-12 if higher_is_better else delta < -1e-12
        rows.append(row)

    result = {
        "baseline": str(args.baseline),
        "candidate": str(args.candidate),
        "phrases": len(phrase_ids),
        "metrics": metric_summary,
        "rows": rows,
    }
    rendered = json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False)
    print(rendered)
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(rendered + "\n", encoding="utf-8")
    if args.csv_out:
        args.csv_out.parent.mkdir(parents=True, exist_ok=True)
        with args.csv_out.open("w", newline="", encoding="utf-8-sig") as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]) if rows else ["id"])
            writer.writeheader()
            writer.writerows(rows)


if __name__ == "__main__":
    main()
