# M35 — six settings × 22 phrases, metric and clock audit

This is diagnostic work; no audio setting is promoted. All 132 rendered WAVs
are byte-identical to the corresponding M34 outputs. Historical metrics below
are unchanged, retaining comparability rather than claiming measurement truth.
Equal phrase means are used unless explicitly labeled pooled coverage.

## Historical metrics

| Setting | Wave corr ↑ | F0 MAE % ↓ | Active MAE % ↓ | Total MAE % ↓ | MFCC-DTW ↓ | RMS MAE % ↓ |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| baseline | 0.196516 | 13.4799 | 5.6790 | 3.5987 | 52.0139 | 10.2377 |
| stateful-unit | 0.199750 | 13.8641 | 26.9387 | 16.8345 | 53.1790 | 9.3399 |
| stateful-shared | 0.207715 | 15.8847 | 16.5177 | 10.5941 | 55.7325 | 7.4986 |
| stateful-length | 0.208550 | 15.7186 | 16.5026 | 10.5661 | 56.0231 | 7.3897 |
| stateful-energy | 0.207704 | 15.8847 | 16.5177 | 10.5941 | 55.7304 | 7.5160 |
| stateful-both | 0.208539 | 15.7186 | 16.5026 | 10.5661 | 56.0210 | 7.4071 |

Length/energy cases use 0.002 on the shared execution path; baseline strengths
are zero. JSON/CSV per-phrase reports are in docs/metrics/m35-*.

## Where the duration discrepancy enters

| Setting | Target+flush MAE % | Actual total MAE % | Mean abs writer−target ms | Max abs writer−budget frames |
| --- | ---: | ---: | ---: | ---: |
| stateful-unit | 16.8763 | 16.8345 | 2.0767 | 51 |
| stateful-shared | 10.6806 | 10.5941 | 2.4688 | 31 |
| stateful-length | 10.6838 | 10.5661 | 2.8977 | 32 |
| stateful-energy | 10.6806 | 10.5941 | 2.4688 | 31 |
| stateful-both | 10.6838 | 10.5661 | 2.8977 | 32 |

Raw WAV frames do not depend on an F0 extractor or alignment. The dominant
observed duration error is already in the portable requested timeline, not
introduced by its writer. This does not validate targets against PC or excuse
the missing PC rollback; it narrows the next experiment. All target/carry,
emitted/PCM and 4800-frame flush contracts pass on every experimental phrase.

## Gain-separated shape and energy diagnostics

Both pitch windows produce the same values below by construction; shape/energy
settings remain fixed. These distances are not numerically comparable to the
historical MFCC metric.

| Setting | Normalized shape MFCC-DTW ↓ | Aligned raw energy MAE dB ↓ |
| --- | ---: | ---: |
| baseline | 37.1928 | 3.0625 |
| stateful-unit | 37.3263 | 2.4960 |
| stateful-shared | 39.5029 | 2.4572 |
| stateful-length | 39.4263 | 2.4299 |
| stateful-energy | 39.5027 | 2.4570 |
| stateful-both | 39.4261 | 2.4298 |

Shared execution improves the diagnostic energy estimate but worsens shape.
Turning physical energy on at 0.002 has a very small effect. DTW can align wrong
phones; these are not perceptual scores. No objective multi-metric promotion.

## F0 coverage is part of the result, not a footnote

| Pitch window | Setting | Pooled matched coverage % | Phrases with matched F0 /22 | Conditional pYIN MAE cents |
| --- | --- | ---: | ---: | ---: |
| 1024 | baseline | 29.24 | 9 | 134.62 |
| 1024 | stateful-unit | 32.40 | 10 | 162.09 |
| 1024 | stateful-shared | 38.60 | 13 | 160.19 |
| 1024 | stateful-length | 38.13 | 13 | 158.96 |
| 1024 | stateful-energy | 38.60 | 13 | 160.19 |
| 1024 | stateful-both | 38.13 | 13 | 158.96 |
| 2048 | baseline | 32.96 | 11 | 154.51 |
| 2048 | stateful-unit | 49.89 | 15 | 203.48 |
| 2048 | stateful-shared | 40.82 | 15 | 182.16 |
| 2048 | stateful-length | 40.00 | 15 | 184.96 |
| 2048 | stateful-energy | 40.82 | 15 | 182.16 |
| 2048 | stateful-both | 40.00 | 15 | 184.96 |

Pooled coverage = summed matched frames / summed reference pYIN-voiced frames.
Conditional MAE = equal phrase mean over phrases with matched frames. Different
candidates admit different frames/phrases. **Do not use this table to declare
a speech F0 winner.** It remains diagnostic despite passing synthetic controls.

At 1024, reference IDs 002/005/006/007 have no admitted voiced frames. The
complete reports explicitly retain null errors and include V/UV mismatches,
missed/false-positive counts, gross/octave errors and independent AC results.
No missing score is substituted by zero. Identity comparisons pass for all
22 references at both windows, but cannot establish real-speech F0 accuracy.

Both calibration sets pass known tones, harmonic chirps, voiced/noise switches,
silence/noise rejection, half-gain separation and octave detection. The
2048 harmonic-chirp errors are 3.39 cents (pYIN) and 1.75 cents (AC), with 100%
interior admission. That narrower verified claim must not be generalized to
irregular synthetic speech or uncaptured PC pitch/voicing feature records.
