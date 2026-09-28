# M38: opt-in connected-word boundary trial (2026-09-28)

This is a reversible acoustic experiment, not original-runtime parity. The
stable renderer remains the default and must remain byte-identical when the
new strength is zero. The Windows talker exposes the candidate as
`m38-boundary`, beside stable, M36 local/chain, and original SAPI.

## Evidence and causal scope

`tools/audit_talker_pair_alignment.py` aligns original and stable WAVs on
RMS-normalized MFCC 1..12, 10-ms hop, with a 20% constrained DTW band. It maps
portable low-energy intervals to reference spans and reports the quiet amount
inside each mapped span. A word-boundary gap is counted when the nearest join
is `#` *and its center lies in the gap*. This improves on raw nearest-join
proximity, but it is still a heuristic: DTW can map the wrong phone, and stop
closures are not annotated. `tools/test_talker_pair_alignment.py` exercises
identity/gain alignment, exact quiet overlap, and batch-log parsing with
synthetic signals.

Across the 10 multiword items in the local 22-phrase reference corpus, stable
had 27 such word-boundary gaps. Many were mapped to reference spans containing
little or no silence. In the user-owned short phrase «ох бля», the stable port
has a 120-ms gap at the internal `#` join; its DTW-mapped original span has
zero quiet samples. By contrast, «Курлык курлык» has a substantial gap in both
recordings. All raw voice data, corpus text, WAVs and detailed alignment JSON
stay in ignored local directories; none is committed.

## Tested interventions

Simply lowering `word_boundary_scale` from 0.45 to 0.20 shortened the word
gaps, but damaged timing. On the 10 multiword reference phrases, mean absolute
total-duration error rose from 59.72 to 273.51 ms. It was rejected.

M38 instead keeps each existing word-boundary diphone's target duration. For
an internal *plain word* `#`, it decreases the duration allocated to the `#`
side and gives the same samples to the spoken side of each adjacent diphone.
It uses the existing phone-side resynthesizer and preserves the target sum;
actual utterance length can still shift slightly because join trimming and
rounding depend on the changed waveform. Comma, sentence, utterance edges,
hyphen, and stateful M36 do not use this trial. Strength 0.5 is opt-in; zero
leaves the old code path intact.

| 10 multiword reference phrases | Stable | M38 strength 0.5 | Shorter whole boundary 0.20 (rejected) |
| --- | ---: | ---: | ---: |
| Mean MFCC-DTW shape cost | 45.0761 | 44.2014 | 41.7949 |
| Mean absolute total-duration error, ms | 59.72 | 59.70 | 273.51 |
| Aligned excess quiet at word joins, ms | 1971.3 | 1144.0 | 350.0 |
| Word-join quiet intervals | 27 | 26 | 17 |
| All within-speech quiet time, ms | 4160 | 3340 | 2540 |

M38 improves MFCC-DTW in 8/10 changed corpus phrases; two worsen. The scalar
summaries and per-phrase values are in
[`m38-boundary-trial-20260928.json`](metrics/m38-boundary-trial-20260928.json).

Three separate user-owned A/B pairs were also checked, using the original WAV
from each successful GUI job and a fresh stable/M38 render of the same text:

| Pair | MFCC-DTW stable → M38 | Word-join excess quiet, ms | Total-duration error, ms |
| --- | ---: | ---: | ---: |
| Startup | 44.843 → 43.229 | 320 → 90 | −56.0 → −32.0 |
| Short repetition | 39.156 → 40.685 | 30 → 13.6 | −168.2 → −171.2 |
| Short exclamation | 50.145 → 45.818 | 120 → 90 | +89.0 → +96.1 |

These measurements are not a listening test. The short repetition's shape
regresses, and M38 has not fixed prosodic pitch/voicing or breathiness. The
earlier pYIN comparison still has sparse/unstable speech coverage; do not use
its conditional F0 score alone for promotion. M38 remains experimental until
listening and a larger held-out corpus justify it.

## Reproduce locally

Install `tools/requirements-parity.txt` in a local Python environment and use
the already-recorded original reference WAVs and private Elan voice files.
Render baseline and M38 with `nicolai_batch_render`; M38's batch-only flag is
`NICOLAI_M38_BOUNDARY_SPEECH_SHARE=0.5`. The Windows GUI selects the same
policy through its `m38-boundary` profile. For one pair:

```powershell
python tools/audit_talker_pair_alignment.py `
  --original C:\path\to\original.wav `
  --portable C:\path\to\m38.wav `
  --join-log C:\path\to\batch.log `
  --phrase-id 012 `
  --output C:\path\to\new-alignment.json
```

If the log comes from `NicolaiTalker.exe --render`, omit `--phrase-id`. The
script refuses to overwrite prior output. Review individual gap mappings and
the negative cases, not only the aggregate. Do not commit original recordings,
text, DLLs or proprietary voice data.
