# M44: bounded original lexicon and stem-stress forms

`m44-lexicon` adds an opt-in lexical layer to M43. It reads the user's existing
EDAT dictionary, suffix paradigms and stress-type tables, rather than adding
per-word patches. Production stable and every earlier profile stay unchanged.
This is **partial original-resource integration**, not a completed original
Russian NLP/prosody port or a claim that pauses, clicks and intonation are fixed.

## Recovered resource contracts

The supported local voice has SHA256
`471bf1266c913784187dae25a2e5784a6ec309162e7dee2167ca9887c253e74d`.
Resource data references belong to the EDAT initialization segment. A null
reference is not a pointer to its beginning. The reader resolves file objects,
checks sizes/bounds and never loads an original DLL in the synthesis path.

| Resource | Contract |
| --- | --- |
| `d.dat` | 2,000,000-byte allocation; 16,384-byte blocks, u16 record offsets; length-prefixed CP866 stem and metadata |
| `d.ind` | 80 records of 44 bytes; 40-byte boundary string + u32 count; first byte `FF` terminates active blocks |
| `non_verb1.flx` / `non_verb2.flx` | 10,570-byte packed allocation, 510-byte u16 header; 256-entry expanded u32 index with entry zero null |
| `vverb1.flx` / `vverb2.flx` | 54,728-byte packed allocation, 512-byte u16 header; expanded index copies 255 slots |
| `types.num` | Owns the entire 3,680-byte shared allocation: 8 kinds x 20 rows x 23 bytes |

There are **68 active blocks, 63,294 records and 55,585 folded stems**. An earlier
63,458 count included the sentinel's obsolete counter; it was not an additional
active block. These are stem records, not a count of supported rendered words.
`s.dat` is a transformed working copy and is deliberately not used as the raw
dictionary. The other `types.*` file objects have null references because their
tables are populated in the shared allocation.

Original code evidence is in the supported `mtsyc32.dll`, SHA256
`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`:

- `0x101032C0`: dictionary/index allocation and loading.
- `0x10103610..0x10103837`: packed/expanded FLX index construction.
- `0x10103840`: shared stress-type allocation.
- `0x101039B0`: type-cell address `23 * (20 * kind + row) + slot`.
- `0x1021AA80` and `0x1021AEAD`: dictionary search and block advance.
- `0x10211EDF..0x10212659`: candidate paradigm/suffix selection.
- `0x102126C0`: length-prefixed suffix variant matching.
- `0x10212890`: intermediate stress candidate authoring; `+` preserves the stem
  vowel ordinal, while `-` requires further ending/marker logic.

Some unused verb FLX slots contain legacy allocator residue. All 255 index
mirror relations are checked, but only paradigms referenced by dictionary
records are treated as payload addresses. Inactive garbage cannot split a
suffix list; an invalid active pointer makes the whole reader fail closed.

The optional x86 original probe maps the supplied image without imports or DLL
initialization. It calls only two inspected import-free primitive paths with
synthetic state: **3,680 type-cell pointer matches and 70 `+` main-verb candidate
field matches**. Fingerprint/PE checks reject a different executable. This does
not run original full text analysis, morphological selection or synthesis.

## What M44 accepts, and what it declines

Queries use a folded stem plus a suffix from the original paradigms. Supported
integration is limited to the noun form lanes and non-reflexive main verb lanes
whose type cell is `+`, with a valid stem vowel ordinal. Marked suffixes,
ending-stress `-` cells, special verb classes 34/35, adjective/participle-only
results and unknown selectors do not enable a result. Known competing
paradigm candidates can veto it or confirm the same stress, but cannot
independently enable an unsupported lane. Context disambiguation and the full
original grammatical masks are not implemented.

Stress priority is explicit acute, `ё`, exact `exc_rus.txt`, M44 accepted
lexicon result, M43 limited `-ика` fallback, built-in fallback, heuristic.
Conflicting/unsupported lexical analyses leave the old fallback path intact.
Exact exceptions and explicit input remain authoritative. An invalid lexicon
resource produces a normal error in M44; other profiles do not parse it.

Controls include `проверя́ем / проверя́ешь / проверя́ют`, `де́лаем` and inflected
`аку́стику / фи́зику / ло́гику`. No production word-specific cases were added.
`молоко`, `воды`, `звонит`, `говорим` and `замки` exercise deliberate refusal,
not a claim that the complete original parser would also decline them.

## Evidence, 2026-10-01

- Synthetic bounded-reader tests run in both Release and Debug, including
  malformed lengths, indices/types, ambiguity, explicit/exception precedence,
  marked/reflexive refusal and inactive-slot handling. x86 Release CTest: 33/33;
  x64 Debug: 32/32.
- [20 local lexical controls and two original primitive contracts](metrics/m44-original-contracts-20261001.json).
- [Resource cross-check](metrics/m44-lexicon-20261001.json): 24,595 accepted out
  of 88,771 distinct single-word exception labels; 24,239 agree and 356 differ
  (98.55% agreement **among accepted cases**, not general word accuracy).
  Rendering still prioritizes exact exceptions. The disagreement remains a
  reason to keep this opt-in and recover the missing selectors.
