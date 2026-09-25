#!/usr/bin/env python3
"""Measure Nicolai portable output against the 22-WAV Windows golden pack.

The summary intentionally keeps the M30/M31 metric definitions together:
active waveform correlation, active and total duration parity, RMS/energy
parity, normalized 12-bin F0 contour MAE, and 12-coefficient MFCC-DTW.
Lower is better for every MAE/distance; higher is better for correlation.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import wave
from pathlib import Path

try:
    import librosa
    import numpy as np
except ImportError as exc:  # pragma: no cover - exercised by local setup
    raise SystemExit(
        "measure_parity.py requires numpy and librosa; "
        "install tools/requirements-parity.txt"
    ) from exc


def read_pcm(path: Path) -> tuple[int, np.ndarray]:
    with wave.open(str(path), "rb") as wav:
        if wav.getnchannels() != 1 or wav.getsampwidth() != 2:
            raise ValueError(f"{path}: expected mono PCM16")
        sample_rate = wav.getframerate()
        samples = np.frombuffer(wav.readframes(wav.getnframes()), dtype="<i2").astype(np.float64)
    return sample_rate, samples


def active_bounds(samples: np.ndarray, sample_rate: int) -> tuple[int, int]:
    if samples.size == 0:
        return 0, 0
    threshold = max(32.0, float(np.max(np.abs(samples))) * 0.005)
    hits = np.flatnonzero(np.abs(samples) > threshold)
    if hits.size == 0:
        return 0, 0
    pad = int(sample_rate * 0.010)
    return max(0, int(hits[0]) - pad), min(samples.size, int(hits[-1]) + 1 + pad)


def correlation_at_lag(a: np.ndarray, b: np.ndarray, lag: int, step: int) -> float:
    if lag >= 0:
        x, y = a[lag:], b
    else:
        x, y = a, b[-lag:]
    count = min(x.size, y.size)
    if count < 128:
        return -2.0
    x = x[:count:step]
    y = y[:count:step]
    if x.size < 32:
        return -2.0
    x = x - np.mean(x)
    y = y - np.mean(y)
    den = float(np.linalg.norm(x) * np.linalg.norm(y))
    return float(np.dot(x, y) / den) if den else 0.0


def best_lag(a: np.ndarray, b: np.ndarray, sample_rate: int, max_ms: float = 250.0) -> tuple[int, float]:
    maximum = int(sample_rate * max_ms / 1000.0)
    coarse = max(1, (2 * maximum) // 500)
    best = (-2.0, 0)
    for lag in range(-maximum, maximum + 1, coarse):
        score = correlation_at_lag(a, b, lag, 8)
        if score > best[0]:
            best = (score, lag)
    for lag in range(max(-maximum, best[1] - coarse), min(maximum, best[1] + coarse) + 1):
        score = correlation_at_lag(a, b, lag, 4)
        if score > best[0]:
            best = (score, lag)
    return best[1], best[0]


def f0_curve(samples: np.ndarray, sample_rate: int, bins: int = 12) -> np.ndarray:
    y = (samples / 32768.0).astype(np.float32)
    y, _ = librosa.effects.trim(y, top_db=35)
    if y.size < 1024:
        return np.full(bins, np.nan)
    f0 = librosa.yin(y, fmin=55, fmax=150, sr=sample_rate, frame_length=1024, hop_length=160)
    level = librosa.feature.rms(y=y, frame_length=1024, hop_length=160)[0][: f0.size]
    f0 = f0[: level.size]
    gate = max(float(np.percentile(level, 30)), 0.005)
    f0 = np.where(level > gate, f0, np.nan)
    curve = []
    for index in range(bins):
        begin = int(f0.size * index / bins)
        end = max(begin + 1, int(f0.size * (index + 1) / bins))
        values = f0[begin:end]
        values = values[np.isfinite(values)]
        curve.append(float(np.median(values)) if values.size else math.nan)
    return np.asarray(curve)


def mfcc_dtw(reference: np.ndarray, portable: np.ndarray, sample_rate: int) -> float:
    def features(samples: np.ndarray) -> np.ndarray:
        y = (samples / 32768.0).astype(np.float32)
        return librosa.feature.mfcc(
            y=y, sr=sample_rate, n_mfcc=12, n_fft=512, hop_length=160, center=True
        )

    ref_features = features(reference)
    port_features = features(portable)
    cumulative, path = librosa.sequence.dtw(
        X=ref_features, Y=port_features, metric="euclidean"
    )
    return float(cumulative[-1, -1] / max(1, len(path)))


def mean(values: list[float]) -> float:
    return float(sum(values) / len(values)) if values else math.nan


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("reference_pack", type=Path)
    parser.add_argument("portable_dir", type=Path)
    parser.add_argument("--json", dest="json_out", type=Path)
    parser.add_argument("--csv", dest="csv_out", type=Path)
    args = parser.parse_args()

    manifest = json.loads((args.reference_pack / "manifest.json").read_text(encoding="utf-8-sig"))
    rows: list[dict[str, object]] = []
    all_f0_error: list[float] = []

    for item in manifest["results"]:
        phrase_id = item["id"]
        reference_path = args.reference_pack / item["wav"]
        portable_path = args.portable_dir / f"{phrase_id}.wav"
        row: dict[str, object] = {"id": phrase_id, "text": item["text"], "status": "missing"}
        if not portable_path.exists():
            rows.append(row)
            continue

        ref_rate, reference = read_pcm(reference_path)
        port_rate, portable = read_pcm(portable_path)
        if ref_rate != port_rate:
            raise ValueError(f"{phrase_id}: sample-rate mismatch {ref_rate} vs {port_rate}")
        ref_begin, ref_end = active_bounds(reference, ref_rate)
        port_begin, port_end = active_bounds(portable, port_rate)
        ref_active = reference[ref_begin:ref_end]
        port_active = portable[port_begin:port_end]
        lag, correlation = best_lag(ref_active, port_active, ref_rate)

        reference_rms = float(np.sqrt(np.mean(reference * reference))) if reference.size else 0.0
        portable_rms = float(np.sqrt(np.mean(portable * portable))) if portable.size else 0.0
        active_ratio = port_active.size / ref_active.size if ref_active.size else math.nan
        total_ratio = portable.size / reference.size if reference.size else math.nan
        rms_ratio = portable_rms / reference_rms if reference_rms else math.nan

        ref_f0 = f0_curve(reference, ref_rate)
        port_f0 = f0_curve(portable, port_rate)
        valid = np.isfinite(ref_f0) & np.isfinite(port_f0) & (ref_f0 != 0)
        phrase_f0 = np.abs((port_f0[valid] - ref_f0[valid]) / ref_f0[valid])
        all_f0_error.extend(float(value) for value in phrase_f0)

        row.update(
            status="ok",
            active_correlation=correlation,
            active_best_lag_ms=1000.0 * lag / ref_rate,
            active_duration_ratio=active_ratio,
            active_duration_error=abs(active_ratio - 1.0),
            total_duration_ratio=total_ratio,
            total_duration_error=abs(total_ratio - 1.0),
            rms_ratio=rms_ratio,
            rms_ratio_error=abs(rms_ratio - 1.0),
            f0_contour_relative_mae=float(np.mean(phrase_f0)) if phrase_f0.size else math.nan,
            mfcc_dtw=mfcc_dtw(ref_active, port_active, ref_rate),
        )
        rows.append(row)

    ok = [row for row in rows if row["status"] == "ok"]
    summary = {
        "reference_voice": manifest.get("selectedVoice", {}).get("description"),
        "phrases": len(rows),
        "rendered": len(ok),
        "mean_active_waveform_correlation": mean([float(row["active_correlation"]) for row in ok]),
        "mean_active_duration_ratio": mean([float(row["active_duration_ratio"]) for row in ok]),
        "active_duration_mae_percent": 100.0 * mean([float(row["active_duration_error"]) for row in ok]),
        "total_duration_mae_percent": 100.0 * mean([float(row["total_duration_error"]) for row in ok]),
        "f0_contour_mae_percent": 100.0 * mean(all_f0_error),
        "mean_mfcc_dtw": mean([float(row["mfcc_dtw"]) for row in ok]),
        "mean_rms_ratio": mean([float(row["rms_ratio"]) for row in ok]),
        "rms_ratio_mae_percent": 100.0 * mean([float(row["rms_ratio_error"]) for row in ok]),
    }
    result = {"summary": summary, "rows": rows}
    rendered = json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False)
    print(rendered)
    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(rendered + "\n", encoding="utf-8")
    if args.csv_out:
        args.csv_out.parent.mkdir(parents=True, exist_ok=True)
        keys: list[str] = []
        for row in rows:
            for key in row:
                if key not in keys:
                    keys.append(key)
        with args.csv_out.open("w", newline="", encoding="utf-8-sig") as stream:
            writer = csv.DictWriter(stream, fieldnames=keys)
            writer.writeheader()
            writer.writerows(rows)


if __name__ == "__main__":
    main()
