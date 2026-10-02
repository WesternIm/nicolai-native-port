# M50: initialized noun е/ё form filters

`m50-lexicon` / "M50 существительные и е/ё" is an opt-in successor to M49.
It retains M49/M43 acoustics and timing; stable and all twelve older port
profiles keep their own behavior. This adds a recovered grammatical filter,
not per-word replacements, a contextual disambiguator or a new pitch contour.

## Ownership and recovery

The original candidate author at `0x10212890` consults noun membership through
`state + 0x384c + 4 * paradigm`. The value at `0x384c` itself is `s.dat`, not
the noun-form table. Paradigms 1/2 alias the FLX pointers at `0x3850/0x3854`;
their membership semantics remain unknown and are still refused.

The initializer at `0x10102592..0x101025de` allocates **200 distinct 20-byte
buffers**: 97 for paradigms 3..99 and 103 for 100..202. Its allocator at
`0x10003280` calls inspected `HeapAlloc` with `HEAP_ZERO_MEMORY` (flag 8).
The import-free lane `0x101025de..0x10102770` assigns **22** length-prefixed
lists through `0x10105a00`. The other **178 are known empty**, not unknown.
The lane leaves 56 argument bytes on the stack. Kind 1 has forms 1..8;
kind 2 has forms 1..6. Other kind/paradigm/form combinations remain unknown.

`tools/export_noun_yo_policy_m50.py` reads the original as bytes, checks its
complete SHA256/PE layout, and decodes only the inspected instruction subset.
Unknown instructions, call targets, argument frames, pointer ranges, duplicate
assignments and invalid list members fail explicitly. It never executes the
DLL or starts a server. A full-source hash pins the allocation/helper semantics
that are not reimplemented by the small instruction decoder.

The new **1848-byte** private file is `nicolai-noun-yo-m50.bin`, beside
`nicolai16.dat`. M50 also requires M49's `nicolai-yo-m49.bin`. Both are ignored
by Git and excluded from packages. Missing/corrupt inputs are errors, not silent
fallback. Older profiles do not read the new noun file. The parser checks size,
source metadata, CRC32, count/member bounds, duplicates and zero padding.
CRC/metadata check compatibility and integrity, not authentication of forged
files. No original code is called by the port's synthesis path.

Pinned original SHA256:
`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`.
Noun export SHA256:
`2b282c4fd68966fb55e0a41be369040304c095c3f973ae812f57e0ecdeb02119`.
M49 export SHA256:
`8bf14233689f3273802f95d5021cb2595c2e753ce82c34cbbdd968373cb4e62f`.

## Integration and evidence (2026-10-02)

M50 passes known noun membership to the existing M49 ending selector. Accepted
stress AND е/ё coordinates must agree across every eligible analysis; record
order cannot select an answer. Explicit acute, literal ё and the exact exception
dictionary retain precedence. Inferred ё reaches phone mapping through the
existing `pronunciation_utf8` path; source spelling remains unchanged.

- Fresh x86 Release/static build: **40/40 CTest**.
- Original oracle: **200/200 initialized 20-byte list matches**, then
  **111,520/111,520 full candidate-byte matches**, including **1,070** positive
  ё cases across every supported noun paradigm, all 20 type slots, every form
  and four synthetic endings. This is candidate-author parity, not full NLP.
  The full-SHA/PE/code-guarded x86 probe maps without DllMain in its own
  disposable process. It runs only the inspected list-writing lane against
  canary-guarded owned buffers; a temporary return epilogue bounds its
  fall-through in that mapping only. It then binds only inspected system-CRT
  memchr/strchr slots for candidate authoring. Installed files, registry,
  activation and user/shared servers are untouched.
- **30 ordinary/invalid controls**, 16 accepted, **8 newly accepted**;
  9 accepted ё controls. Ambiguous nominative forms remain declined.
- Resource audit: 88,771 labels, accepted **40,336 → 40,344 (+8)**;
  zero previous acceptances lost, zero changed prior stress or pronunciation.
  Stress agreement is 39,629/40,344 (98.23%), 715 disagree; 29 accepted ё
  choices. Exact exception entries already win rendering, so these are NOT
  eight audible fixes or a spelling-accuracy estimate.
- Six fresh normal/trace original pairs preserve original PCM in **6/6**.
  They contain 16 nonempty + 18 zero-word calls, 153 source phones, no skipped
  records and no scan-model mismatches.
- Actual M49/M50 pronunciation comparison on those six phrases aligns all
  **35 words**, 29 marked stresses and 26 е/ё positions: **6 е/ё fixes,
  0 regressions**. The corrected controls are `бельем`, `копьем`, `ружьем`,
  `мытьем`, `землей`, `семьей`. Stress is unchanged: three differences remain
  in other words. Four nominative ё differences also remain unresolved.
