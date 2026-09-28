# M40: selective PSOLA coverage-edge repair (2026-09-28)

This checkpoint addresses one reproducible source of crackling in the native
port. It does not claim to fix swallowed phones, word stress, or intonation.
The stable profile is unchanged; `m40-transient` is a separate listening trial.

## Root cause

In the local 22-phrase comparison, the port WAV for phrase 021 (`2026 год`)
had a 24,576-unit adjacent-sample jump about 608 ms into speech. The original
WAV's largest jump was 14,813. The port jump was not at an external diphone
join. It was inside the voiced run of `A1->ch`, exactly where the TD-PSOLA
grain sum stopped covering samples and the renderer fell back to time-mapped
source samples. The decoded source unit's largest step was only 5,040. Thus
the jump was introduced by coverage-to-fallback switching, not present in the
voice recording. In this corpus, 185 of 468 SEG runs have some uncovered
samples, making the mechanism general even though most edges are not loud.

## Trial and decision

An initial broad blend at every uncovered edge reduced the example jump but
changed the duration of all 22 phrases (up to 358 samples), and degraded
mean MFCC-DTW from 52.01 to 53.07. It was rejected.

The narrower M40 trial acts only when a covered/uncovered boundary already
has a sample step greater than 6,000 before the usual 2x output gain. It
crossfades at most 32 samples on the covered side toward the existing
time-mapped source. It adds no silence and does not remove phones. The choice
of 6,000 is a conservative severe-transient screen, not a learned perceptual
threshold; a step alone cannot prove that a sound is an audible click.

| 22-phrase diagnostic | Stable | M40 selective |
| --- | ---: | ---: |
| Phrase 021 maximum sample step | 24,576 | 13,456 |
| Corpus steps above 16,000 | 8 | 7 |
| Corpus steps above 12,000 | 35 | 33 |
| Mean MFCC-DTW vs original, lower is better | 52.014 | 52.082 |
| F0 contour MAE, %, lower is better | 13.480 | 13.420 |
| Total-duration MAE, %, lower is better | 3.599 | 3.611 |
| Phrases with changed sample count | 0 | 6 (at most 56 samples) |

The stable WAVs rendered by the instrumented build were byte-identical to the
prior M38 stable pack for all 22 phrases. M40's F0 change is too small, and
the F0 metric too weakly voiced, to count as an intonation improvement.
MFCC/duration also move slightly in the wrong direction. This justifies an
opt-in listening test, not replacing stable. Sample-step counts are not
ground-truth click labels.

## Reproduce

Use the same private 22-phrase original reference pack and ignored Elan voice
files; neither is committed. Build `nicolai_batch_render`. Render the corpus
once without experimental flags and once with
`NICOLAI_M40_BLEND_UNCOVERED=1`. Run:

```powershell
python tools/audit_transients_m40.py `
  metrics-work/m40-stable-verify metrics-work/m40-selective-trial `
  --output metrics-work/new-m40-transient-report.json
python tools/measure_parity.py C:\path\to\reference_pack `
  metrics-work/m40-selective-trial --json metrics-work/new-m40-parity.json
```

`NICOLAI_AUDIT_TRANSIENTS=1` prints read-only per-unit/run `U` records in
the batch log, including uncovered sample counts and the largest step's
coverage weights. It is disabled in normal synthesis. The private WAVs, input
text, voice data and logs remain local. The general next boundaries are
phone-preservation analysis for missing sounds and original phone-feature
capture for stress/intonation; this crossfade does neither.
