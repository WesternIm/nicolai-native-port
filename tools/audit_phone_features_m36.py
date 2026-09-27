#!/usr/bin/env python3
"""Audit original 0x101a2780 phone-feature/coefficient records.

Two input formats are accepted:

1. Legacy scalar JSON records used by the first M36 slice. They validate only
   previous-right + next-left support and duration Q11.
2. JSONL records emitted by nicolai_m36_runtime_capture. These contain complete
   before/after feature and descriptor snapshots and are independently replayed
   against the statically recovered 0x101a2780 arithmetic/control flow.

No proprietary bytes are required by this auditor.
"""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path
from typing import Any

RUNTIME_SCHEMA = "nicolai-m36-runtime-record-v1"


def trunc_div(a: int, b: int) -> int:
    if b == 0:
        raise ValueError("division by zero")
    q = abs(a) // abs(b)
    return -q if (a < 0) != (b < 0) else q


def i16(value: int) -> int:
    value &= 0xFFFF
    return value if value < 0x8000 else value - 0x10000


def i32(value: int) -> int:
    value &= 0xFFFFFFFF
    return value if value < 0x80000000 else value - 0x100000000


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

    duration_q11 = trunc_div(i32(feature_duration_sum << 11), support)
    if duration_q11 == 0:
        duration_q11 = 1
    return {
        "previous_right_support": previous_right,
        "next_left_support": next_left,
        "combined_source_support": support,
        "duration_q11": duration_q11,
    }


def _validate_descriptor(d: dict[str, Any], label: str) -> None:
    node_count = int(d["node_count"])
    split = int(d["split_index"])
    positions = [int(x) for x in d["source_position"]]
    voicing = [i16(int(x)) for x in d["voicing"]]
    duration = [i16(int(x)) for x in d["duration_q11"]]
    pitch = [i16(int(x)) for x in d["pitch_q11"]]
    if node_count < 2 or len(positions) != node_count:
        raise ValueError(f"{label}: invalid node_count/positions")
    if split < 0 or split >= node_count:
        raise ValueError(f"{label}: invalid split_index")
    if any(positions[i] <= positions[i - 1] for i in range(1, node_count)):
        raise ValueError(f"{label}: source positions not strictly increasing")
    intervals = node_count - 1
    if len(voicing) != intervals or len(duration) != intervals or len(pitch) != intervals:
        raise ValueError(f"{label}: coefficient lane size mismatch")


def _repair_anchor(pitch_anchor: list[int], index: int) -> None:
    if index < 0 or index + 1 >= len(pitch_anchor):
        return
    left = i16(pitch_anchor[index])
    right = i16(pitch_anchor[index + 1])
    if left == 0 and right != 0:
        pitch_anchor[index] = right
        left = right
    if left != 0 and right == 0:
        pitch_anchor[index + 1] = left


def _scaled_feature_width(duration_word: int, duration_q11: int) -> int:
    numerator = i32(i16(duration_word) << 11)
    return trunc_div(numerator, duration_q11)


def _reciprocal_pitch(start: int, length: int, end: int, position: int) -> int:
    if start == 0 or end == 0 or length == 0:
        raise ValueError("invalid reciprocal-pitch input")
    a = trunc_div(0x10000000, start)
    b = trunc_div(0x10000000, end)
    slope = trunc_div(i32(b - a), length)
    return i32(a + i32(slope * position))


def _pitch_q11(width: int, left: int, right: int, feature_width: int, position: int) -> int:
    reciprocal = _reciprocal_pitch(left, feature_width, right, position)
    product = i32(reciprocal * width)
    return i16((product & 0xFFFFFFFF) >> 17)


