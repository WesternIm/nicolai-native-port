"""Bounded debugger smoke checks on an owned synthetic child only."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--tracer", type=Path, required=True)
    parser.add_argument("--fixture", type=Path, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="nicolai-startup-test-") as temp:
        for mode, code, child_exit in [("ok", 0, 0), ("fail", 4, 7), ("wait", 3, 1460)]:
            output = Path(temp) / f"{mode}.jsonl"
            result = subprocess.run([str(args.tracer.resolve()), str(args.fixture.resolve()),
                                     str(output), "1", mode], capture_output=True, timeout=10)
            assert result.returncode == code, result.stderr
            rows = [json.loads(line) for line in output.read_text(encoding="utf-8").splitlines()]
            assert rows[-1]["event"] == "exit" and rows[-1]["code"] == child_exit, rows[-1]
            assert any(r.get("text") == 'startup fixture: quote=" slash=\\ newline=\n' for r in rows)
            before = output.read_bytes()
            again = subprocess.run([str(args.tracer.resolve()), str(args.fixture.resolve()),
                                    str(output), "1"], capture_output=True, timeout=10)
            assert again.returncode == 1 and output.read_bytes() == before
    print("startup debugger contracts: normal exit, nonzero exit, timeout, no overwrite passed")


if __name__ == "__main__":
    main()
