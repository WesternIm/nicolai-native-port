# M49: data-driven ending е/ё selection

An opt-in successor to M48: `m49-lexicon` / "M49 окончания и е/ё" in the
Windows EXE. It inherits M48/M43 acoustics and timing unchanged. Stable and
all eleven previous port profiles retain their own behavior. This is a
general lexical/grammatical selector, not a list of word replacements.

## Recovered behavior and private input

The pinned original candidate author at `0x10212890`, non-plus ending lane
`0x10212af5..0x10212dcf`, has a separate spelling flag in addition to the
stress ordinal. Only a last scanned CP866 `е` can become `ё`. M49 recovers:

- Length-prefixed paradigm and type memberships for kinds 1–7 at
  RVA `0x4e8530 + kind * 61` and `0x4e854e + kind * 61`.
- Kind 7's allowed form ranges 79–104 and 109–112.
- Kind 6's exclusions: paradigm 27/forms 12–13, and paradigms 30/32/33
  with forms 1/8/11.
- For kinds 1/2, a separate per-paradigm noun-form membership filter.
  Its real runtime binding is NOT established. A positive global membership
  with unknown noun filter is refused. A negative global membership proves
  "keep е" without knowing that filter; no unknown value is guessed false.
- The character coordinate of inferred ё after removing the ending marker.
  A later non-е vowel clears an earlier ё possibility.

The generic memberships are compiled into the original DLL, not all in EDAT.
`tools/export_yo_policy_m49.py` reads the user's local original as bytes,
verifies its complete SHA256/PE layout and extracts **536 bytes** of private
data. It does not execute the DLL, run a server or alter activation. The
exported `nicolai-yo-m49.bin` lives beside `nicolai16.dat`; it is ignored by
Git and excluded from public packages. M49 requires it explicitly. Missing,
truncated or corrupt policy is an error, never silent fallback to M48.
Older profiles do not read or require it.

Pinned original SHA256:
`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`.
Local export SHA256:
`8bf14233689f3273802f95d5021cb2595c2e753ce82c34cbbdd968373cb4e62f`.
The runtime parser checks exact size, source metadata, CRC32, length bounds,
unique members, value ranges and zero padding. CRC/source metadata are
compatibility/integrity checks, not authentication of arbitrary forged files.

## Actual synthesis route

Candidate agreement now includes both the stress ordinal and optional ё
character coordinate. Same stress with conflicting е/ё choices is still
ambiguous; dictionary order does not decide. Explicit acute, literal ё and
the exact exception dictionary retain precedence.

`FrontendWord.source_utf8` retains the input spelling;
`pronunciation_utf8` records the letters actually sent to phone mapping.
An accepted ё is applied before reduction/phoneme selection, not merely to a
diagnostic stress field. The EXE emits `frontend_pronunciation_utf8_hex` to
make this route auditable. No original code is called by the port profile.

## Evidence (2026-10-02)

- Fresh x86 Release/static build: **39/39 CTest**.
- Original synthetic oracle: **545,480/545,480 full candidate byte matches**,
  including **6,458 positive ё cases**. All 20 type slots and the supported
  paradigm/form domains are exercised. Noun form filters use owned synthetic
  all-present/all-absent lists, NOT recovered real noun runtime state.
  **2,630** unknown-positive-noun-filter checks correctly refuse selection.
  The probe hash/PE/code guards the original, maps without DllMain in its
  own process and binds only inspected CRT memchr/strchr slots. Caller-owned
  buffers have checked canaries; installed files/registry/server are untouched.
- **30 ordinary/invalid controls**, 22 accepted, 11 accepted ё spellings.
- Resource audit: 88,771 exception labels, accepted analyses
  **40,056 → 40,336 (+280)**, zero previous acceptances lost/changed in stress.
  Agreement 39,623/40,336 (98.23%), 713 disagree; 27 accepted ё choices.
  This audit labels stress only, NOT ё correctness. Exact exceptions already
  win rendering, so these counts are not 280 audible corrections.
- Six fresh original normal/trace pairs have identical PCM in **6/6**.
  Captures contain 14 nonempty + 18 zero-word calls, 191 source phones,
  zero skipped records and zero scan-model mismatches.
- The actual M48/M49 frontend comparison on those six phrases exactly aligns
  47 words (allowing only е/ё spelling variants): **6 stress fixes / 0
  regressions** on 42 marked positions; **9 е/ё fixes / 0 regressions** on
  29 е/ё positions. One marked stress and two ё spellings remain different
  (`воды`, ambiguous `поешь`/`поем`). No context-specific answer is forced.
