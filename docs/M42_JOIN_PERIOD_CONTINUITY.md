# M42: local join pitch hints and guarded endpoint reconciliation (2026-10-01)

This is an opt-in listening checkpoint, not a stable promotion or a complete
intonation/click fix. It addresses two independently testable join mechanisms.
The existing stable and M41 profiles remain byte-identical on all 22 reference
texts. Private voice inputs, original WAVs and rendered WAVs remain local.

## What was inconsistent

Each diphone has its own recorded carrier period and authored pitch triplet.
Even when two recordings represent the same shared phone, their edge target
periods can differ. A correlation-based waveform join does not by itself make
their pitch clocks continuous. The new `JP` telemetry reports SEG edge period
divided by authored endpoint pitch ratio, not measured output F0.

In the M41 baseline, 137 of 265 external joins have two positive hints; 128
do not. The eligible mean absolute gap is 157.985 cents, with 68 above 100
cents and 36 above 200. Some hints are implausible: one shared-phone edge in
phrase 017 implies 46.24 versus 177.91 samples, a 2332.71-cent gap. It must
not be forced to a common pitch as if it were reliable F0.

There is also a concrete coordinate mismatch in the uniform M15 run join:
the generated runs use the contour at their local source positions, but the
join's period hints formerly use positions 1 and 0 of the complete diphone.
M42's independent `local_join_pitch_m42` switch uses the adjacent runs'
`source_end / source_length` and `source_begin / source_length` instead.
The unequal phone-side M32 path already uses local positions.

## Guarded correction, not a shared phase clock

All existing per-unit pitch parameters are authored before rendering. For
each non-`#` shared phone with voiced edge hints, an optional helper reconciles
the left owner's end and right owner's start toward the geometric mean of
their target periods. It does not change either middle pitch parameter or
the opposite endpoints; uniform contours become three-point contours only
when a correction is accepted. At strength 0.5, the authored cent gap halves.

At the voice's 16-kHz sample rate, the screen accepts target periods only
between 80 and 400 samples and gaps between 80 and 700 cents. Missing,
nonpositive, nonfinite, very short/long and octave-like hints are skipped.
All queried contour scales must be finite and between 0.05 and 8. Strength
must be finite and positive, and is capped at 1. This is a conservative
experimental screen, not a recovered rule from the original engine.

The helper leaves authored duration and gain controls unchanged. Actual PCM
inside a unit can still change because its synthesis marks move. Later phase
search/overlap trimming can also change utterance length. Neither mechanism
implements a persistent phase clock across diphones, corrects lexical stress,
or recovers the original feature/coefficient builder. M36 remains off.

## Independent trials against the same 22 original WAVs

Every trial inherits the selected M41 policy: M40 coverage-edge blending,
external short-feature protection, and fractional uncovered sampling. M41
internal protection remains off. No new global duration multiplier is used.

| Trial | Endpoint strength | Local run hints | MFCC-DTW | Total-duration MAE, % | F0 MAE, %* | >12k steps | >16k steps |
| --- | ---: | :---: | ---: | ---: | ---: | ---: | ---: |
| M41 baseline | 0 | off | 51.880602 | 3.612154 | 13.468998 | 33 | 7 |
| Quarter | 0.25 | off | 52.208377 | 3.481496 | 13.201025 | 34 | 7 |
| Half | 0.5 | off | 51.800985 | 3.482175 | 13.067699 | 33 | 5 |
| Full | 1 | off | 50.969099 | 3.344826 | 13.722111 | 34 | 4 |
| Local only | 0 | on | 51.893296 | 3.459315 | 13.458837 | 33 | 7 |
| **M42 listening profile** | **0.5** | **on** | **51.776757** | **3.325269** | **13.019311** | **33** | **5** |
| Full + local | 1 | on | 51.114150 | 3.253437 | 13.794784 | 34 | 4 |

*The historical F0 metric is diagnostic only; limited coverage and octave
ambiguity prevent using it as a promotion gate. PCM-step counts screen
transients, including legitimate source releases, not ground-truth clicks.

The partial profile is exposed as `m42-join-pitch` for listening, not selected
as a proven optimum. Full correction has better aggregate MFCC/timing but
worse diagnostic F0 and more >12k steps. The selected trial improves MFCC on
only 10/22 phrases; 12 worsen despite the slightly better mean. Total-duration
error improves on 17/22, active-duration error on 14/22. Active-waveform
correlation falls from 0.194194 to 0.181106, and RMS-ratio MAE rises from
10.293207% to 10.342224%. All 22 lengths change, by at most 180 samples
(11.25 ms); maximum PCM step rises on six phrases. These are material limits.

On three additional saved-text cases, >12k steps rise from 12 to 14; >16k
steps remain zero. All lengths change, by at most 114 samples (7.125 ms).
One maximum rises from 14714 to 14762; another falls from 15774 to 15488.
No paired-original spectral or human listening score is claimed for these
additional cases. The patch does not establish that real swallowed phones
or every audible crackle have been fixed.

