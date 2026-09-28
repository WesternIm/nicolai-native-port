#!/usr/bin/env python3
"""Gain-invariant spectral diagnostics for paired original/portable WAVs.

This is an utterance-level screen, not a phoneme alignment or breathiness score.
The JSON contains scalar measurements and hashes, never text or PCM.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np

from measure_parity import active_bounds, read_pcm

FRAME = 2048
HOP = 256
LOW = (300.0, 3000.0)
HIGH = (3000.0, 7000.0)


def summarize(samples: np.ndarray, rate: int) -> dict[str, float | int]:
    if rate != 16000:
        raise ValueError("expected 16-kHz PCM")
    begin, end = active_bounds(samples, rate)
    y = samples[begin:end] / 32768.0
    if len(y) < FRAME:
        raise ValueError("active WAV is shorter than one analysis frame")
    frames = np.lib.stride_tricks.sliding_window_view(y, FRAME)[::HOP]
    frame_rms = np.sqrt(np.mean(frames * frames, axis=1))
    gate = max(0.003, float(np.max(frame_rms)) * 0.08)
    frames = frames[frame_rms >= gate]
    if not len(frames):
        raise ValueError("no frames pass the fixed active-energy gate")
    power = np.abs(np.fft.rfft(frames * np.hanning(FRAME), axis=1)) ** 2
    freq = np.fft.rfftfreq(FRAME, 1.0 / rate)
    low = np.sum(power[:, (freq >= LOW[0]) & (freq < LOW[1])], axis=1)
    high = np.sum(power[:, (freq >= HIGH[0]) & (freq < HIGH[1])], axis=1)
    ratio_db = 10.0 * np.log10((high + 1e-12) / (low + 1e-12))
    band = power[:, (freq >= LOW[0]) & (freq < HIGH[1])]
    flatness = np.exp(np.mean(np.log(band + 1e-12), axis=1)) / np.mean(band + 1e-12, axis=1)
    return {
        "frames": int(len(frames)),
        "high_to_low_median_db": float(np.median(ratio_db)),
        "flatness_median": float(np.median(flatness)),
    }


def audit(reference: Path, portable: Path) -> dict:
    names = sorted({p.name for p in reference.glob("*.wav")} & {p.name for p in portable.glob("*.wav")})
    if not names:
        raise ValueError("no paired WAVs")
    rows = []
    for name in names:
        rp, pp = reference / name, portable / name
        rs, ry = read_pcm(rp)
        ps, py = read_pcm(pp)
        ref, port = summarize(ry, rs), summarize(py, ps)
        rows.append({
            "id": Path(name).stem,
            "reference_sha256": hashlib.sha256(rp.read_bytes()).hexdigest(),
            "portable_sha256": hashlib.sha256(pp.read_bytes()).hexdigest(),
            "reference": ref,
            "portable": port,
            "high_to_low_delta_db": port["high_to_low_median_db"] - ref["high_to_low_median_db"],
            "flatness_delta": port["flatness_median"] - ref["flatness_median"],
        })
    ratio_deltas = [r["high_to_low_delta_db"] for r in rows]
    flatness_deltas = [r["flatness_delta"] for r in rows]
    return {
        "version": "m39-spectral-screen-1",
        "caveat": "Utterance-level energy-gated spectra are not phone-aligned or a perceptual breathiness measure.",
        "frame_samples": FRAME,
        "hop_samples": HOP,
        "low_hz": LOW,
        "high_hz": HIGH,
        "phrases": len(rows),
        "mean_high_to_low_delta_db": float(np.mean(ratio_deltas)),
        "median_high_to_low_delta_db": float(np.median(ratio_deltas)),
        "phrases_port_brighter": sum(v > 0 for v in ratio_deltas),
        "mean_flatness_delta": float(np.mean(flatness_deltas)),
        "phrases_port_flatter_spectrum": sum(v > 0 for v in flatness_deltas),
        "rows": rows,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path)
    parser.add_argument("portable", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError("refusing to overwrite existing report")
    result = audit(args.reference, args.portable)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: result[key] for key in (
        "phrases", "mean_high_to_low_delta_db", "median_high_to_low_delta_db",
        "phrases_port_brighter", "mean_flatness_delta", "phrases_port_flatter_spectrum")}, indent=2))


if __name__ == "__main__":
    main()
