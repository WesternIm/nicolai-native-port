"""Verify byte-identical fallback for the same complete 22-WAV corpus."""
import argparse
import hashlib
import json
from pathlib import Path


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()
    expected = {f"{i:03}.wav" for i in range(1, 23)}
    for directory in (args.baseline, args.candidate):
        found = {p.name for p in directory.glob("*.wav")}
        if found != expected:
            raise SystemExit(f"Incomplete/extra corpus in {directory}: {sorted(found ^ expected)}")
    rows = [dict(id=name[:-4], baseline_sha256=digest(args.baseline / name),
                 candidate_sha256=digest(args.candidate / name)) for name in sorted(expected)]
    for row in rows:
        row["identical"] = row["baseline_sha256"] == row["candidate_sha256"]
    report = dict(phrases=22, identical=sum(r["identical"] for r in rows), rows=rows)
    output = json.dumps(report, indent=2) + "\n"
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(output, encoding="utf-8")
    print(f"identical={report['identical']}/22")
    raise SystemExit(0 if report["identical"] == 22 else 1)


if __name__ == "__main__":
    main()