- Previous 42 phrases: 37 exactly align. All 162 marked stresses and all
  64 е/ё positions preserve M48 results, including five unresolved stress
  differences. Five differently expanded cases are excluded, not successes.
  Combined: 48 cases/43 aligned, **6 stress fixes and 9 е/ё fixes, no observed
  regressions**, six remaining stress differences and two spelling differences.
  This small corpus is not a universal accuracy estimate.
- **12** hidden UI startup jobs, **192/192 EXE/batch WAV pairs**, and
  **176/176 old-profile WAV pairs** against the preserved M48 EXE pass.
  In M49, "Живете. Найдете. Пьете. Льете. Вернете. Вернет. Поете. Споет."
  produces exactly the same WAV as the literal-ё counterpart. None of the
  old profiles does. This proves frontend-to-audio activation, not listening
  superiority or original PCM equality.
- Six end-to-end missing/header/checksum refusals pass across EXE/batch.
  M48 renders without the new file. Original caller files are not modified.

The tested local EXE SHA256 is
`1fccfbcc223877c934dab3607bafa72c14f7e2bb6d1bf7403354e258d6b45174`.
Scalar reports/hashes only are committed under `docs/metrics/m49-*.json`.
Raw tables, voice inputs, snapshots, per-word audit details and WAVs stay local.

## Reproduce

Export once to a **fresh private path** beside local voice data using standard
Python; use your own matching original file, not a downloaded replacement:

```powershell
python tools/export_yo_policy_m49.py --original C:/path/Elan/mtsyc32.dll `
  --output voice-data/nicolai-yo-m49.bin
python tools/test_m49_lexicon.py --probe build_m49_talker/nicolai_m49_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --policy voice-data/nicolai-yo-m49.bin `
  --original C:/path/Elan/mtsyc32.dll --report metrics-work/fresh-m49-contracts.json
python tools/audit_lexicon_m44.py --dual-m49 `
  --probe build_m49_talker/nicolai_m49_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --exceptions voice-data/exc_rus.txt `
  --policy voice-data/nicolai-yo-m49.bin --out metrics-work/fresh-m49-resources.json
python tools/test_windows_talker.py --exe build_m49_talker/NicolaiTalker.exe `
  --voice voice-data --batch build_m49_talker/nicolai_batch_render.exe `
  --baseline-exe C:/path/preserved-M48/NicolaiTalker.exe --baseline-includes-m48 `
  --report metrics-work/fresh-m49-wrapper.json
python tools/test_m49_policy_failure.py --exe build_m49_talker/NicolaiTalker.exe `
  --batch build_m49_talker/nicolai_batch_render.exe --voice voice-data `
  --report metrics-work/fresh-m49-policy-refusals.json
```

Build fresh x86 Release with static runtime first and run all CTest targets.
The batch renderer uses `NICOLAI_M49_LEXICON_STRESS=1` with the unchanged
M43 acoustic flags. `--original` is optional for portable-only controls.
Voice-free CI covers selectors/precedence/parser, exporter hash refusals,
comparison alignment/diagnostic contracts and the proprietary-input-free ZIP.

For real reference pairs use `tools/original_capture_m49_corpus.tsv` and the
bounded serial `tools/test_original_capture.py` workflow. Compare using
`tools/compare_yo_frontend_m49.py --exe PATH/M49.exe --baseline-exe PATH/M48.exe
--voice voice-data --capture-root PRIVATE/CAPTURES --output PRIVATE/FRESH
--report PRIVATE/FRESH.json`. Folding е/ё is used only for alignment;
correctness compares the actual pronunciation letters. Existing verified
logs can be reused with repeated `--comparison CAPTURES M48_LOGS M49_LOGS`.
Ordinary exact-word stress comparisons also remain independently reproducible.

## Still open / continuation

The [M50 successor](M50_NOUN_YO.md) now recovers real noun-form ownership for
paradigms 3..202 and validates initialized filters plus actual pronunciation.
Earlier normalization/analysis paths and contextual selection remain open.
`поешь`/`поем`, `голоса` and `воды` must not become hand-written overrides.
Common forms such as `несет`/`ведет` can still miss the currently read stem lane.
Reflexive classes and full morphology are incomplete. Phrase pauses, prominence,
intonation and shared acoustic phase continuity are unchanged by M49. Continue
with isolated primitives and paired route evidence before new integration.
