# M32 PC parity report

Golden target: original Windows **ELAN TTS Russian (Nicolai 16Khz)** reference
pack `reference_pack_20260924_222948`, 22 phrases.

## Validation

- MSVC host tests: **20/20**.
- GitHub Actions: Ubuntu and Windows pass.
- Every candidate: **22/22** portable WAVs rendered.
- Evaluated: 39 unique baseline/coarse/micro/fine settings.
- M32 zero-strength baseline vs clean M31 build: **22/22 SHA-256 identical**.
- Golden/reference WAVs and proprietary voice resources remain untracked.

## Baseline and representative candidates

The table below uses the committed reproducible M32 metric implementation.
Higher correlation is better; every MAE/distance is lower-is-better.

| candidate | correlation | active MAE | total MAE | F0 MAE | MFCC-DTW | RMS ratio | RMS MAE |
|---|---:|---:|---:|---:|---:|---:|---:|
| **baseline 0/0** | **0.1965157** | 5.6790% | 3.5987% | **13.4799%** | 52.0139 | 0.9344 | 10.2377% |
| length 0.0005 | **0.1969053** | **5.6243%** | **3.5714%** | 13.7524% | 52.0930 | 0.9341 | 10.2563% |
| energy 0.002 | 0.1965258 | 5.6789% | 3.5987% | 13.4799% | **52.0133** | 0.9342 | 10.2558% |
| length 0.0625 | 0.1925048 | 5.5966% | 3.5314% | 13.9436% | 52.3230 | 0.9395 | 10.2266% |
| length 0.125 | 0.1872074 | 5.7849% | 3.4714% | 14.0258% | 52.3383 | 0.9463 | 9.9264% |
| energy 1.0 | 0.1933462 | 5.6313% | 3.5987% | 13.3613% | 53.1227 | 0.8673 | 16.9505% |

The tiny `energy=0.002` movements are below a meaningful selection threshold
and RMS parity moves in the wrong direction. Larger energy strengths confirm
the same trend. Duration settings can improve timing or RMS, but lose waveform,
F0, and/or spectral parity.

## Fine-grid outlier audit

`length=0.0005` is the only non-zero point whose aggregate correlation exceeds
baseline. Adjacent `0.0004` and `0.0006` settings fall below baseline, so the
point is not a smooth optimum. Phrase-level comparison gives:

| metric | improved | regressed | tied |
|---|---:|---:|---:|
| active correlation | 10 | 9 | 3 |
| active-duration error | 8 | 6 | 8 |
| total-duration error | 6 | 8 | 8 |
| F0 contour error | 9 | **13** | 0 |
| MFCC-DTW | 9 | **13** | 0 |
| RMS-ratio error | 10 | **12** | 0 |

Phrase `002` (`папа`) contributes the largest correlation gain, +0.03421, but
also the largest F0 regression, +2.520 percentage points at phrase level. The
candidate fails the predeclared multi-metric acceptance rule.

## MFCC scale note

The historical M30/M31 report records MFCC-DTW **128.589932660**, but its
one-off implementation was not included in the M31 source archive. The new
committed implementation uses 12 MFCC coefficients, 512-sample FFT, 160-sample
hop, active-region input, Euclidean DTW, and path-length normalization; it gives
the same baseline audio **52.013872810**. These absolute scales must not be
mixed. Candidate decisions use only like-for-like values from the committed
implementation. All non-MFCC baseline metrics reproduce the archived M31
values to rounding precision.

## Production decision

```text
physical_length_strength = 0.0
physical_energy_strength = 0.0
```

M32 retains the phone-local execution code and its tests, but its validated
production audio follows the exact M31 fallback. No phrase-specific rule was
added.

Reproducible artifacts:

- `metrics/m32-coarse-sweep-summary.{json,csv}`
- `metrics/m32-micro-sweep-summary.{json,csv}`
- `metrics/m32-length-fine-summary.{json,csv}`
- `metrics/m32-baseline-parity.{json,csv}`
- `metrics/m32-length-0.0005-parity.{json,csv}`
- `metrics/m32-length-0.0005-vs-baseline.{json,csv}`
