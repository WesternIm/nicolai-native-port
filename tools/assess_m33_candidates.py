"""Conservative aggregate parity gate; missing/invalid corpus evidence fails."""
import argparse
import json
import math
from pathlib import Path

METRICS = {
    "mean_active_waveform_correlation": True,
    "active_duration_mae_percent": False,
    "total_duration_mae_percent": False,
    "f0_contour_mae_percent": False,
    "mean_mfcc_dtw": False,
    "rms_ratio_mae_percent": False,
}


def load_report(path):
    report = json.loads(path.read_text(encoding="utf-8-sig"))
    summary, rows = report["summary"], report["rows"]
    expected = {f"{i:03}" for i in range(1, 23)}
    if (summary["phrases"] != 22 or summary["rendered"] != 22 or len(rows) != 22 or
            {row["id"] for row in rows} != expected or any(row["status"] != "ok" for row in rows)):
        raise ValueError(f"{path}: expected complete unique 22/22 successful corpus")
    if any(not math.isfinite(summary[metric]) for metric in METRICS):
        raise ValueError(f"{path}: non-finite aggregate metric")
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_root", type=Path)
    parser.add_argument("--json", type=Path, required=True)
    args = parser.parse_args()
    settings = json.loads((args.output_root / "summary.json").read_text(encoding="utf-8-sig"))
    names = [setting["candidate"] for setting in settings]
    if len(names) != len(set(names)) or "baseline" not in names:
        raise ValueError("Missing/duplicate baseline or candidate identity")
    baseline = load_report(args.output_root / "baseline" / "parity.json")
    candidates = []
    for setting in settings:
        name = setting["candidate"]
        candidate = load_report(args.output_root / name / "parity.json")
        if candidate["reference_voice"] != baseline["reference_voice"]:
            raise ValueError("Reference voice mismatch")
        directional = {metric: (candidate[metric] - baseline[metric]) * (1 if up else -1)
                       for metric, up in METRICS.items()}
        regressions = [metric for metric, delta in directional.items() if delta < -1e-9]
        improvements = [metric for metric, delta in directional.items() if delta > 1e-9]
        candidates.append(dict(candidate=name, settings={k: v for k, v in setting.items() if k != "metrics"},
                               eligible=not regressions and bool(improvements),
                               improved_metrics=improvements, regressed_metrics=regressions,
                               directional_deltas=directional))
    result = dict(phrases=22, rule="No aggregate metric regression (>1e-9), at least one improvement",
                  caveat="Fixed selection corpus; not independent held-out validation",
                  candidates=candidates)
    args.json.parent.mkdir(parents=True, exist_ok=True)
    args.json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("Eligible: " + (", ".join(c["candidate"] for c in candidates if c["eligible"]) or "none"))


if __name__ == "__main__":
    main()
