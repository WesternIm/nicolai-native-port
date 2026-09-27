#!/usr/bin/env python3

from __future__ import annotations

import copy
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import audit_phone_features_m36 as audit  # noqa: E402


def descriptor(positions: list[int], split: int, voicing: list[int]) -> dict:
    intervals = len(positions) - 1
    return {
        "node_count": len(positions),
        "split_index": split,
        "source_position": positions,
        "voicing": voicing,
        "duration_q11": [111] * intervals,
        "pitch_q11": [222] * intervals,
    }


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

    # Independent runtime replay contract. The exact 2047 pitch value is an
    # important x86 integer-truncation fingerprint, not an idealized Q11 unity.
    runtime = {
        "schema": audit.RUNTIME_SCHEMA,
        "sequence": 1,
        "thread_id": 7,
        "feature_before": {
            "count": 3,
            "interval_duration": [160, 160],
            "pitch_anchor": [80, 80, 80],
        },
        "previous_before": descriptor([0, 80, 160, 240], 1, [1, 1, 1]),
        "next_before": descriptor([1000, 1080, 1160, 1240], 2, [1, 1, 1]),
    }
    expected = audit.simulate_runtime_record(runtime)
    assert expected["feature_after"]["pitch_anchor"] == [80, 80, 80]
    assert expected["previous_after"]["duration_q11"] == [111, 2048, 2048]
    assert expected["previous_after"]["pitch_q11"] == [222, 2047, 2047]
    assert expected["next_after"]["duration_q11"] == [2048, 2048, 111]
    assert expected["next_after"]["pitch_q11"] == [2047, 2047, 222]

    runtime.update(copy.deepcopy(expected))
    full = audit.audit_runtime_records([runtime])
    assert full["records"] == 1
    assert full["matched"] == 1
    assert full["mismatched"] == 0
    assert full["invalid"] == 0

    broken = copy.deepcopy(runtime)
    broken["next_after"]["pitch_q11"][0] += 1
    full_bad = audit.audit_runtime_records([broken])
    assert full_bad["matched"] == 0
    assert full_bad["mismatched"] == 1
    assert "next.pitch_q11" in full_bad["mismatches"][0]["reasons"]

    # Missing anchor repair is part of the runtime model now.
    repaired = {
        "schema": audit.RUNTIME_SCHEMA,
        "sequence": 2,
        "thread_id": 7,
        "feature_before": {
            "count": 3,
            "interval_duration": [80, 80],
            "pitch_anchor": [80, 0, 120],
        },
        "previous_before": descriptor([0, 80, 160, 240], 1, [1, 1, 1]),
        "next_before": descriptor([1000, 1080, 1160, 1240], 2, [1, 1, 1]),
    }
    repaired_expected = audit.simulate_runtime_record(repaired)
    assert repaired_expected["feature_after"]["pitch_anchor"] == [80, 80, 120]
    assert repaired_expected["previous_after"]["pitch_q11"] == [222, 2047, 2047]
    assert repaired_expected["next_after"]["pitch_q11"] == [1706, 1706, 222]

    print("test_m36_phone_features: PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
