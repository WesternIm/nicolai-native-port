# M47: general exact-entry and adjective stress lanes

M47 is an opt-in frontend extension (`m47-lexicon` in the test EXE,
`NICOLAI_M47_LEXICON_STRESS=1` in the batch renderer). It uses existing local
voice resources, not a list of word-specific overrides. It inherits M44's
acoustic/timing settings; stable and the nine previous port profiles do not
enable the new lookup.

## Recovered behavior

The pinned local `mtsyc32.dll` has SHA256
`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`.
Static inspection and synthetic import-free calls establish:

- `0x10211b40(state, 0)` first authors exact entries. Four-byte metadata has
  selector, one-based vowel stress, auxiliary byte and an unused trailing byte.
  Selector zero creates kind 10; recognized selector ranges encode kind/form.
  Reserved 254/255 do not emit a first-lane candidate. Unmapped selector holes
  remain refused by the portable decoder, not treated as a known class.
- For supported six-byte nonverb records with final byte below 247, paradigms
  203–231 use kind 3, 26 forms, stress at metadata byte 3 and type at byte 5.
  Their '+' type sign is slot 1 for all forms, not the form's own slot.
  Paradigms 232–255 use kind 4 and four form-specific type slots. Earlier
  paradigms in this record lane need an unported selector table and are declined.
- The '+' lane at `0x10212890` copies the stem vowel ordinal without consulting
  an internal '<' suffix marker. M44's marker refusal stays unchanged; only
  M47 allows this proven path. Minus/ending-stress authoring is not executed by
  the isolated oracle and is not implemented in M47.

The exact-entry author writes fields starting at block+24, including provenance
at +36/+40. M51 corrected the earlier interpretation: the actual payload is
20 bytes at block+24; M46's historical window at block+20 starts with the
previous record's dictionary ID, NOT a priority score. Context score is at
payload+8. The M47 oracle's 24-byte comparison covers the complete authored
payload and is still valid. See [M51](M51_ANALYSIS_SELECTION.md).
The probe does not call full NLP in a DLL mapped without initialized imports.

Lookup accepts only one distinct stress with an eligible supported lane and no
unresolved matching candidate. Exact entries are not given blind priority over
competing inflections. Explicit acute, ё and exact exception dictionary labels
still take precedence. Unresolved zero-stress records, alternate analyses and
unimplemented ending-stress candidates retain conservative fallback behavior.

## Evidence and limits (2026-10-02)

- Fresh x86 Release/static build: **37/37 CTest**.
- Pinned original synthetic oracle: **810/810 exact-entry byte comparisons**,
  **20/20 reserved-selector refusals**, **464/464 '+' candidate-field checks**
  across five kinds and marked/unmarked synthetic suffixes. No original entry
  data is included in the fixtures.
- Local ordinary-word controls: **21 accepted / 8 intentionally declined**.
- Previous 30 real captured phrases: 25 exactly align after normalization,
  74 original marked words. M44 has 69 matches; M47 has **72**. Five differently
  expanded cases are excluded, not scored as successes.
- Six new original phrases covering adjective forms, cases, adverbs, irregular
  verbs and ambiguous words: all six align, 52 marked words. M44 has 31 matches;
  M47 has **45**. The final EXE's original trace versus preserved M46b normal
  output gives **6/6 identical PCM pairs**, 22 nonempty + 18 zero-word calls,
  332 source phone records, zero skipped records and zero scan-model mismatches.
- Combined paired position audit: **17 fixes, 0 regressions, 9 remaining wrong,
  100 unchanged correct** on 126 marked positions. This is a small control
  corpus, not a universal accuracy estimate.
- Resource audit on 88,771 exception labels: accepted analyses grow
  **24,595 → 35,449** (+10,854), with no previously accepted lookup declined.
  Agreement is 34,933 / 35,449 (98.54%); 516 disagree. These labels are an
  independent resource check, not full original NLP ground truth. Rendering
  already prioritizes the exact exception dictionary, including these conflicts.
- Ten real hidden UI startup jobs and **120/120 EXE/batch WAV pairs** pass.
  **108/108 old-profile WAVs** are byte-identical to preserved M46b. In M47,
  unmarked `Хорошая. Спокойно. Длинное. Быстро.` produces exactly the same WAV
  as the explicitly stressed version; M44 does not. This verifies actual
  frontend activation, not just a standalone lookup test.

Scalar reports are under `docs/metrics/m47-*.json` and the M44 controls baseline
is `docs/metrics/m44-frontend-m47-controls-20261002.json`. Raw original snapshots,
words from the resource audit, per-word logs and WAVs remain private/ignored.
The `m47-clean-*` reports repeat runtime checks on the final clean rebuild,
whose EXE SHA256 is
`62d7ea04fed873e7da72bf363e16e00c699a9272b4313da580ad33bcb48786ab`.
Earlier reports retain their own tested executable hashes instead of being
relabeled as results from the final artifact.

No improved pauses, global speaking speed, phrase intonation, pitch contour,
clicks or perceptual acoustic superiority are claimed. Original unmarked words
are counted separately; assigning a lexical ordinal is not reproducing their
perceived prominence.

## Reproduction

Build an x86 Release/static configuration from a VS developer environment and
run all CTest targets. On hosts with only the voice resources, run:

```powershell
python tools/test_m47_lexicon.py --probe build_m47_talker/nicolai_m47_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --report metrics-work/fresh-m47-controls.json
python tools/audit_lexicon_m44.py --dual-m47 `
  --probe build_m47_talker/nicolai_m47_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --exceptions voice-data/exc_rus.txt `
  --out metrics-work/fresh-m47-agreement.json
python tools/test_windows_talker.py --exe build_m47_talker/NicolaiTalker.exe `
  --voice voice-data --batch build_m47_talker/nicolai_batch_render.exe `
  --baseline-exe C:/path/to/preserved/M46b/NicolaiTalker.exe --baseline-includes-m44 `
  --report metrics-work/fresh-wrapper.json
```

For the isolated x86 original oracle, add `--original C:/path/to/mtsyc32.dll`
to `test_m47_lexicon.py`; the probe refuses a different whole-file hash. It
does not initialize the module, change its installation or invoke activation.

For full original comparison, use `test_original_capture.py` with
`tools/original_capture_m47_corpus.tsv`, a preserved original-capable baseline
EXE, fresh private output/report paths and, if required on this host, explicit
`--windows-managed-launch`. Then run `compare_original_frontend.py` against the
same capture root twice, with `--profile m44-lexicon` and `--profile m47-lexicon`,
using different fresh output directories. `compare_frontend_delta.py` accepts
these directories as `--comparison CAPTURES BASELINE CANDIDATE` and reports
fixed/regressed positions without publishing words or raw records.

## Continuation boundary

The remaining errors include `будет/было/будут`, `словами`, `закрыты` and original
selection for `голоса`. Next recover general '-' ending-stress authoring and
its special ё/verb tables with a safely initialized original oracle; then
candidate filtering/context. Do not resolve them by selecting the first entry
or giving exact entries universal precedence. After lexical coverage, compare
original marker groups/raw phrase splits before changing pause/F0 rules.
