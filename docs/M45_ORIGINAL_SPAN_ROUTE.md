# M45 — original phrase-boundary producer and isolated oracle

This is a diagnostic checkpoint, not a new acoustic profile. The user asked
to look directly in the original instead of continuing blind parameter tuning.
The local `mtsyc32.dll` is the primary source. No original code, lexical-list
dump, DLL, voice data or WAV is redistributed.

## What is new

M31 recovered the final dynamic splitting portion of `0x10216c70` but left
the producer of its raw space/underscore separator lattice unresolved. That
producer is the **initial portion of the same function**, beginning at
`0x10216ca4`. It considers trailing punctuation, embedded lexical categories
and neighboring morphology candidates before the previously recovered split
passes. It is not the numerical-inflection pass at `0x101a00e0`.

The byte tested at `0x10217467` / `0x102175bd` is the first byte of each
`0x59c`-byte morphology block: **candidate count**, not a dedicated skip flag.
The original counts a scanned successor when its word contains `<` OR its
candidate count is zero. An unmarked item with candidates is not counted.
The synthetic oracle distinguishes all three states explicitly.

## Verified caller sequence

The calls occur consecutively in the original sentence dispatcher:

| Call site | Target | Inspected responsibility |
| --- | --- | --- |
| `0x101a17b7` | `0x101a1830` | Normalize candidate form codes and boundary text |
| `0x101a17bd` | `0x10217720` | Candidate/context pass |
| `0x101a17ee` | `0x10216c70` | Author raw separators, convert and dynamically split |
| `0x101a17f4` | `0x10216970` | Contextual marker preparation |
| `0x101a17fa` | `0x101a1bf0` | Fallback stress-marker normalization |
| `0x101a1800` | `0x1019e350` | Build source phonetic descriptors |
| `0x101a1806` | `0x10215de0` | Select physical intonation class |
| `0x101a180c` | `0x10214f40` | Author `[l]`, `[e]`, `[t]` parameters |
| `0x101a1812` | `0x10212df0` | Construct/consume runtime prosodic records |

Between the second and third rows, the caller initializes all raw separators
to spaces (`0x101a17d1..0x101a17e5`) and calls the numerical-inflection pass
at `0x101a17e8`. This ordering matters: the later stress normalizer cannot
retroactively supply marker counts to the earlier splitting function.

## Separator function contract

The original accepts one caller-owned state pointer and uses:

| State offset | Input/output |
| --- | --- |
| `+0x124` | Morphology blocks, stride `0x59c`, one-based word index |
| `+0x128` | Raw separator bytes |
| `+0x12c` | Four-byte boundary-code slots |
| `+0x134` | Last word index/count |
| `+0x13c` | Annotated word pointer array |

The initial raw producer forces the last separator to comma. Trailing
punctuation in the inspected `, : ( ) _` category can set an internal comma;
there is also a lexical exception branch. Embedded lexical categories can
author `+` or protect neighboring spaces by changing them to `=`. Morphology
predicates can change eligible spaces to `_`. Those byte meanings are about
the boundary selection lattice, **not measured pause milliseconds**.

Two table-free grammar predicates are dynamically verified:

- current candidate kind 4 followed by kind 6/form 60 permits `_`;
- current kind 10 followed by one of the inspected kind values permits `_`.

Negative controls change the next form/kind. Existing `=` and `+` bytes are
not overwritten by these guarded space-to-underscore rules. Other grammar
branches depend on state tables and remain outside this oracle.

The conversion writes final `///`, internal comma `//`, and otherwise an
empty code. The minor pass searches space candidates at effective count
`>= 5` and writes `/`; the hard pass searches underscore candidates at
effective count `> 6` and writes `(/)`. The midpoint is integer floor, with
lower-index ties. Scan initialization, successor counting and resuming after
an insertion are significant: the M31 one-decision helper is not a faithful
whole-function replacement.

For example, six unmarked synthetic words with zero candidates and spaces
receive `/` at word 3; with one candidate each, they do not. Eight counted
items on an underscore lattice receive `(/)` at word 4. These are controlled
intermediate inputs, not claims about the original's analysis of real prose.

## Validation and isolation

Current port gap: `legacy_authoring_spans_m30()` in `src/legacy_prosody.cpp`
forms spans from frontend sentence/question/exclamation/semicolon boundaries
or the last word. It does not consume this original raw separator lattice.
`legacy_authoring_split_decision_m31()` has callers only in its unit tests,
not in synthesis. M44 lexical stress is connected, but this original complete
span route is not. This is a confirmed architectural difference, not proof
that it alone explains all audible rushing, stress or crackling.

[Recorded scalar result](metrics/m45-original-spans-20261001.json): **164/164**
original-function cases agree with the independent scan/search model:
144 uniform controls (12 lengths, four lattices, three count/marker states),
eight punctuation controls and 12 positive/negative morphology controls.
Caller arrays retain their guard bytes. Two negative invocations reject
missing arguments and an unsupported image before original execution.
The fresh Win32 Release build also passes the full **33/33 CTest** suite.

The optional Win32 x86 `nicolai_m45_span_probe` checks SHA256, PE architecture,
timestamp, image size and function entry before executing. It maps with
`DONT_RESOLVE_DLL_REFERENCES`: no proprietary DLL entry point, SAPI process,
server initialization or voice-data loading. In its own disposable process,
it binds only the three inspected system `msvcrt` imports (`sprintf`,
`strchr`, `strncmp`). No installed binary is modified. Do not extend its
calls to other original functions while their imports/state are unresolved.

The wrapper imposes a 30-second timeout. Only short invented ASCII words,
synthetic candidate records and punctuation are passed. Manual `_`, `=`, `+`
lattices test downstream behavior; they are **not** the real dispatcher's
initialization, which starts with spaces.

```powershell
cmake -S . -B build-m45 -A Win32 -DNICOLAI_STATIC_RUNTIME=ON -DBUILD_TESTING=ON
cmake --build build-m45 --config Release --target nicolai_m45_span_probe
python tools/test_m45_spans.py --probe build-m45/Release/nicolai_m45_span_probe.exe `
  --original C:\path\to\Elan\mtsyc32.dll
```

For an NMake build, use an x86 Visual Studio developer command prompt, omit
`-A Win32`, set `-DCMAKE_BUILD_TYPE=Release`, and use the EXE at the build root.
`python tools/test_m45_spans.py` alone runs voice-free model contracts. CI
builds the optional x86 probe and runs model contracts, but cannot execute the
proprietary original oracle without local inputs.

## Next targeted boundary

1. Capture the actual candidate and annotated-word arrays immediately before
   `0x10216c70`, using a separately owned original session. Validate a complete
   real-text route; do not infer candidate presence from the M44 stress lookup.
2. Reproduce raw separator production with the actual grammatical selectors,
   then validate full boundary arrays before integrating them into the port.
3. Follow those same records through `0x1019e350`, `0x10214f40` and
   `0x10212df0` to phone-local `[l]/[t]` values and their clock. M31 already
   identifies `[l]` as `base_duration * (1 + l/100)`; enabling it on mismatched
   records would not reproduce original rhythm.

This narrows the investigation to a confirmed route. It does **not** establish
full morphology, physical authoring, silence duration, F0 or PCM parity. Stable,
M44, the user's test EXE and all renderer defaults remain unchanged.