def simulate_runtime_record(record: dict[str, Any]) -> dict[str, Any]:
    feature = copy.deepcopy(record["feature_before"])
    previous = copy.deepcopy(record["previous_before"])
    next_desc = copy.deepcopy(record["next_before"])
    _validate_descriptor(previous, "previous")
    _validate_descriptor(next_desc, "next")

    count = int(feature["count"])
    durations = [i16(int(x)) for x in feature.get("interval_duration", [])]
    anchors = [i16(int(x)) for x in feature.get("pitch_anchor", [])]
    feature["interval_duration"] = durations
    feature["pitch_anchor"] = anchors

    for desc in (previous, next_desc):
        desc["source_position"] = [int(x) for x in desc["source_position"]]
        desc["voicing"] = [i16(int(x)) for x in desc["voicing"]]
        desc["duration_q11"] = [i16(int(x)) for x in desc["duration_q11"]]
        desc["pitch_q11"] = [i16(int(x)) for x in desc["pitch_q11"]]

    prev_last_interval = int(previous["node_count"]) - 1
    prev_split = int(previous["split_index"])
    next_split = int(next_desc["split_index"])

    if count <= 0:
        for j in range(prev_split, prev_last_interval):
            previous["duration_q11"][j] = 2048
            previous["pitch_q11"][j] = 2048
        for j in range(0, next_split):
            next_desc["duration_q11"][j] = 2048
            next_desc["pitch_q11"][j] = 2048
        return {"feature_after": feature, "previous_after": previous, "next_after": next_desc}

    feature_intervals = count - 1
    if feature_intervals <= 0 or len(durations) != feature_intervals or len(anchors) != count:
        raise ValueError("feature count/array size mismatch")

    support = (
        previous["source_position"][-1]
        - previous["source_position"][prev_split]
        + next_desc["source_position"][next_split]
        - next_desc["source_position"][0]
    )
    if support == 0:
        raise ValueError("zero combined source support")
    total_duration = sum(durations)
    duration_q11 = trunc_div(i32(total_duration << 11), support)
    if duration_q11 == 0:
        duration_q11 = 1

    feature_index = 0
    coordinate = 0
    feature_width = _scaled_feature_width(durations[0], duration_q11)
    if feature_width == 0:
        raise ValueError("zero feature width")
    last_source_width = 0
    previous_index = prev_split

    # Previous-right half.
    while feature_index < feature_intervals and previous_index < prev_last_interval:
        feature_width = _scaled_feature_width(durations[feature_index], duration_q11)
        if feature_width == 0:
            raise ValueError("zero feature width")
        _repair_anchor(anchors, feature_index)
        left = anchors[feature_index]
        right = anchors[feature_index + 1]

        while previous_index < prev_last_interval:
            if coordinate > feature_width:
                feature_index += 1
                feature_width = i32(feature_width + last_source_width)
                coordinate = i32(coordinate - feature_width)
                break

            positions = previous["source_position"]
            last_source_width = positions[previous_index + 1] - positions[previous_index]
            coordinate = i32(coordinate + last_source_width)
            if coordinate > i32(feature_width + 3):
                continue

            previous["duration_q11"][previous_index] = i16(duration_q11)
            if (left == 0 and right == 0) or previous["voicing"][previous_index] == 0:
                previous["pitch_q11"][previous_index] = 2048
            else:
                previous["pitch_q11"][previous_index] = _pitch_q11(
                    last_source_width, left, right, feature_width, coordinate
                )
            previous_index += 1

    # The original touches a sentinel pair after some terminal paths. The
    # capture schema intentionally records only declared anchors, so declared
    # records stop here if no feature interval remains.
    if feature_index >= feature_intervals or next_split <= 0:
        feature["pitch_anchor"] = anchors
        return {"feature_after": feature, "previous_after": previous, "next_after": next_desc}

    # Next-left half.
    _repair_anchor(anchors, feature_index)
    next_index = 0
    while feature_index < feature_intervals and next_index < next_split:
        feature_width = _scaled_feature_width(durations[feature_index], duration_q11)
        if feature_width == 0:
            raise ValueError("zero feature width")
        _repair_anchor(anchors, feature_index)
        left = anchors[feature_index]
        right = anchors[feature_index + 1]

        while next_index < next_split:
            if coordinate > feature_width:
                feature_index += 1
                feature_width = i32(feature_width + last_source_width)
                coordinate = i32(coordinate - feature_width)
                break

            positions = next_desc["source_position"]
            last_source_width = positions[next_index + 1] - positions[next_index]
            coordinate = i32(coordinate + last_source_width)
            if coordinate > i32(feature_width + 3):
                continue

            next_desc["duration_q11"][next_index] = i16(duration_q11)
            if (left == 0 and right == 0) or next_desc["voicing"][next_index] == 0:
                next_desc["pitch_q11"][next_index] = 2048
            else:
                position = coordinate
                if next_index == next_split - 1 and feature_index == feature_intervals - 1:
                    position = i32(feature_width - last_source_width)
                next_desc["pitch_q11"][next_index] = _pitch_q11(
                    last_source_width, left, right, feature_width, position
                )
            next_index += 1

    feature["pitch_anchor"] = anchors
    return {"feature_after": feature, "previous_after": previous, "next_after": next_desc}


