# M43 — word-budget timing and limited dictionary-form stress

This is an **opt-in listening checkpoint**, selectable as `m43-word-rhythm`.
It inherits M42 join-pitch behavior, blends word-budget timing at 0.25, and
enables a limited dictionary-backed fixed-stem `-ика` form fallback. Stable
and all older profiles remain unchanged. There is no global slowdown, pitch
multiplier, phrase substitution, original DLL dependency or default promotion.

The user's report is a generally hurried delivery, plus a perceived emphasis
on `с` in `акустика`. Lexical selection and acoustic prominence are different
defects: fixing the former does not demonstrate that the latter is gone.

## What was found

The legacy duration path applies an average of two nominal phone durations
to a whole diphone. One phone is represented by the preceding diphone's right
side and the following diphone's left side, whose source extents may differ.
These two owners can consequently receive different scales. Utterance timing
alone cannot show whether stressed vowels, consonants and word endings have
the right relative durations.

`plan_word_rhythm_m43()` starts from the existing post-policy side scales.
For each eligible word it sums that word's spoken source budget, distributes
it proportionally to nominal `duration.par` weights, and derives one shared
target scale per phone. Both source owners blend linearly toward that target.
One common blend amount per word conserves the pre-join budget, including
when any side hits a scale/change bound. Quiet `#` sides are untouched;
spoken sides of boundary diphones can change. Actual waveform length after
PSOLA rounding, overlap and trimming is **not** conserved by this guarantee.

Limits are 0.2..3 per side and a maximum 1.5x change from its baseline.
Missing/nonpositive duration weights, insufficient support, a single-phone
word, or a zero usable blend skip the whole word. Invalid structural/numeric
input returns an invalid plan without modifying the copied units; the caller
retains its baseline. Zero strength preserves the old rendering path exactly.
The selected M15 SEG split is still approximate; this is not recovered
original feature-builder or shared phase-clock parity.

## Stress diagnosis and narrow correction

Using the supplied dictionary and M42:

```
акустика / аку́стика: # a1 k u0 s t' i4 k a4 #
акустику:           # a1 k u4 s t' i1 k u0 #
```

The isolated nominative already selects stressed `u0`, and ordinary versus
explicitly accented spelling gives identical WAVs. No consonant is selected
as a lexical stress vowel. This does **not** invalidate the heard emphasis
on `с`: timing, spectral energy and surrounding transitions remain suspects.

The inflected form is genuinely wrong: the original exception file has an
entry for the lemma, not that form, and the port falls back to the last vowel.
The exception file documents `/i` as case-insensitive matching, not an
inflection flag; its instructions require whole-word entries. Do not interpret
it as a morphology table or mutate the supplied dictionary to hide this gap.

M43 adds an opt-in fixed-stem `-ика` family lookup for `-ику`, `-ике`, `-ики`,
`-икой`, `-икою`. It requires a real exact `-ика` anchor with stress inside
the stem and only lowercase Russian letters. Any known family form or `-ик`
homonym with conflicting stress refuses the missing-form fallback. Compounds,
unknown anchors, ending-stressed anchors and arbitrary `-а/-у` conversions
are not guessed. Exact dictionary forms, explicit accents and `ё` retain
priority. The helper is a conservative reconstruction, **not** the original
Russian NLP or a general morphology/stress solution.

The final M43 startup form becomes:

```
акустику: # a1 k u0 s t' i4 k u4 #
```

Voice-free tests also cover `физику`, `логику`, conflicting forms and masculine
anchors, absent anchors, unsupported endings, explicit override and old-path
isolation. Local voice wrapper contracts require the four-word ordinary
stress probe and its explicitly accented version to be byte-identical only
in M43. No example word is added to the builtin dictionary.

## Evidence — mixed, not a solved tempo problem

The same 22 original WAVs were used for ten timing configurations. The final
stress-enabled selected mode matches its timing-only quarter-strength trial
22/22 because none of these corpus words invokes the new family fallback.

| 22-phrase diagnostic | M42 | M43 selected |
|---|---:|---:|
| Historical MFCC-DTW | 51.77676 | 51.77783 |
| Active waveform correlation | 0.18111 | 0.20054 |
| Active duration MAE, % | 5.48049 | 5.67654 |
| Total duration MAE, % | 3.32527 | 3.46890 |
| RMS-ratio MAE, % | 10.34222 | 9.30761 |
| PCM steps over 12000 | 33 | 36 |
| PCM steps over 16000 | 5 | 5 |
| DTW-mapped excess quiet at `#`, ms | 2055.4 | 2091.3 |

This does not prove an overall improvement: both timing errors and quiet-gap
screen regress. F0 is diagnostic only (13.01931 -> 14.47697%); sparse reliable
voicing coverage prevents using it as a promotion gate.

