"""Voice-free contracts for the M40 scalar step audit."""

import json
import subprocess
import sys
import tempfile
import wave
from pathlib import Path

import numpy as np


SCRIPT = Path(__file__).with_name("audit_transients_m40.py")


def write_wav(path: Path, samples: list[int], rate: int = 16000) -> None:
    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(1)
        stream.setsampwidth(2)
        stream.setframerate(rate)
        stream.writeframes(np.asarray(samples, dtype="<i2").tobytes())


def main() -> None:
    with tempfile.TemporaryDirectory() as root:
        temp = Path(root)
        base, trial = temp / "baseline", temp / "trial"
        base.mkdir()
        trial.mkdir()
        write_wav(base / "001.wav", [0, 20000, 0])
        write_wav(trial / "001.wav", [0, 10000, 0, 0])
        report = temp / "report.json"
        command = [sys.executable, str(SCRIPT), str(base), str(trial), "--output", str(report)]
        subprocess.run(command, check=True, capture_output=True)
        parsed = json.loads(report.read_text(encoding="utf-8"))
        assert parsed["summary"]["phrases"] == 1
        assert parsed["summary"]["phrases_with_duration_change"] == 1
        assert parsed["summary"]["total_baseline_steps_over_16000"] == 2
        assert parsed["summary"]["total_trial_steps_over_16000"] == 0
        assert parsed["rows"][0]["sample_delta"] == 1
        assert subprocess.run(command, capture_output=True).returncode != 0, "report was overwritten"
        write_wav(trial / "002.wav", [0, 1])
        assert subprocess.run(command[:-1] + [str(temp / "fresh.json")], capture_output=True).returncode != 0
        (trial / "002.wav").unlink()
        write_wav(trial / "001.wav", [0, 1], rate=8000)
        assert subprocess.run(command[:-1] + [str(temp / "bad-rate.json")], capture_output=True).returncode != 0
    print("M40 transient audit contracts passed")


if __name__ == "__main__":
    main()