- [22 saved-original comparisons](metrics/m44-parity-22-20261001.json): three
  WAVs change (013, 016, 021), nineteen remain identical to M43. All three
  changed spectral-shape scores improve; mean gain-normalized MFCC-DTW changes
  36.930 -> 35.739. Estimated voiced match coverage remains low, 30.26% ->
  33.56%; conditional F0 error slightly worsens. These estimates cannot certify
  natural intonation or correct phoneme alignment.
- [22-phrase step screen](metrics/m44-transients-22-20261001.json): 36 -> 36
  steps above 12,000 and 5 -> 5 above 16,000; no higher per-phrase maximum.
  Duration MAE versus saved originals changes 46.557 -> 44.091 ms. This is not
  an independent new click fix.
- [Three user pairs](metrics/m44-rhythm-user-20261001.json): only startup
  changes; mean shape cost 43.780 -> 43.446 and duration MAE 127.333 ->
  126.767 ms. Aligned excess quiet at `#` joins is unchanged at 453.6 ms.
  [Step counts](metrics/m44-transients-user-20261001.json) also stay unchanged.
- [Wrapper/isolation checks](metrics/m44-wrapper-isolation-20261001.json): nine
  GUI startup jobs, 90 EXE/batch WAV pairs, and 80 old-profile WAV pairs versus
  the previous M43 EXE. Every old-profile pair is byte-identical. Fresh M43
  renders also match all 22 previous corpus WAVs and all three previous user
  WAVs. No reference data, decoded dictionary dump or audio is committed.

## Reproduction

Use a Win32 Release build with static runtime for the packaged GUI, and a fresh
output directory. Local `Elan` inputs remain outside Git. The x64 build can read
resources but the optional original primitive probe requires x86.

```powershell
cmake -S . -B build-talker -A Win32 -DNICOLAI_STATIC_RUNTIME=ON -DBUILD_TESTING=ON
cmake --build build-talker --config Release --parallel
ctest --test-dir build-talker -C Release --output-on-failure
python tools/test_m44_lexicon.py --probe build-talker/Release/nicolai_m44_lexicon_probe.exe `
  --voice C:\path\to\Elan\nicolai16.dat --original C:\path\to\Elan\mtsyc32.dll
python tools/audit_lexicon_m44.py --probe build-talker/Release/nicolai_m44_lexicon_probe.exe `
  --voice C:\path\to\Elan\nicolai16.dat --exceptions C:\path\to\Elan\exc_rus.txt `
  --out metrics-work/m44-resource-agreement.json
python tools/test_windows_talker.py --exe build-talker/Release/NicolaiTalker.exe `
  --voice C:\path\to\Elan --batch build-talker/Release/nicolai_batch_render.exe `
  --baseline-exe C:\path\to\previous-M43\NicolaiTalker.exe
```

The GUI selection is **Порт — M44 словарь форм (эксперимент)**. The CLI selection is
`m44-lexicon`. For the batch renderer, start with no inherited `NICOLAI_*` flags
and apply the existing M43 recipe plus `NICOLAI_M44_LEXICON_STRESS=1`:

```powershell
$env:NICOLAI_M40_BLEND_UNCOVERED='1'
$env:NICOLAI_M41_PRESERVE_JOINS='1'
$env:NICOLAI_M41_INTERPOLATE_UNCOVERED='1'
$env:NICOLAI_M42_JOIN_PERIOD_CONTINUITY='0.5'
$env:NICOLAI_M42_LOCAL_JOIN_PITCH='1'
$env:NICOLAI_M43_WORD_RHYTHM='0.25'
$env:NICOLAI_M43_FIXED_IKA_STRESS='1'
$env:NICOLAI_M44_LEXICON_STRESS='1'
build-talker/Release/nicolai_batch_render.exe C:\path\to\Elan\nicolai16.dat `
  C:\path\to\Elan\exc_rus.txt C:\path\to\Elan\abb_rus.txt `
  tools/parity_corpus_22.tsv metrics-work/m44-trial-22
python tools/measure_parity_v2.py C:\path\to\reference_pack metrics-work `
  --candidates m44-baseline-22 m44-trial-22 --json metrics-work/m44-parity.json `
  --cache metrics-work/m44-feature-cache
python tools/audit_transients_m40.py metrics-work/m44-baseline-22 `
  metrics-work/m44-trial-22 --output metrics-work/m44-transients.json
```

Render the M43 baseline before setting the M44 flag. The generic paired rhythm
tool remains `audit_rhythm_m43.py` (its M43 schema describes the measurement,
not the trial profile); supply saved original WAVs, paired output directories,
logs and the matching corpus as documented by `--help`.

## Next boundary

Recover `-` ending-stress authoring and the actual grammatical selectors, then
reflexive/participle/context selection, before expanding lexical acceptance.
Separately capture original phrase-level pause, duration and pitch authoring
from the word/morphology result. Correct lexical stress alone cannot restore
original phrase rhythm, acoustic prominence or a shared phase clock. Do not
promote this profile based only on resource agreement or MFCC-DTW.
