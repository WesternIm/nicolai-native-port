# M51: earlier analysis, corrected candidate layout and isolated primitives

M51 is a reverse-engineering/diagnostic checkpoint, **not a new audible profile**.
All thirteen port profiles, including M50, keep their existing synthesis paths.
There is no `m51-lexicon` profile, word override, random parameter search or
automatic first-candidate fallback. The new portable primitives are deliberately
not called by the renderer until candidate generation and context scoring agree.

## Corrected layout

For one-based candidate c, the actual 20-byte payload is at
`word_block + 20*c + 4`. The historical M46 `candidates_hex` window starts at
`word_block + 20*c`: for c>1 its leading DWORD belongs to the preceding record
(for c=1 it is unused leading storage), NOT a priority score.
This corrects the old explanation in M47; its full 24-byte
author comparison remains valid. The last payload (c=70) ends exactly at
punctuation offset 0x590 in the 0x59c-byte word block.

| Payload offset | Inspected use |
| --- | --- |
| 0 | One-based ё character position, zero means none |
| 1 | One-based stress vowel ordinal, zero means unmarked |
| 2 | Auxiliary pronunciation byte; its full semantics remain open |
| 3 / 4 / 5 | Kind / tag / form |
| 6..7 | Unassigned by the inspected author; do not invent meaning |
| 8..11 | Signed 32-bit context score |
| 12 | Original dictionary block ID |
| 13..15 | Unassigned by the inspected author |
| 16..19 | Original dictionary record ID |

Evidence: the authors at 0x10211b40/0x10212890, score writes in 0x10217720,
the selector at 0x10228270, bounded synthetic snapshot fixtures, and actual
before/after captures. New snapshots preserve legacy windows and add
`candidate_payloads_hex` plus
`candidate_layout="payload20-at-stride-plus4-v1"`. Both audits verify byte
overlap, including provenance across adjacent records. Old four-stage
captures remain readable; the extended capture has its own schema.

## Original pipeline and portable boundary

The dispatcher at 0x101a0ac0 first extracts explicit markers into state+0x168
and searches morphology through 0x1021a550. That search tries suffix lengths
downwards, bounded by ten bytes, through 0x1021a8e0/0x1021aa80, authors analyses,
and applies explicit-marker filtering at 0x1021af70. Generation/order and
all supported classes are not yet equivalent to the portable consensus lookup.

The candidate lane of 0x101a1830 is now portable:

- Kind 0 adds 81 to form; kind 1 preserves it; kinds 2..6 add
  8, 19, 45, 49 and 59 respectively. Arithmetic wraps to an unsigned byte.
- Kind 7 maps ranges 1..26, 27..52, 53..78, 79..108, 109..112 and 113
  by +19, -7, -33, -59, -63 and -40; other bytes remain unchanged.
- All candidates have score bytes 8..11 cleared. Other payload bytes remain
  unchanged. This does NOT implement the original punctuation cleanup.

The portable predicate at 0x10217830 compares only the first three payload
bytes. Zero/one candidate, or a shared triple across all candidates, returns
false even if forms, dictionary IDs or existing scores differ.
When all triples agree, 0x10217720 gives every candidate score 1; otherwise
0x102178b0 dispatches grammatical scorers (0x10217940, 0x10218c00,
0x10219c10). Those scorers and their selector-table ownership remain open.

0x10228270 selects the first greatest signed score (initial floor -32767),
reads the selected dictionary record, and writes ё/stress. This observed
tie behavior is NOT a license to choose the first portable consensus candidate:
the producer, ordering, scores and possible context-driven word replacements
must first be reproduced. For example, the original's two `замок` contexts
still tie between stress variants; these captures are not semantic perfection.

## Seven-stage capture

Explicit `--render-analysis-trace` / PowerShell `-Trace -Analysis` enables
the new stages in an owned fresh original SAPI session only:

| Event | Pinned instruction address | Observation |
| --- | --- | --- |
| before_normalization | 0x101a17b6 | Generated analyses |
| before_context | 0x101a17bc | Normalized forms, zeroed scores |
| after_context | 0x101a17c2 | Immediately after scoring/selection |
| before_split | 0x101a17ed | After number processing, before separators |
| after_split | 0x101a17f3 | Separator codes |
| after_markers | 0x101a17ff | Final annotated words before phonetic authoring |
| after_authoring | 0x101a1811 | Source-phone records |

The first three snapshots intentionally do not dereference separator/code/
phone arrays: they have not all been initialized. The new non-push instruction
has a complete six-byte fingerprint; all hooks require the pinned whole-file
SHA, existing ownership/lifetime guards and restoration while threads stop.
Seven ordered rows per call are required, including zero-word service calls.
The normal trace still has four stages and the original M46 schema.
The current seven-stage audit requires fixed word counts across these stages;
number expansion that changes the count is outside this contract, not silently
counted as agreement.
The private output filename remains `linguistics-m46.jsonl` for the shared
worker/cleanup lifecycle; the schema, not the filename, identifies the format.

