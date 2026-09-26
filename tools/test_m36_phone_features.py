#!/usr/bin/env python3

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import audit_phone_features_m36 as audit  # noqa: E402


def main() -> int:
    one = audit.expected_duration_record({
        "previous_split": 80,
        "previous_last": 120,
        "next_first": 200,
        "next_split": 260,
        "feature_duration_sum": 100,
    })
    assert one == {
        "previous_right_support": 40,
        "next_left_support": 60,
        "combined_source_support": 100,
        "duration_q11": 2048,
    }

    tiny = audit.expected_duration_record({
        "previous_split": 0,
        "previous_last": 2048,
        "next_first": 0,
        "next_split": 2048,
        "feature_duration_sum": 1,
    })
    assert tiny["combined_source_support"] == 4096
    assert tiny["duration_q11"] == 1

    report = audit.audit_records([
        {
            "id": "match",
            "previous_split": 0,
            "previous_last": 40,
            "next_first": 100,
            "next_split": 160,
            "feature_duration_sum": 50,
            "observed_support": 100,
            "observed_duration_q11": 1024,
        },
        {
            "id": "duration-mismatch",
            "previous_split": 0,
            "previous_last": 40,
            "next_first": 100,
            "next_split": 160,
            "feature_duration_sum": 50,
            "observed_duration_q11": 1025,
        },
        {
            "id": "invalid-geometry",
            "previous_split": 10,
            "previous_last": 9,
            "next_first": 0,
            "next_split": 1,
            "feature_duration_sum": 1,
            "observed_duration_q11": 2048,
        },
    ])
    assert report["records"] == 3
    assert report["matched"] == 1
    assert report["mismatched"] == 1
    assert report["invalid"] == 1
    assert report["mismatches"][0]["id"] == "duration-mismatch"
    assert report["invalid_records"][0]["id"] == "invalid-geometry"

    print("test_m36_phone_features: PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
