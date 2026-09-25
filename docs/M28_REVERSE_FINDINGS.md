# M28 reverse findings — exact physical marker selector topology

M28 continues the Windows Nicolai reverse against the supplied MSI,
`mtsyc32.dll`, `nicolai16.dat`, and the same 22-WAV PC oracle corpus.  No phrase
or corpus ID is special-cased.

## 1. `exc_rus.txt` proves the legacy orthographic marker grammar

The original MSI's Russian exception file documents two independent syntaxes:

- `[...]` — phonetic codes;
- `<...>` — orthographic codes;
- the second `<` marker follows the stressed vowel.

Examples in the shipped file include forms equivalent to:

```
абажур : <абажу<р> /i
абака  : <абака<>  /i
```

The second example is important: when the stressed vowel is the final spoken
orthographic segment, the stress marker is literally an empty marker `<>`.
This explains the special `next == '>'` branch in `0x10214f40`.

## 2. `[t%d]` is inserted at the marker position

The writer formats `[t%d]` at `0x1021578a` and calls `0x1019eca0` as:

```
insert(formatted_t, current_word_string, marker_index)
```

The insertion index comes from the marker search result produced earlier by
`0x1019eb60`.  `0x1019eca0` is a normal string-insertion helper.

The downstream parser `0x1019e530` advances one 0x20-byte feature record only
for a recognized Russian letter.  If that letter is immediately followed by
`[t...]`, successive t-values are stored in record fields `+0x10/+0x14/+0x18`.
Therefore a stress marker placed after a vowel becomes:

```
vowel[t0][t1][t2]<...
```

and the three values are attached to the feature record of the preceding vowel.
This closes the marker-to-feature placement ambiguity left by M27.

## 3. Exact `physical.int` selector columns

For a 42-byte `physical.int` class record:

- 18 / 19: alternate start/base values;
- 20: end/base value;
- lane 0 additives: 21..27;
- lane 1 additives: 28..34;
- lane 2 additives: 35..41.

The t-writer uses six of the seven additive columns:

| selector | lane 0 | lane 1 | lane 2 |
|---|---:|---:|---:|
| k0 | 21 | 28 | 35 |
| k1 | 22 | 29 | 36 |
| k2 | 23 | 30 | 37 |
| k3 | 24 | 31 | 38 |
| k4 | 25 | 32 | 39 |
| k5 | 26 | 33 | 40 |
| k6 | 27 | 34 | 41 |

No t-writer path observed in `0x10214f40` references **k5**.  k0/k1/k2/k3/k4/k6
are all referenced.

## 4. Exact base 18 versus base 19 decision

Before authoring t-nodes, the PC code counts words in the current span whose
orthographic representation contains at least one `<` or `>` marker (with the
legacy `(/)` special case handled separately).  That count is compared with
`wordstr[24]`, which is **2.0** for the supplied Nicolai database.

Recovered selection:

```
marker-bearing word count <= 2 -> record[18]
marker-bearing word count >  2 -> record[19]
```

Portable M28 exposes this exact decision through
`legacy_physical_base_slot_pc()`.

## 5. Exact empty-marker `<>` branches

The writer separately counts all `<` occurrences in the current span and tracks
the current marker ordinal.  For a marker whose next byte is `>`:

- if it is the final `<` globally: **k4**, with `record[20]` as the base;
- first non-final empty marker: **k0**, using the selected start/base directly;
- subsequent non-final empty marker: **k2**, using exact
  `0x10215d30` interpolation.

The final branch is the k4 path that M27 had previously projected broadly onto
the last active phone.  M28 corrects the interpretation: k4 is exact for the
**final global empty stress marker**, not for every phrase-final phone.

The portable helper `legacy_physical_empty_marker_triplet_pc()` now implements
these branches exactly.

## 6. Other marker families

Further static tracing establishes the remaining selector topology:

- literal `<<` uses **k6**;
- ordinary single-`<` paths use **k1/k3**, selected by an older context flag;
- `<<<` is inserted by the proprietary marker machinery and also participates
  in the physical-class logic recovered in M27;
- **k5 remains unreferenced** by the t-writer.

The exact linguistic/context guard that chooses k1 versus k3 is still the main
unrecovered selector needed before enabling the full physical lattice.

## 7. Conservative portable experiment

M28 maps only the now-proven standard final-vowel `<>` case onto the stressed
vowel feature record.  Terminal `<>` retains M27's already validated 0.016
terminal blend.  A separate environment control,
`NICOLAI_PHYSICAL_EMPTY_MARKER_PITCH_STRENGTH`, therefore affects only the new
non-terminal empty-marker contribution.

A small 22-WAV grid did not produce a robust joint improvement:

| non-terminal `<>` strength | active corr | F0 contour MAE |
|---:|---:|---:|
| 0.000 | 0.1953980 | 13.4680% |
| 0.002 | 0.1953971 | 13.4688% |
| 0.004 | 0.1954906 | 13.4830% |
| 0.006 | 0.1952760 | 13.4363% |
| 0.008 | 0.1943171 | 13.4363% |

The 0.004 point gives a tiny raw-correlation gain but worsens the F0 trajectory;
0.006 improves F0 but loses raw correlation.  Production therefore keeps the
new non-terminal strength at **0.0** rather than selecting a corpus-dependent
micro-optimum.

## Next

M29 should finish the ordinary single-`<` path: identify the context bit that
selects k1 versus k3, reproduce the exact marker span/ordinal state rather than
a two-markers-per-word approximation, and then retest the full physical pitch
lattice.  Once ordinary stress markers are faithful, the M24/M25 reconstructed
contour can begin to be removed rather than merely blended with it.
