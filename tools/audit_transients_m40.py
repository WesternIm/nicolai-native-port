"""Compare PCM step outliers for two paired 16-kHz renderer outputs.

This is a click-screening diagnostic, not a perceptual or phone-accuracy score.
The report contains only scalar measurements; WAVs stay local.
"""

import argparse
import json
import wave
from pathlib import Path

import numpy as np


def read_pcm(path: Path) -> np.ndarray:
    with wave.open(str(path), "rb") as stream:
        if (stream.getnchannels(), stream.getsampwidth(), stream.getframerate()) != (1, 2, 16000):
            raise ValueError(f"Expected mono PCM16/16 kHz: {path}")
        return np.frombuffer(stream.readframes(stream.getnframes()), dtype="<i2").astype(np.int32)


def steps(pcm: np.ndarray) -> np.ndarray:
    return np.abs(np.diff(pcm))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("trial", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error("Refusing to overwrite an existing report")
    base_files = sorted(args.baseline.glob("*.wav"))
    trial_files = sorted(args.trial.glob("*.wav"))
    if not base_files or [p.name for p in base_files] != [p.name for p in trial_files]:
        parser.error("Paired WAV sets are empty or do not match")

    rows = []
    for source, candidate in zip(base_files, trial_files):
        a, b = read_pcm(source), read_pcm(candidate)
        x, y = steps(a), steps(b)
        rows.append({
            "id": source.stem,
            "baseline_samples": len(a),
            "trial_samples": len(b),
            "sample_delta": len(b) - len(a),
            "changed_samples_if_equal_length": int(np.count_nonzero(a != b)) if len(a) == len(b) else None,
            "baseline_max_step": int(x.max(initial=0)),
            "trial_max_step": int(y.max(initial=0)),
            "baseline_steps_over_12000": int(np.count_nonzero(x > 12000)),
            "trial_steps_over_12000": int(np.count_nonzero(y > 12000)),
            "baseline_steps_over_16000": int(np.count_nonzero(x > 16000)),
            "trial_steps_over_16000": int(np.count_nonzero(y > 16000)),
        })
    summary = {
        "phrases": len(rows),
        "phrases_with_duration_change": sum(row["sample_delta"] != 0 for row in rows),
        "max_absolute_sample_delta": max(abs(row["sample_delta"]) for row in rows),
        "total_baseline_steps_over_12000": sum(row["baseline_steps_over_12000"] for row in rows),
        "total_trial_steps_over_12000": sum(row["trial_steps_over_12000"] for row in rows),
        "total_baseline_steps_over_16000": sum(row["baseline_steps_over_16000"] for row in rows),
        "total_trial_steps_over_16000": sum(row["trial_steps_over_16000"] for row in rows),
        "phrases_with_higher_max_step": sum(row["trial_max_step"] > row["baseline_max_step"] for row in rows),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps({"summary": summary, "rows": rows}, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