## Coverage-aware period audit

The new auditor uses 40-ms windows outside each overlap, removes DC, and
requires RMS >=100 and autocorrelation >=0.75 in a 50..300-Hz lag search.
It prefers the shortest nearly equally strong interior peak and refines it
parabolically. Silence, short windows, noise and missing tracks stay missing.
Original coordinates are MFCC-DTW estimates, not original phone labels.

The selected profile halves 79 accepted authored gaps; none increase. On the
same 137 hint-eligible joins, mean gap falls from 157.985 to 95.468 cents.
That is a parameter-level result, not proof of the same improvement in PCM.
Autocorrelation yields both sides on only 18/265 baseline joins and 19/265
trial joins. Their intersection is just 13/265: mean gap is 125.639 ->
122.538 cents, with five lower, three higher, five within one cent. The other
252 joins are excluded, not counted as success. Unpaired eligible means must
not be compared. Original DTW estimates also have sparse, changing coverage
(37 baseline, 33 trial); they are not a truth/promotion gate.

For the additional three cases, authored matched coverage is 39/65, mean gap
133.613 -> 77.381 cents (20 lower, none higher). PCM matched coverage is only
9/65, 108.607 -> 102.426 cents (three lower, one higher). No original track
was passed to that audit.

## Reproduce

Clear inherited `NICOLAI_*` experimental variables in this shell first. Build
`nicolai_batch_render` with CMake, then set the M41 baseline:

```powershell
$env:NICOLAI_M40_BLEND_UNCOVERED='1'
$env:NICOLAI_M41_PRESERVE_JOINS='1'
$env:NICOLAI_M41_INTERPOLATE_UNCOVERED='1'
$env:NICOLAI_M41_PRESERVE_RUNS='0'
$env:NICOLAI_AUDIT_JOIN_PERIODS='1'
./build/nicolai_batch_render.exe voice-data/nicolai16.dat voice-data/exc_rus.txt `
  voice-data/abb_rus.txt tools/parity_corpus_22.tsv out/m42-baseline > out/m42-baseline.log

# Selected trial; change these two independently for the ablations above.
$env:NICOLAI_M42_JOIN_PERIOD_CONTINUITY='0.5'
$env:NICOLAI_M42_LOCAL_JOIN_PITCH='1'
./build/nicolai_batch_render.exe voice-data/nicolai16.dat voice-data/exc_rus.txt `
  voice-data/abb_rus.txt tools/parity_corpus_22.tsv out/m42-trial > out/m42-trial.log

python tools/audit_join_periods_m42.py --portable out/m42-baseline `
  --join-log out/m42-baseline.log --original PRIVATE_REFERENCE_PACK `
  --output out/m42-baseline-periods.json
python tools/audit_join_periods_m42.py --portable out/m42-trial `
  --join-log out/m42-trial.log --original PRIVATE_REFERENCE_PACK `
  --baseline-audit out/m42-baseline-periods.json --output out/m42-trial-periods.json
python tools/audit_transients_m40.py out/m42-baseline out/m42-trial `
  --output out/m42-transients.json
python tools/measure_parity.py PRIVATE_REFERENCE_PACK out/m42-trial `
  --json out/m42-parity.json
python tools/test_join_periods_m42.py
```

Create the `out` parent first; WAV/report targets must be fresh. Build layouts
with configuration subdirectories use `build/Release` or `build/Debug`.
`M42` batch records and GUI logs report accepted reconciliation counts;
the existing `J`/GUI join-record format stays unchanged. Scalar aggregate
evidence is in [the M42 metric artifact](metrics/m42-join-period-20261001.json).

Release-active C++ contracts verify the geometric period target, residual
cent gap, unchanged middle/opposite parameters, zero/invalid/octave bypass,
strength capping, and local contour coordinates. Six voice-free Python
tests cover known frequencies, gain/DC/polarity, harmonic octave controls,
missing data, log validation, and mutually eligible paired comparisons.
Local x64 Debug and x86 static Release suites pass 30/30 and 31/31. All seven
port profiles pass startup GUI jobs and 42 CLI/batch byte comparisons with
deliberately contaminated inherited flags. Stable and M41 each match their
previous 22-WAV baselines; x64 Debug and x86 Release M42 match 22/22.

## Continuation boundary

Listen to identical text in M41 and M42 before promoting anything. Retain
exact word/time spans and original/M41/M42 WAVs for residual breaks. Next
separate phase-clock discontinuity, wrong/ambiguous SEG hints, and lexical
prosody using source/run coordinates and original feature capture. A shared
phase-clock experiment needs duration/phoneme-preservation guardrails and
an independent corpus; lowering parameter gaps alone is insufficient.
