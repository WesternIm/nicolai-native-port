# M41: short-feature protection and fractional fallback (2026-10-01)

This is an opt-in listening checkpoint for swallowed sounds and crackling,
not a claim of complete phone accuracy or perceptual superiority. Stable and
M40 remain selectable. No phrase-specific replacements, global denoiser,
limiter, pitch changes or new duration multiplier are used. Private voice
files, original WAVs and rendered test WAVs remain local.

## Two mechanisms

1. The old correlation join treats an unknown/nonperiodic edge as an 80-sample
   period at 16 kHz and may trim up to 40 samples from each side. A synthetic
   matching-noise fixture proves that a 20,000-amplitude short release can be
   entirely removed by `right_trim`; its mirrored version proves the same
   for `left_trim`. M41 checks the part of each nonperiodic edge threatened by
   the selected trim/window. A 1-ms energy window must contain at least 55%
   of edge energy, peak amplitude must be at least 1,500, and the old window
   must retain less than 75% at that window's center. Only then are trims
   disabled and the overlap capped at 1 ms. This protects a short feature,
   not a guaranteed phoneme: period hints can be unavailable even on vowels.
   Ordinary stationary noise and periodic joins stay unchanged in the tests.
2. Uncovered PSOLA samples formerly use `floor(source_time)`. Compression can
   skip source samples and produce a larger step than the recording contains.
   M41 linearly samples the fractional source time, bounded to the source
   endpoints. Covered grains and unvoiced OLA stretching are unchanged. The
   same fractional mapping supplies M40's selective coverage-edge fade.
   Both uniform and unequal phone-side duration paths are covered by tests.

In the measured `l->a0` unit of phrase 015, the decoded source maximum step
is 8,512, M40's rendered unit has 12,752, and fractional fallback has 9,708
(about 24% less, before output gain). This is a unit-level reduction. The
complete phrase maximum remains 16,408 because other transients dominate.
Do not relabel all source plosive/fricative steps as artificial clicks.

## Rejected experiments and selected policy

Blanket no-trim/1-ms-overlap at every unknown internal/external edge greatly
lengthens speech: internal-only total-duration MAE rises from 3.611% to 5.44%,
and both together to 8.19%. It was rejected and is not in the code.

Selective internal protection is retained as an independent batch ablation,
but not enabled in the listening profile. It triggers on 18/22 phrases, changes
length by up to 847 samples, reduces active-waveform correlation from 0.192713
to 0.185791, and raises active-duration MAE from 5.708% to 6.103%.
Even though total-duration MAE is slightly better, that does not establish
sound-preservation superiority.

The GUI profile `m41-preserve` enables M40 blending, selective **external**
protection, and fractional uncovered sampling. Internal protection is off.
External protection alone triggers once in the 22-phrase corpus, near the
final `a4` of phrase 017, preserving 101 additional samples. With fractional
sampling, downstream phase search can also change trims, so utterance lengths
are not guaranteed identical even though resynthesis target lengths are.

| Diagnostic vs the same private 22 originals | M40 | M41 selected |
| --- | ---: | ---: |
| Mean MFCC-DTW, lower is better | 52.082176 | 51.880602 |
| Active-waveform correlation, higher is better | 0.192713 | 0.194194 |
| Active-duration MAE, % | 5.707960 | 5.710210 |
| Total-duration MAE, % | 3.610891 | 3.612154 |
| F0 contour MAE, % (diagnostic only) | 13.419612 | 13.468998 |
| RMS-ratio MAE, % | 10.281684 | 10.293207 |
| Steps greater than 12,000 | 33 | 33 |
| Steps greater than 16,000 | 7 | 7 |

M41 changes length on 10/22 phrases by at most 110 samples (6.875 ms).
Maximum step rises on two phrases, without creating new >12k/>16k outliers.
On the three additional saved-text cases, one length changes by -94 samples;
maximum steps do not rise, but >12k steps increase from 11 to 12. Thus the
small spectral metric gain and synthetic deletion fix justify an experiment,
not promoting it over stable or claiming that real swallowed phones are fixed.
No human listening score or intelligibility annotation was collected here.
F0 is not reliable enough to claim intonation progress. Stress is untouched.

## Reproduce and continue

Build `nicolai_batch_render` and run `tools/parity_corpus_22.tsv` with ignored
voice inputs. Clear inherited `NICOLAI_*` experimental variables first.
For the selected trial set:

```powershell
$env:NICOLAI_M40_BLEND_UNCOVERED='1'
$env:NICOLAI_M41_PRESERVE_JOINS='1'
$env:NICOLAI_M41_INTERPOLATE_UNCOVERED='1'
$env:NICOLAI_M41_PRESERVE_RUNS='0'
```

For M40 baseline use only the first flag. For independent ablations, enable
one M41 flag at a time. The batch log's `M41` record contains phrase ID,
protected internal count, protected external count. `NICOLAI_AUDIT_TRANSIENTS=1`
adds per-unit/run `U` diagnostics. GUI job logs include the protection counts.
Run `tools/audit_transients_m40.py baseline trial --output fresh-report.json`
and `tools/measure_parity.py private-reference-pack trial --json parity.json`.
The aggregate checkpoint is in `docs/metrics/m41-preservation-20261001.json`.

Tests exercise actual preserved/deleted PCM, mirrored tail deletion,
stationary-noise/voiced bypass, endpoint-safe interpolation, covered-grain
identity, and piecewise/unvoiced policy isolation. The existing Windows
wrapper test checks all six port profiles against batch PCM, with inherited
flags intentionally contaminated. Public CI needs no proprietary inputs.

Local verification passed 30/30 x64 Debug tests and 31/31 x86 Release tests.
Fresh x86 renders match the x64 M41 trial byte-for-byte on all 22 phrases;
stable and M40 match their previous 22-WAV baselines byte-for-byte as well.

Next: collect exact words/time spans where M41 still swallows a sound or
crackles, retain paired original/M40/M41 WAVs, and attribute them to a source
SEG run, uncovered map, or join before changing more parameters. Internal
run boundaries need their own timing-safe construction, not blanket short
overlaps or a global click filter. Promotion requires listening and a broader
independent corpus, not just an improved MFCC aggregate.
