"""Validate live original caller routes against the compiled portable primitive.

This checks route selection and trace completeness, NOT PCM parity. Raw captures
contain local reference-derived arrays and must never be committed.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import subprocess

SCHEMA = "nicolai-m36-route-v1"
WRITERS = {"initial", "cross", "ordinary"}


def audit(rows: list[dict], probe: Path) -> dict:
    if not rows:
        raise ValueError("empty capture is not evidence")
    active, decisions, pending, complete = {}, [], {}, 0
    errors = []
    for line, row in enumerate(rows, 1):
        if row.get("schema") != SCHEMA:
            raise ValueError(f"line {line}: wrong schema")
        key = (row["thread_id"], row["call"])
        event = row["event"]
        if event == "entry":
            if key in active or any(k[0] == key[0] for k in active):
                raise ValueError(f"line {line}: duplicate/nested call")
            active[key] = dict(row)
            nodes = row["current"]["node_count"]
            if not 2 <= nodes <= 1000 or len(row["current"]["source_position"]) != nodes:
                raise ValueError(f"line {line}: invalid descriptor")
        elif key not in active:
            raise ValueError(f"line {line}: event without entry")
        elif event == "decision":
            if active[key].get("post_loop"):
                raise ValueError(f"line {line}: decision after post_loop")
            if key in pending:
                raise ValueError(f"line {line}: missing interval_end")
            step = row["step"]
            if len(step) != 5 or step[0] != row["interval_index"]:
                raise ValueError(f"line {line}: invalid step")
            nodes = active[key]["current"]["node_count"]
            index = row["interval_index"]
            if not 0 <= index < nodes - 1 or not 0 <= step[1] <= 32767:
                raise ValueError(f"line {line}: decision outside guarded domain")
            if index != active[key].get("next_interval", 0):
                raise ValueError(f"line {line}: missing/reordered decision")
            pending[key] = {"row": row, "nodes": nodes, "writer": None}
        elif event in WRITERS:
            if key not in pending or pending[key]["writer"] is not None:
                raise ValueError(f"line {line}: unexpected writer")
            pending[key]["writer"] = event
        elif event == "interval_end":
            if key not in pending:
                raise ValueError(f"line {line}: interval_end without decision")
            decision = pending.pop(key)
            r = decision["row"]
            decision["observed"] = decision["writer"] or ("dropped" if r["step"][1] == 0 else "deferred")
            decisions.append(decision)
            active[key]["next_interval"] = r["interval_index"] + 1
        elif event == "failure_return":
            errors.append(f"call {key}: original writer failed")
            pending.pop(key, None)
            active.pop(key)
        elif event == "return":
            entry = active.pop(key)
            if key in pending or not entry.get("post_loop") or entry.get("next_interval", 0) != entry["current"]["node_count"] - 1:
                raise ValueError(f"line {line}: incomplete call")
            complete += 1
        elif event == "post_loop":
            if key in pending or active[key].get("post_loop"):
                raise ValueError(f"line {line}: misplaced post_loop")
            active[key]["post_loop"] = True
        elif event == "terminal_flush":
            if not active[key].get("post_loop") or active[key].get("terminal_flush"):
                raise ValueError(f"line {line}: misplaced terminal_flush")
            active[key]["terminal_flush"] = True
        else:
            raise ValueError(f"line {line}: unknown event {event}")
    if active or pending or not decisions or not complete:
        raise ValueError("capture has incomplete/no successful calls")
    request = "".join(f"{d['row']['step'][1]} {d['row']['interval_index']} {d['nodes']} "
                      f"{int(d['row']['cross_pending'] != 0)} {int(d['row']['already_started'] != 0)}\n"
                      for d in decisions)
    result = subprocess.run([str(probe.resolve())], input=request, text=True,
                            capture_output=True, timeout=30, check=True)
    expected = result.stdout.splitlines()
    if len(expected) != len(decisions):
        raise ValueError("portable probe row count mismatch")
    counts = {}
    for d, want in zip(decisions, expected):
        got = d["observed"]
        counts[got] = counts.get(got, 0) + 1
        if got != want:
            errors.append(f"call {d['row']['call']} interval {d['row']['interval_index']}: {got} != {want}")
    return {"schema": "nicolai-m36-route-audit-v1", "calls": complete,
            "decisions": len(decisions), "observed_routes": counts,
            "mismatches": len(errors), "errors": errors, "pcm_parity_verified": False}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    rows = [json.loads(line) for line in args.capture.read_text(encoding="utf-8").splitlines() if line.strip()]
    report = audit(rows, args.probe)
    text = json.dumps(report, ensure_ascii=False, indent=2)
    if args.output:
        with args.output.open("x", encoding="utf-8") as handle:
            handle.write(text + "\n")
    print(text)
    return int(report["mismatches"] != 0)


if __name__ == "__main__":
    raise SystemExit(main())
