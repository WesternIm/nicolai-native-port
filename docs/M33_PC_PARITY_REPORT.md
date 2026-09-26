# M33 PC parity evaluation — no production promotion

The exact SEG expansion and Q11 step primitive passed direct original-DLL
checks. The experimental portable execution adapter did **not** pass the
audio gate. Production keeps `use_pc_seg_timeline=false`,
`search_join_phase=true`, physical length/energy strengths **0.0**, and the
unchanged M32 pitch policy. Its output is SHA256-identical to M32 for **22/22**
golden corpus phrases.

## Evidence and definitions

- Clean Debug x64 build: **20/20 CTest tests**; assertions are enabled.
- Original 32-bit DLL oracle: **2665/2665** complete node timelines and
  **683/683** synthesis-step records match.
- Seven configurations, **22/22 WAVs each** (154 local renders).
- Same PC pack `reference_pack_20260924_222948`, voice Nicolai 16 kHz.
- Same `measure_parity.py` definitions as committed M32: active waveform
  correlation, active/total duration, 12-bin normalized F0 contour MAE,
  active-region 12-coefficient MFCC-DTW, active-region RMS ratio/error.
- No golden/reference/rendered PCM, DLL, dictionary, or voice database is
  committed. Reports contain only phrases, scalar metrics, and hashes.

MFCC here uses the reproducible M32 definition (baseline **52.01387**), not the
earlier undocumented one-off number 128.5899. Those scales cannot be compared.

## Aggregate results

Higher correlation is better; all other columns are errors/distances, lower is
better. Duration, F0, and RMS errors are percentages.

| Setting | Wave corr | F0 MAE | Active duration MAE | Total duration MAE | MFCC-DTW | RMS MAE |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| M32 baseline | 0.196516 | 13.4799 | 5.6790 | 3.5987 | 52.0139 | 10.2377 |
| PC timeline adapter | 0.198696 | 15.3872 | 5.6521 | 3.3084 | 48.5336 | 9.5517 |
| No phase search | 0.185113 | 15.9572 | 10.5461 | 6.0662 | 53.6274 | 10.0947 |
| Timeline + no phase search | 0.196332 | 15.1693 | 10.5172 | 6.0756 | 49.2071 | 9.4285 |
| Timeline + [l] 0.002 | 0.201392 | 15.4613 | 5.6498 | 3.3015 | 48.8457 | 9.5533 |
| Timeline + [e] 0.002 | 0.198694 | 15.3872 | 5.6521 | 3.3084 | 48.5332 | 9.5687 |
| Timeline + [l]/[e] 0.002 | 0.201391 | 15.4613 | 5.6498 | 3.3015 | 48.8451 | 9.5703 |

Mean active-duration ratio moves from 0.99255 to 0.99768 with the timeline.
Removing phase search raises it to 1.09608 (old timeline) or 1.09720 (PC
timeline): deleting phase trims alone exposes a substantial clock change.
Mean RMS ratio is 0.93440 for baseline and 0.93336 for the PC timeline; ratio
closeness of the mean must not be confused with mean per-phrase RMS error.

## Per-phrase audit and decision

PC timeline versus baseline:

| Metric | Improved phrases | Regressed phrases |
| --- | ---: | ---: |
| Correlation | 9 | 13 |
| Active-duration error | 11 | 11 |
| Total-duration error | 15 | 7 |
| F0 contour error | 8 | 14 |
| MFCC-DTW | 22 | 0 |
| RMS error | 15 | 7 |

The spectral improvement is consistent, but F0 worsens by **1.9072 percentage
points**. The highest-correlation length candidate worsens F0 by **1.9814 pp**.
Neither is a safe all-round replacement for M32. Energy at 0.002 slightly
worsens RMS error relative to the timeline-only candidate.

`assess_m33_candidates.py` applies a conservative reproducible rule: no
aggregate regression above 1e-9 on any of the six metric families and at least
one improvement. It validates complete unique 22/22 successful reports and
finite metrics first. **No candidate qualifies.** This fixed corpus is a
selection/evaluation pack, not independent held-out proof of generalisation.

## Committed evidence

`docs/metrics/m33-*.json/csv` includes all seven per-phrase reports, sweep
settings/summary, the aggregate gate, timeline deltas, original-DLL oracle
counts, baseline identity hashes, and source/input/dependency fingerprints. Local WAVs/logs are under ignored
`metrics-work/m33-final/`. Reproduction commands are in `M33_CONTINUATION.md`.

The next missing layer is not another phrase-specific tuning table: it is the
stateful shared-half-phone regulator and synthesis-mark/windowed join. M34
should connect the verified source-node and Q11 primitives there, then re-test
the existing phone-local [l]/[e] path.
