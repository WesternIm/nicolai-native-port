# M34 parity and measurement audit

**No production promotion.** Exact primitive checks pass, but the experimental
stateful adapter has real clock errors. The user's metric-validity question
also exposed material weaknesses in F0 and RMS interpretation. Historical
metrics remain unchanged for comparability, not treated as perceptual truth.

## Validation

- Clean Debug MSVC x64 build: 20/20 host tests, assertions enabled.
- Win32 original DLL: 1024/1024 window writes, 400/400 reciprocal interpolation
  cases, 1000/1000 sequential carry records; rerun M33 2665/2665 timelines and
  683/683 isolated records.
- Six settings, 22/22 WAVs each; same original Nicolai 16-kHz PC pack.
- Stable baseline versus saved M33: 22/22 raw WAV SHA256 identical.
- All new stateful/shared-duration switches and physical [l]/[e] strengths
  remain disabled by default; no phrase-specific rules.

## Historical metric results

Correlation is higher-is-better. Error percentages and MFCC-DTW are
lower-is-better. F0 is historical 55–150 Hz YIN with an RMS gate; RMS in this
table is **whole-WAV**, including silence, not active speech.

| Setting | Corr | F0 MAE % | Active MAE % | Total MAE % | MFCC-DTW | RMS MAE % |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Baseline | 0.196516 | 13.4799 | 5.6790 | 3.5987 | 52.0139 | 10.2377 |
| Stateful, unit duration | 0.199750 | 13.8641 | 26.9387 | 16.8345 | 53.1790 | 9.3399 |
| Stateful, shared duration | 0.207715 | 15.8847 | 16.5177 | 10.5941 | 55.7325 | 7.4986 |
| Shared + [l] 0.002 | 0.208550 | 15.7186 | 16.5026 | 10.5661 | 56.0231 | 7.3897 |
| Shared + [e] 0.002 | 0.207704 | 15.8847 | 16.5177 | 10.5941 | 55.7304 | 7.5160 |
| Shared + both 0.002 | 0.208539 | 15.7186 | 16.5026 | 10.5661 | 56.0210 | 7.4071 |

Mean active-duration ratio: baseline 0.99255, unit adapter 1.26939, shared
adapter 1.10538. Mean whole-WAV RMS ratio: 0.93440, 0.95452, 0.95121 respectively.
Shared adapter per-phrase improved/regressed: correlation 11/11, active
duration 5/17, total duration 6/16, historical F0 7/15, MFCC 6/16, RMS 13/9.
The unchanged conservative historical gate selects no candidate.

## Are the metrics themselves misleading?

**Partly, and materially for F0.** `audit_parity_metrics.py` calibrates the
metrics without modifying inputs or redefining historical reports.

Identity calibration on all 22 reference WAVs gives correlation 1 (floating
error <=2.3e-16), zero lag, zero duration/F0/MFCC/RMS error. There is no generic
self-comparison bug. However only 229/264 F0 bins are covered, and cross-render
comparison covers 220/264 for baseline/shared. The metric averages available
bins, not all bins, and does not penalize missing coverage.

In-memory controls on reference phrase 001:

| Control | Observed result | Interpretation |
| --- | --- | --- |
| Gain x0.5 | Corr 1, F0 error 0, RMS ratio 0.5, MFCC 61.289 | MFCC includes level effects, not pure timbre |
| Polarity flip | Corr 0.1632, F0/MFCC/RMS errors 0 | Correlation measures raw waveform agreement, not perceptual voice identity |
| Append 300 ms silence | Active metrics unchanged; total RMS ratio 0.8640 | Whole-WAV RMS confounds loudness with silence/duration |
| Pure tone 166 Hz | Historical F0 83.14 Hz; wide-band 166.53 Hz | Narrow band can force octave/subharmonic errors |
| Pure tone 220 Hz | Historical F0 110.14 Hz; wide-band 220.53 Hz | Same failure reproduced above 150 Hz |

Broadening only YIN fmax to 400 Hz reverses the M34 F0 ranking:

| F0 extraction | Baseline MAE % | Shared adapter MAE % |
| --- | ---: | ---: |
| Historical 55–150 Hz | 13.4799 | 15.8847 |
| Sensitivity 55–400 Hz | 26.7358 | 20.8267 |

This is evidence that the **ranking is extractor-sensitive**, not proof that
the wide-band estimate is correct. Both use YIN without explicit voicing
classification, equal normalized-time bins rather than aligned phones, and
candidate-dependent energy gating. Neither is authoritative F0 ground truth.
RMS over the active bounding span (still including internal pauses) improves
10.7949% -> 8.6960% for the shared adapter. That is a more isolated energy
check, but not a time-aligned loudness-envelope or voiced-frame comparison.

The same audit was rerun on saved M33 renders: the standalone PC-timeline
adapter remains worse than baseline under both narrow and wide F0 definitions
(wide 29.0806% vs 26.7358%). Timeline + no phase search reverses its F0 ranking
under the wide setting, but still has worse raw total-duration error.
Thus M33's measurement limitations should be acknowledged without claiming
that every rejected candidate was actually superior.

## What remains a real regression

Raw total duration uses WAV frame counts divided by sample rate, not active
thresholds, pitch tracking or amplitude normalization. The audit independently
reproduces baseline error 3.5987% versus shared adapter 10.5941%.
The spectral error also rises under the unchanged MFCC implementation.
Therefore measurements do not justify enabling this incomplete state machine.
The F0 regression claim alone is withdrawn as a reliable quality verdict.

Next evaluation work should maintain historical and calibrated metrics side
by side: prove F0 range/voicing against a second estimator or original feature
traces, report octave-error and coverage rates, use common phone/time alignment,
separate level-normalized spectral distance from energy, and inspect/listen to
paired outputs. Do not simply replace one arbitrary fmax with another and
declare the engine fixed. The 22 selection phrases are not held-out evidence.

## Tracked evidence

`docs/metrics/m34-*.json/csv` contains all six per-phrase reports, summary,
historical gate, shared per-phrase deltas, identity hashes, exact-oracle counts,
M34 and M33 metric calibration/sensitivity audits, and a source/input manifest.
No WAV, original DLL, disassembly dump or voice database is committed.
