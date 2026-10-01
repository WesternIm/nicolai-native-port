"""Paired rhythm diagnostics. Quiet intervals and DTW are not phone labels.

Exports scalar positions/statistics only; originals and rendered WAVs stay
local. Separates quiet near # joins (including punctuation) from other gaps.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

from audit_talker_pair_alignment import audit


def gap_summary(report: dict) -> dict:
    word = [row for row in report["portable_gaps"]
            if row["nearest_join"]["phone"] == "#" and row["nearest_join"]["distance_ms"] == 0]
    aligned = [row for row in word if row.get("aligned")]
    return {
        "word_join_intervals": len(word),
        "word_join_mapped_intervals": len(aligned),
        "word_join_unmapped_intervals": len(word) - len(aligned),
        "word_join_excess_quiet_ms": sum(max(0, row["portable_quiet_ms"] - row["reference_quiet_ms_in_span"]) for row in aligned),
        "quiet_total_ms": report["stable_port"]["low_energy_total_ms"],
        "original_quiet_total_ms": report["original"]["low_energy_total_ms"],
        "duration_error_ms": report["stable_port"]["duration_ms"] - report["original"]["duration_ms"],
        "shape_cost": report["mean_shape_cost"],
    }


def read_budgets(log: str) -> dict:
    words = []
    seen = set()
    for line in log.splitlines():
        if not line.startswith("WT\t"):
            continue
        fields = line.split("\t")
        if len(fields) != 7:
            raise ValueError("malformed WT record")
        _, phrase, first, last, before, after, strength = fields
        first, last = int(first), int(last)
        before, after, strength = float(before), float(after), float(strength)
        key = (phrase, first, last)
        if not phrase or first < 0 or first >= last or key in seen or not all(
                math.isfinite(value) for value in (before, after, strength)):
            raise ValueError("invalid/duplicate WT record")
        if before <= 0 or after <= 0 or not 0 < strength <= 1:
            raise ValueError("invalid WT budget/strength")
        seen.add(key)
        words.append({"id": phrase, "first_phone": first, "last_phone": last,
                      "baseline_samples": before, "trial_samples": after,
                      "effective_strength": strength})
    return {"words": len(words), "phrases": len({row["id"] for row in words}),
            "max_logged_budget_delta_samples": max((abs(row["trial_samples"] - row["baseline_samples"]) for row in words), default=0),
            "minimum_effective_strength": min((row["effective_strength"] for row in words), default=None),
            "maximum_effective_strength": max((row["effective_strength"] for row in words), default=None)}


def summarize(rows: list[dict]) -> dict:
    if not rows:
        raise ValueError("empty rhythm comparison")
    result = {"phrases": len(rows)}
    for profile in ("baseline", "trial"):
        result[profile] = {key: sum(row[profile][key] for row in rows) for key in (
            "word_join_intervals", "word_join_mapped_intervals", "word_join_unmapped_intervals",
            "word_join_excess_quiet_ms", "quiet_total_ms", "original_quiet_total_ms")}
        result[profile]["duration_mae_ms"] = sum(abs(row[profile]["duration_error_ms"]) for row in rows) / len(rows)
        result[profile]["mean_shape_cost"] = sum(row[profile]["shape_cost"] for row in rows) / len(rows)
    result["lower_shape_cost"] = sum(row["trial"]["shape_cost"] < row["baseline"]["shape_cost"] for row in rows)
    result["higher_shape_cost"] = sum(row["trial"]["shape_cost"] > row["baseline"]["shape_cost"] for row in rows)
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("original", "baseline", "trial", "baseline-log", "trial-log", "corpus", "output"):
        parser.add_argument("--" + name, type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error("refusing to overwrite an existing report")
    names = [line.split("\t", 1)[0] for line in args.corpus.read_text(encoding="utf-8-sig").splitlines() if "\t" in line]
    if not names or len(names) != len(set(names)):
        raise ValueError("empty/duplicate corpus IDs")
    rows = []
    for name in names:
        row = {"id": name}
        for profile in ("baseline", "trial"):
            folder = getattr(args, profile)
            log = getattr(args, profile + "_log")
            report = audit(args.original / (name + ".wav"), folder / (name + ".wav"), log, name)
            row[profile] = gap_summary(report)
        rows.append(row)
    report = {"schema": "nicolai-rhythm-screen-m43-v1",
              "caveat": "Low energy is not necessarily missing speech; DTW may align wrong phones. # gap grouping includes punctuation, not only plain word boundaries. Budget conservation is before waveform joins; none of these values prove listening superiority.",
              "summary": summarize(rows), "word_budgets": read_budgets(args.trial_log.read_text(encoding="utf-8")), "rows": rows}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(report, stream, indent=2, allow_nan=False)
        stream.write("\n")
    print(json.dumps({"summary": report["summary"], "word_budgets": report["word_budgets"]}, indent=2))


if __name__ == "__main__":
    main()