- Previous 48 phrases: **43 aligned**, all 204 marked stresses and all 93
  е/ё positions unchanged from M49. Six stress/two spelling differences remain.
  Five differently expanded cases are exclusions, not successes. Combined
  54 cases/49 aligned: six spelling fixes, no observed stress/spelling
  regressions; nine stress and six spelling differences remain. This small
  corpus is not universal accuracy or original acoustic parity.
- **13** hidden GUI startup jobs, **234/234 EXE/batch WAV pairs**, and
  **216/216 old-profile WAV pairs** against the preserved M49 EXE pass.
  Only M50 renders the six plain-е noun controls byte-identically to their
  literal-ё counterparts; M49's verb ё controls remain active in both modes.
  This proves audio-route activation/isolation, not perceptual superiority.
- Six end-to-end missing/header/checksum refusals pass across EXE/batch.
  M49 works without the new file; caller inputs are not modified.

The tested EXE SHA256 is
`d8c0453fd0df32e44d85b47a19ef5fdd4cbade84db4ade1002ccf23460fa04fa`.
Scalar reports are in `docs/metrics/m50-*.json`; initialized payloads, voice
data, snapshots, detailed candidate/word logs and WAVs stay private.

## Reproduce

Build a fresh x86 Release with static runtime and run the full CTest suite.
Export to fresh private paths using ordinary Python and your own matching
original; existing files are never overwritten:

```powershell
python tools/export_yo_policy_m49.py --original C:/path/Elan/mtsyc32.dll `
  --output voice-data/nicolai-yo-m49.bin
python tools/export_noun_yo_policy_m50.py --original C:/path/Elan/mtsyc32.dll `
  --output voice-data/nicolai-noun-yo-m50.bin
python tools/test_m50_lexicon.py --probe build_m50_release/nicolai_m50_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --policy voice-data/nicolai-yo-m49.bin `
  --noun-policy voice-data/nicolai-noun-yo-m50.bin --original C:/path/Elan/mtsyc32.dll `
  --report metrics-work/fresh-m50-contracts.json
python tools/audit_lexicon_m44.py --dual-m50 `
  --probe build_m50_release/nicolai_m50_lexicon_probe.exe `
  --voice voice-data/nicolai16.dat --exceptions voice-data/exc_rus.txt `
  --policy voice-data/nicolai-yo-m49.bin --noun-policy voice-data/nicolai-noun-yo-m50.bin `
  --out metrics-work/fresh-m50-resources.json
python tools/test_windows_talker.py --exe build_m50_release/NicolaiTalker.exe `
  --voice voice-data --batch build_m50_release/nicolai_batch_render.exe `
  --baseline-exe C:/path/preserved-M49/NicolaiTalker.exe --baseline-includes-m49 `
  --report metrics-work/fresh-m50-wrapper.json
python tools/test_m49_policy_failure.py --profile m50-lexicon `
  --exe build_m50_release/NicolaiTalker.exe --batch build_m50_release/nicolai_batch_render.exe `
  --voice voice-data --report metrics-work/fresh-m50-refusals.json
```

`--original` is optional for portable controls. The batch renderer enables
`NICOLAI_M50_LEXICON_STRESS=1` plus the unchanged M43 acoustic flags.
Voice-free CI exercises parser/selector/precedence/frontend contracts, exporter
refusals, comparison alignment and the package without proprietary inputs.

Real reference pairs use `tools/original_capture_m50_corpus.tsv` and the bounded
serial `tools/test_original_capture.py` workflow. Audit actual letters with:

```powershell
python tools/compare_yo_frontend_m49.py --exe PATH/M50.exe --baseline-exe PATH/M49.exe `
  --baseline-profile m49-lexicon --candidate-profile m50-lexicon --voice voice-data `
  --capture-root PRIVATE/CAPTURES --output PRIVATE/FRESH --report PRIVATE/FRESH.json
```

Folding е/ё is ONLY for alignment; correctness uses the actual pronunciation.
Verified logs can be reused via repeated `--comparison CAPTURES M49_LOGS M50_LOGS`.

## Continuation

Noun-list ownership is closed for paradigms 3..202, but earlier analysis and
context selection are not. `белье`, `копье`, `ружье`, `мытье` now have known
filters yet conflicting analyses, so their unmarked nominative pronunciations
still miss original ё. Do not resolve them by dictionary order or word overrides.
`поешь`/`поем`, `голоса`, `воды`, missing `несет`/`ведет`, reflexive classes and
other morphology remain open. Next isolate the original earlier analysis/
candidate filtering and validate real-route controls before integration.
Phrase pauses, prominence, F0 and shared acoustic phase continuity are unchanged
by this checkpoint and still need independent paired investigation.