def _runtime_reasons(record: dict[str, Any], expected: dict[str, Any]) -> list[str]:
    reasons: list[str] = []
    actual_feature = record["feature_after"]
    if [i16(int(x)) for x in actual_feature.get("pitch_anchor", [])] != expected["feature_after"]["pitch_anchor"]:
        reasons.append("pitch_anchor_repair")

    for label in ("previous", "next"):
        actual = record[f"{label}_after"]
        exp = expected[f"{label}_after"]
        for lane in ("source_position", "voicing", "duration_q11", "pitch_q11"):
            actual_values = [int(x) for x in actual[lane]]
            if lane != "source_position":
                actual_values = [i16(x) for x in actual_values]
            if actual_values != exp[lane]:
                reasons.append(f"{label}.{lane}")
        if int(actual["node_count"]) != int(exp["node_count"]):
            reasons.append(f"{label}.node_count")
        if int(actual["split_index"]) != int(exp["split_index"]):
            reasons.append(f"{label}.split_index")
    return reasons


def audit_runtime_records(records: list[dict[str, Any]]) -> dict[str, Any]:
    mismatches: list[dict[str, Any]] = []
    invalid: list[dict[str, Any]] = []
    for index, record in enumerate(records):
        record_id = str(record.get("sequence", index))
        try:
            if record.get("schema") != RUNTIME_SCHEMA:
                raise ValueError("unexpected runtime schema")
            expected = simulate_runtime_record(record)
            reasons = _runtime_reasons(record, expected)
        except (KeyError, TypeError, ValueError, ZeroDivisionError) as exc:
            invalid.append({"id": record_id, "error": str(exc)})
            continue
        if reasons:
            mismatches.append({"id": record_id, "reasons": reasons})

    return {
        "schema": "m36-phone-runtime-audit-v2",
        "records": len(records),
        "matched": len(records) - len(mismatches) - len(invalid),
        "mismatched": len(mismatches),
        "invalid": len(invalid),
        "mismatches": mismatches,
        "invalid_records": invalid,
        "scope": {
            "modeled": [
                "feature record count/duration/pitch-anchor layout",
                "previous-right + next-left source ownership",
                "duration Q11 construction",
                "missing pitch-anchor repair",
                "voicing-forced unity pitch",
                "reciprocal pitch interpolation with x86 integer behavior",
                "+3 feature-boundary tolerance",
                "terminal next-left pitch-position special case",
            ],
            "not_modeled": [
                "feature production upstream of 0x101a2780",
                "descriptor packaging upstream of 0x101a2780",
                "runtime drop/rewind after descriptor packaging",
                "source selection and window construction",
            ],
        },
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
                "pitch lane",
                "runtime feature producer",
                "descriptor packaging and runtime rollback",
            ],
        },
    }


def load_records(path: Path) -> tuple[str, list[dict[str, Any]]]:
    text = path.read_text(encoding="utf-8-sig")
    stripped = text.lstrip()
    if not stripped:
        raise ValueError("input is empty")

    # Runtime capture is JSONL. A regular JSON object/list remains supported for
    # the first-slice scalar audit and hand-authored test fixtures.
    if path.suffix.lower() == ".jsonl":
        records = [json.loads(line) for line in text.splitlines() if line.strip()]
        if not all(isinstance(x, dict) for x in records):
            raise ValueError("JSONL input must contain one object per line")
        return "runtime", records

    payload = json.loads(text)
    if isinstance(payload, dict):
        payload = payload.get("records")
    if not isinstance(payload, list) or not all(isinstance(x, dict) for x in payload):
        raise ValueError("input must be a JSON record list/object or runtime JSONL")
    mode = "runtime" if payload and payload[0].get("schema") == RUNTIME_SCHEMA else "scalar"
    return mode, payload


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path, help="captured feature records JSON/JSONL")
    parser.add_argument("--output", type=Path, help="write report JSON instead of stdout only")
    args = parser.parse_args()

    try:
        mode, records = load_records(args.input)
        report = audit_runtime_records(records) if mode == "runtime" else audit_records(records)
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