Three user-provided phrases have SHA256-verified **older successful original
captures**, not fresh proof that SAPI now works. Final M43 versus M42 has
normalized shape cost 44.507 -> 43.780 (two lower, one higher), mapped `#`
excess quiet 473.6 -> 453.6 ms, total quiet 1740 -> 1720 ms, and duration MAE
108.633 -> 127.333 ms. The startup loses 851 samples after the lexical
correction: 67141 -> 66290. Its correct stress does not bring total duration
closer to the original. Steps over 12000 rise 14 -> 15; over 16000 stay 0.
The lower-threshold `курлык` maximum jump increases 4318 -> 7784, so even the
absence of new >16000 outliers is not evidence of complete click repair.

Stronger redistribution (0.5/1.0), eighth-strength, M38 boundary combinations
and the alternative PC SEG timeline were screened separately. Boundary and
timeline variants improve some shape/quiet statistics but introduce severe
steps on the user corpus: M38 alone 1, quarter+M38 3, timeline alone 2,
quarter+timeline 4 over 16000 versus M42's zero. They are not GUI presets.
Keep them reproducible batch ablations, not candidate defaults.

Historical parity MFCC and the gain-normalized rhythm auditor use different
FFT/scaling conventions. Their numeric costs must not be compared to one
another. Quiet intervals are not phone labels; DTW can align different
phones, and `#` grouping includes punctuation. Pre-join budgets are not
final waveform or perceived rhythm measurements.

Scalar evidence, all trial summaries/flags and validation counts are in
[the M43 metric artifact](metrics/m43-rhythm-stress-20261001.json). Private
WAVs, voice dictionaries, detailed text/logs and binaries are not in Git.

## Reproduce

Use a matching compiler developer shell; build the existing CMake targets.
Clear inherited `NICOLAI_*` flags and create a fresh output parent first.

```powershell
Get-ChildItem Env:NICOLAI_* | Remove-Item
$env:NICOLAI_M40_BLEND_UNCOVERED='1'
$env:NICOLAI_M41_PRESERVE_JOINS='1'
$env:NICOLAI_M41_INTERPOLATE_UNCOVERED='1'
$env:NICOLAI_M42_JOIN_PERIOD_CONTINUITY='0.5'
$env:NICOLAI_M42_LOCAL_JOIN_PITCH='1'
./build/nicolai_batch_render.exe voice-data/nicolai16.dat voice-data/exc_rus.txt `
  voice-data/abb_rus.txt tools/parity_corpus_22.tsv out/m43-baseline > out/m43-baseline.log

$env:NICOLAI_M43_WORD_RHYTHM='0.25'
$env:NICOLAI_M43_FIXED_IKA_STRESS='1'
./build/nicolai_batch_render.exe voice-data/nicolai16.dat voice-data/exc_rus.txt `
  voice-data/abb_rus.txt tools/parity_corpus_22.tsv out/m43-trial > out/m43-trial.log

python tools/audit_rhythm_m43.py --original PRIVATE_REFERENCE_PACK `
  --baseline out/m43-baseline --trial out/m43-trial `
  --baseline-log out/m43-baseline.log --trial-log out/m43-trial.log `
  --corpus tools/parity_corpus_22.tsv --output out/m43-rhythm.json
python tools/audit_transients_m40.py out/m43-baseline out/m43-trial `
  --output out/m43-transients.json
python tools/measure_parity.py PRIVATE_REFERENCE_PACK out/m43-trial `
  --json out/m43-parity.json
python tools/test_rhythm_m43.py
```

The two new flags are independent in batch experiments. Omit the stress flag
to reproduce the timing-only trials. To reproduce ablations, use rhythm 0,
0.125, 0.25, 0.5 or 1 and optionally
`NICOLAI_M38_BOUNDARY_SPEECH_SHARE=0.5` or `NICOLAI_PC_SEG_TIMELINE=1`.
Do not set both ablation flags unless running an additional, unmeasured test.
`tools/stress_corpus_m43.tsv` provides fresh synthetic accent probes.

`WT` records report first/last phone, pre-join budgets and effective strength.
`W` records independently expose word index, vowel-stress ordinal and source;
existing `P`, `D`, `J` formats are unchanged. GUI logs include word-budget
records and equivalent `stress_word=word-index,vowel-ordinal,source` records.
All output targets used for evidence must be fresh.

Local complete suites pass x86 static Release 32/32 and x64 Debug 31/31.
The eight port profiles pass startup GUI jobs and 64 CLI/batch PCM comparisons,
including deliberate inherited-flag contamination and explicit-accent probes.
Stable/M42 match their old 22-WAV outputs. Final M43 x86/x64 matches 22/22;
the executable imports Windows system DLLs only. These contracts establish
implementation/profile consistency, not human listening superiority.

## Next boundary

Compare M42 and M43 on identical long sentences at unchanged settings. Track
`акустика` separately from `акустику`: the first still needs acoustic prominence
analysis even though its lexical stress is correct. Capture original phone
durations and authored pitch/energy features before broad stress/morphology
or phase-clock integration. In particular, unknown forms elsewhere in the
startup phrase still use the old last-vowel heuristic; this checkpoint does
not claim to fix all stress, phrase melody, hurried delivery or residual clicks.
