# M48: bounded ending-stress authoring

An opt-in successor to M47: select `m48-lexicon` in the Windows EXE or
set `NICOLAI_M48_LEXICON_STRESS=1` in the batch renderer together with
M43/M44 acoustic flags. It inherits M47's timing/acoustics unchanged.
All previous profiles and the default remain independent.

## Recovered primitive

Static inspection of the pinned local `mtsyc32.dll`
(SHA256 `f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`)
and isolated synthetic calls establish the non-plus branch at
`0x10212af5..0x10212dcf` of the candidate author `0x10212890`:

- Ignore the passed stem-stress ordinal; count vowels in the stem.
- Locate the first '<' in a length-prefixed ending using the one-based
  string helper `0x1019eb30`. If present, scan through its following byte;
  otherwise scan the whole ending. Each vowel increments the ordinal.
- Store the resulting one-based stress in candidate field 1. Auxiliary,
  kind, form, tag and source provenance use the same authored layout as M47.
  The portable API returns a zero-based ordinal, not a byte offset.
- The last scanned ending vowel also controls a separate е/ё selection
  path. This requires paradigm/form/type membership tables and special verb
  exclusions. M48 does NOT infer that behavior from the stress ordinal.

The bounded portable primitive accepts lowercase CP866 letters and at most one
marker immediately before a vowel. Empty, consonant-only, truncated, multi-marker,
invalid-character and overlength endings are declined. The original can emit a
stem ordinal for vowel-free endings; that broader behavior is deliberately NOT
integrated. A last scanned 'е' reports `needs_yo_selection`; matching integrated
candidates veto acceptance until that selector has been recovered.

Type '-' is required for the new integration. Existing '+' behavior is unchanged.
Unsupported noun classification for paradigms 1/2 remains refused on this lane.
Known competing analyses must agree on stress, and an eligible main/supported
analysis must exist. Exact entries do not override a conflict; neither dictionary
order nor the first candidate decides. Explicit acute, literal ё, and the exact
exception dictionary still have priority.

## Isolation and evidence

- The optional x86 oracle hash-guards the complete original image and checks PE
  and function fingerprints before execution. It maps without DllMain and binds
  ONLY the inspected system CRT `memchr`/`strchr` IAT slots in its own mapping.
  No installed file, registry, activation state, server or voice is changed.
  Every caller-owned array has checked guards. No private lexical data is used
  in oracle fixtures.
- **10,320/10,320** synthetic ordinal comparisons across kinds 1–7, all respective
  form slots, five stem-vowel counts, and twelve synthetic endings.
- **9,460/9,460** complete authored-candidate byte comparisons for the non-е subset.
  The other **860** compare only the stress ordinal; they do NOT prove ё selection
  and are not accepted by the integration merely because the ordinal matches.
- **36** ordinary-word/invalid-input controls: **29 accepted / 7 declined**.
- Resource audit on **88,771** exception labels: accepted analyses **35,449 → 40,056**
  (+4,607). No previously accepted M47 analysis is declined or changes stress.
  Agreement is **39,368/40,056 (98.28%)**, 688 disagree. New acceptance alone
  is not a correctness measurement; the exact exception dictionary already
  wins rendering for these labeled entries.
- Fresh x86 Release/static build: **38/38 CTest**. M47's separate oracle still
  passes 810 exact comparisons, 20 reserved refusals, and 464 plus-lane checks.
- Previous 36 real phrases: 31 exactly align, 126 marked positions. M47 has
  117 matches; M48 has **123**: **6 fixes / 0 regressions / 3 remaining wrong**.
- Six new original phrases: all align, 36 marked positions. M47 has 28 matches;
  M48 has **34**. Their original normal/trace PCM is identical in **6/6 pairs**:
  13 nonempty + 18 zero-word calls, 182 source phones, zero skipped records
  and zero scan-model mismatches.
- Combined paired audit: **12 fixes / 0 regressions / 5 remaining wrong /
  145 unchanged correct** on 162 marked positions. Five differently expanded
  cases are excluded. This small corpus is not a universal accuracy estimate.
- **11** real hidden UI startup jobs and **154/154 EXE/batch WAV pairs** pass.
  **140/140** old-profile WAVs (all ten old port profiles, including M47) are
  byte-identical to the preserved M47 EXE. The unmarked phrase
  "Будет. Будут. Было. Словами. Закрыты. Говорим. Звонит." is byte-identical
  to its explicitly stressed counterpart in M48; M47 does not match. This
  verifies the actual frontend-to-audio route, not just standalone lookup.

The final clean local EXE SHA256 is
`873c6ea363f99f2c8c7de4a49016e977db9a7432f7b94a28d38fb48fca546e0d`.
Runtime reports bind to that exact artifact. The five remaining marked-position
differences in these controls concern ambiguous "голоса"/"воды" analyses;
M48 does not silently resolve them by candidate order.

Reports contain scalars/hashes only under `docs/metrics/m48-*.json`.
Original snapshots, voice files, per-word audit records and WAVs stay private.
This does not establish universal stress correctness, complete morphology,
contextual ambiguity resolution, acoustic superiority, pauses or phrase intonation.

## Reproduce / continue

Build fresh x86 Release with `NICOLAI_STATIC_RUNTIME=ON`, then run all CTest targets.

```powershell
python tools/test_m48_lexicon.py --probe build_m48_talker/nicolai_m48_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --report metrics-work/fresh-m48-contracts.json
python tools/audit_lexicon_m44.py --dual-m48 `
  --probe build_m48_talker/nicolai_m48_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --exceptions voice-data/exc_rus.txt `
  --out metrics-work/fresh-m48-resources.json
python tools/test_windows_talker.py --exe build_m48_talker/NicolaiTalker.exe `
  --voice voice-data --batch build_m48_talker/nicolai_batch_render.exe `
  --baseline-exe PATH/TO/PRESERVED-M47/NicolaiTalker.exe --baseline-includes-m47 `
  --report metrics-work/fresh-m48-wrapper.json
```

On a licensed local original installation, add `--original PATH/TO/mtsyc32.dll`
to the first command. Use `tools/original_capture_m48_corpus.tsv` with
`tools/test_original_capture.py` for bounded serial real normal/trace pairs;
use `--windows-managed-launch` only explicitly if the inherited launch context
hangs the SDK. Render both profiles with `tools/compare_original_frontend.py`
and compare the private logs via `tools/compare_frontend_delta.py`.

Next: recover and independently validate е/ё selectors, then contextual candidate
selection. Do not turn ambiguous forms such as "голоса" into word-specific fixes.
