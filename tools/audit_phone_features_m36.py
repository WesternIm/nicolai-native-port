#!/usr/bin/env python3
"""Audit captured 0x101a2780 duration records without proprietary inputs.

The accepted JSON is either a list of records or {"records": [...]}. Each record
must contain:
  id, previous_split, previous_last, next_first, next_split,
  feature_duration_sum, observed_duration_q11
and may contain observed_support.

Only behavior already supported by reverse evidence is checked here. Pitch-anchor
repair, voicing and terminal next-left ownership stay intentionally unmodeled
until original runtime records prove their exact rules.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def expected_duration_record(record: dict[str, Any]) -> dict[str, int]:
    previous_split = int(record["previous_split"])
    previous_last = int(record["previous_last"])
    next_first = int(record["next_first"])
    next_split = int(record["next_split"])
    feature_duration_sum = int(record["feature_duration_sum"])

    previous_right = previous_last - previous_split
    next_left = next_split - next_first
    support = previous_right + next_left
    if previous_right < 0 or next_left < 0 or support <= 0:
        raise ValueError("invalid source-support geometry")
    if feature_duration_sum <= 0:
        raise ValueError("feature_duration_sum must be positive")

    duration_q11 = (feature_duration_sum * 2048) // support
    if duration_q11 == 0:
        duration_q11 = 1
    return {
        "previous_right_support": previous_right,
        "next_left_support": next_left,
        "combined_source_support": support,
        "duration_q11": duration_q11,
    }


def audit_records(records: list[dict[str, Any]]) -> dict[str, Any]:
    mismatches: list[dict[str, Any]] = []
    invalid: list[dict[str, Any]] = []
    for index, record in enumerate(records):
        record_id = str(record.get("id", index))
        try:
            expected = expected_duration_record(record)
            observed_duration = int(record["observed_duration_q11"])
            observed_support = record.get("observed_support")
        except (KeyError, TypeError, ValueError) as exc:
            invalid.append({"id": record_id, "error": str(exc)})
            continue

        reasons: list[str] = []
        if observed_duration != expected["duration_q11"]:
            reasons.append("duration_q11")
        if observed_support is not None and int(observed_support) != expected["combined_source_support"]:
            reasons.append("combined_source_support")
        if reasons:
            mismatches.append({
                "id": record_id,
                "reasons": reasons,
                "expected": expected,
                "observed_duration_q11": observed_duration,
                "observed_support": observed_support,
            })

    return {
        "schema": "m36-phone-duration-audit-v1",
        "records": len(records),
        "matched": len(records) - len(mismatches) - len(invalid),
        "mismatched": len(mismatches),
        "invalid": len(invalid),
        "mismatches": mismatches,
        "invalid_records": invalid,
        "scope": {
            "proven": [
                "previous-right + next-left source support",
                "positive total feature duration -> Q11 coefficient",
                "zero positive quotient fallback -> 1",
            ],
            "not_modeled": [
                "pitch anchor repair",
                "voicing lane construction",
                "terminal next-left interval adjustment",
                "descriptor packaging and runtime rollback",
            ],
        },
    }


def load_records(path: Path) -> list[dict[str, Any]]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if isinstance(payload, dict):
        payload = payload.get("records")
    if not isinstance(payload, list) or not all(isinstance(x, dict) for x in payload):
        raise ValueError("input must be a JSON record list or an object containing records[]")
    return payload


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path, help="captured feature records JSON")
    parser.add_argument("--output", type=Path, help="write report JSON instead of stdout only")
    args = parser.parse_args()

    try:
        report = audit_records(load_records(args.input))
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        parser.error(str(exc))

    text = json.dumps(report, indent=2, sort_keys=True)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text + "\n", encoding="utf-8")
    print(text)
    return 0 if report["mismatched"] == 0 and report["invalid"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