## Evidence (2026-10-02)

Fresh x86 Release/static build and coherent final rebuild: **41/41 CTest**.
Voice-free tests include 65,536 full portable normalization payloads, 692
pronunciation-conflict fixtures, both capacity refusals, corrected snapshot
offset/capacity/early-array guards, lifecycle tests and both strict audits.

Six authored ordinary-text controls, with actual installed original SAPI:

- **31 complete seven-stage calls / 217 records**, 13 nonempty calls,
  18 zero-word calls, 44 analyzed words, 190 source phones; no skipped records
  or scan-model mismatches. **6/6 original normal/trace PCM pairs are identical**.
- **29 words** change candidate forms in normalization. Portable AND isolated
  original normalization match the actual stage transition for **44/44 word
  vectors / 93/93 full payloads**.
- Full byte-domain normalizer oracle: **65,536 distinct kind/form pairs**
  per run, repeated for six controls (**393,216 executions**). Another
  **651** actual stage payloads agree when passed to both normalizers.
- Conflict oracle: **2,076 synthetic cases** per run across count 0..70,
  word indices 0/1/256, each pronunciation byte, first/middle/last position
  and unrelated-byte variation; six repetitions (**12,456 matches**).
  **308/308 actual captured word vectors** also agree.
- **6 word occurrences** have competing pronunciation triples.
  The 37 common-triple words all receive the observed score-one fast path.
  Selected stress/ё annotations match first-max-score payloads in
  **43/43 exactly aligned candidate-bearing words**, without exclusions.
  Among those, 27 have a unique greatest score and 16 have a tie.
- In the control contexts, `лица` changes stress between
  `смотрит на лица` and `нет лица`; `белье` selects ё. These are observations
  of the ORIGINAL, not newly fixed port outputs or word-specific rules.
- Original oracle full buffers/canaries show no unexpected mutation.
  SHA mismatch and five malformed candidate/normalization inputs are refused.
- Final wrapper isolation: **13 hidden GUI jobs**, **234/234 EXE/batch PCM
  pairs**, **234/234 all-profile PCM pairs against preserved M50**. No audible
  improvement is claimed; the isolation is intentional.
- The default four-stage capture also passes a fresh original normal/trace
  control with unchanged PCM, four complete calls and no skipped records.

Scalar reports: `docs/metrics/m51-*.json`. Raw records, original DLL/resources,
normalization-pair exports, detailed logs and WAVs remain private/ignored.
The final locally tested talker SHA256 is
`2b97d1296b49f8d14cca1f3d29bccf2e9e7fe53ca1574d92d9cabc56723c4ec2`;
the original-probe SHA256 is
`cd531596ac473d41488715b8ae770a1141218d80713da49d02b094d8728236ae`.

The oracle maps only the matching x86 original with DONT_RESOLVE_DLL_REFERENCES,
without DllMain/import initialization. The conflict function is import-free.
The normalizer is executed with owned empty leading punctuation and a nonempty
non-special separator without spaces/quotes: its inspected deletion and CRT
lanes cannot execute. It requires the preferred image base because of absolute
read-only globals. It patches no original code. Full owned-buffer comparisons
prove the expected candidate-only writes; mismatched DLLs/mappings are refused.
No installed DLL, registry, activation, licensing or user/shared server changes.
No original code is called by portable synthesis.

## Reproduce and continue

Build x86 Release/static and run the full CTest suite. No voice data is needed
for the portable tests or `python tools/audit_m51_analysis.py --self-test`.
For real capture use your own installed original and fresh private outputs:

```powershell
python tools/test_original_capture.py --exe build_m51_release/NicolaiTalker.exe `
  --baseline-exe PATH/preserved-M50/NicolaiTalker.exe `
  --corpus tools/original_capture_m51_corpus.tsv `
  --output metrics-work/FRESH-CAPTURES --report metrics-work/FRESH-CAPTURES.json `
  --windows-managed-launch --analysis
python tools/test_m51_analysis.py --capture-root metrics-work/FRESH-CAPTURES `
  --probe build_m51_release/nicolai_m51_candidate_probe.exe `
  --original PATH/Elan/mtsyc32.dll --output metrics-work/FRESH-ORACLE `
  --report metrics-work/FRESH-ORACLE.json
python tools/test_windows_talker.py --exe build_m51_release/NicolaiTalker.exe `
  --voice voice-data --batch build_m51_release/nicolai_batch_render.exe `
  --baseline-exe PATH/preserved-M50/NicolaiTalker.exe --baseline-includes-m50 `
  --report metrics-work/FRESH-WRAPPER.json
```

Next close the exact ordered candidate producer (suffix/stem search, explicit
annotation filtering, supported class/type membership), comparing corrected
full payloads at before_normalization. Then restore scorer/table ownership
per grammatical family, starting with noun 0x10217940 controls. Only after
score/selection and real stress/ё regressions are proven should an opt-in
frontend profile consume them. Phrase pauses, F0 and acoustic joins still
require independent paired work; this checkpoint does not change them.
