#!/usr/bin/env python3
"""Compare a Windows-reference Nicolai WAV with the portable-port WAV.

The report deliberately separates exact byte/sample equality from acoustic
similarity. A non-zero timing difference does not automatically mean the port
is wrong; it shows where remaining duration/prosody parity work is needed.
"""
from __future__ import annotations
import argparse
import math
import struct
import wave
from pathlib import Path


def read_pcm16_mono(path: Path):
    with wave.open(str(path), "rb") as w:
        if w.getsampwidth() != 2:
            raise ValueError(f"{path}: expected 16-bit PCM")
        if w.getnchannels() != 1:
            raise ValueError(f"{path}: expected mono")
        rate = w.getframerate()
        n = w.getnframes()
        raw = w.readframes(n)
    samples = list(struct.unpack("<" + "h" * (len(raw)//2), raw))
    return rate, samples


def rms(x):
    if not x:
        return 0.0
    return math.sqrt(sum(v*v for v in x) / len(x))


def peak(x):
    return max((abs(v) for v in x), default=0)


def normalized_corr(a, b):
    n = min(len(a), len(b))
    if n == 0:
        return 0.0
    a = a[:n]; b = b[:n]
    ma = sum(a)/n; mb = sum(b)/n
    aa = [v-ma for v in a]; bb = [v-mb for v in b]
    den = math.sqrt(sum(v*v for v in aa) * sum(v*v for v in bb))
    return sum(x*y for x,y in zip(aa,bb))/den if den else 0.0


def best_lag(ref, test, max_lag):
    # Downsample during lag search to keep this tool stdlib-only and quick.
    step = max(1, min(len(ref), len(test)) // 20000)
    rr = ref[::step]; tt = test[::step]
    ml = max(1, max_lag // step)
    best = (float("-inf"), 0)
    for lag in range(-ml, ml+1):
        if lag >= 0:
            a = rr[lag:]; b = tt[:len(a)]
        else:
            b = tt[-lag:]; a = rr[:len(b)]
        n = min(len(a), len(b))
        if n < 128:
            continue
        c = normalized_corr(a[:n], b[:n])
        if c > best[0]:
            best = (c, lag*step)
    return best[1], best[0]


def aligned_corr(ref, test, lag):
    if lag >= 0:
        ref = ref[lag:]
    else:
        test = test[-lag:]
    n = min(len(ref), len(test))
    return normalized_corr(ref[:n], test[:n]) if n else 0.0, n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("reference", type=Path)
    ap.add_argument("portable", type=Path)
    ap.add_argument("--max-lag-ms", type=float, default=250.0)
    args = ap.parse_args()

    rr, ref = read_pcm16_mono(args.reference)
    pr, port = read_pcm16_mono(args.portable)
    if rr != pr:
        raise SystemExit(f"sample-rate mismatch: reference={rr}, portable={pr}")

    max_lag = int(rr * args.max_lag_ms / 1000.0)
    lag, search_corr = best_lag(ref, port, max_lag)
    corr, compared = aligned_corr(ref, port, lag)
    exact = ref == port

    print(f"sample_rate_hz={rr}")
    print(f"reference_samples={len(ref)}")
    print(f"portable_samples={len(port)}")
    print(f"reference_duration_ms={1000*len(ref)/rr:.3f}")
    print(f"portable_duration_ms={1000*len(port)/pr:.3f}")
    print(f"duration_delta_ms={1000*(len(port)-len(ref))/rr:.3f}")
    print(f"reference_rms={rms(ref):.3f}")
    print(f"portable_rms={rms(port):.3f}")
    print(f"reference_peak={peak(ref)}")
    print(f"portable_peak={peak(port)}")
    print(f"best_lag_samples={lag}")
    print(f"best_lag_ms={1000*lag/rr:.3f}")
    print(f"search_corr={search_corr:.6f}")
    print(f"aligned_corr={corr:.6f}")
    print(f"compared_samples={compared}")
    print(f"exact_samples={'yes' if exact else 'no'}")

if __name__ == "__main__":
    main()
