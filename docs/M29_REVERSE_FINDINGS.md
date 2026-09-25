# M29 reverse findings — ordinary `<` stress marker writer

M29 closes the main selector ambiguity left by M28 around the ordinary
orthographic stress marker handled by `mtsyc32.dll:0x10214f40`.  All reverse
claims below refer to the supplied Nicolai/SpeechCube 5.1 MSI and its
`mtsyc32.dll`; no phrase-specific rules are introduced.

## 1. M28 correction: interpolation state is vowel-based

M28 correctly placed `[t%d]` at the orthographic marker position, but its
portable diagnostic still approximated interpolation with synthetic marker
ordinals.  Static tracing in M29 identifies the actual counters.

The global at `0x1072fe74` points to a CP866 vowel string at `0x1073003c`:

```
АЕЁИОУЫЭЮЯаеёиоуыэюя
```

The writer scans the current authoring span against that set:

- `EBX` accumulates the **total vowel count** in the span;
- `EBP` is the zero-based **current vowel ordinal**.

Those are the `position`/`count` arguments passed to the interpolation helper
`0x10215d30`.  Portable M29 therefore maps the frontend's stressed-vowel
ordinal back to the corresponding phone and uses the global vowel ordinal and
total vowel count rather than `2*word_index+1`.

## 2. The k1/k3 selector is dead in this Nicolai build

M28 documented an apparent ordinary-`<` split between k1 and k3.  M29 follows
the controlling stack local through the full reachable CFG.

Relevant addresses:

- `0x1021516f`: entry into the relevant writer region;
- `0x1021517b`: stores zero (`EBP==0` at this point) to the local at effective
  baseline `ESP+0x3c`;
- `0x10215b13`: reloads that same local;
- `0x10215b1d`: tests it to choose the k1 versus k3 branch.

Between initialization and the test there is no reachable write to that
stack slot and no `lea`/alias of it passed to a callee.  Consequently the
non-zero k3 branch is unreachable for this binary: the live ordinary-`<` path
always uses **k1**.

For the three pitch lanes this means:

```
k1 = record[22], record[29], record[36]
```

k3 remains present in the generic SpeechCube code image but is not selected by
the supplied Nicolai engine along this path.

## 3. Exact wordstr[33] bias

The prologue at `0x10214f65..0x10214f98` computes:

```
(wordstr[33] * 0.5) - wordstr[33]
```

and converts it through the legacy `_ftol` path.  Nicolai's exact
`wordstr[33]` is `20`, so the bias is **-10**.

The ordinary marker start value is therefore:

```
start = trunc(-0.5 * wordstr[33]) + record[base_slot]
```

where the already recovered M28 base-slot rule is:

```
marker-bearing word count <= wordstr[24] (2.0) -> record[18]
marker-bearing word count >  wordstr[24]       -> record[19]
```

The interpolation target is `record[20]`.

## 4. Exact ordinary-`<` triplet

For each pitch lane:

```
additive = record[22 + lane*7]       # k1
start    = trunc(-0.5*wordstr[33]) + record[18 or 19]
end      = record[20]

if vowel_count <= 1:
    t = additive + start
else:
    t = trunc((additive + start)
              + (end - start) * vowel_position / (vowel_count - 1))
```

This is implemented as `legacy_physical_single_marker_triplet_pc()` and uses
the previously recovered `legacy_physical_interp_add_pc()` semantics.

A class-1 unit fixture verifies, with Nicolai defaults:

- <=2 marker-bearing words, vowel 0/4 -> `[15, 13, 10]`;
- <=2 marker-bearing words, vowel 3/4 -> `[-5, -7, -10]`;
- >2 marker-bearing words (byte 19) at vowel 0/4 -> `[25, 23, 20]`.

## 5. Zero-strength isolation bug fixed

The first M29 experiment exposed a subtle portable-only interaction.  Even
when the new single-marker local strength was zero, replacing the generic
physical lattice value could affect the midpoint of a neighbouring endpoint
whose terminal strength was non-zero.  That meant a nominally disabled layer
was not acoustically disabled.

M29 now writes the recovered single-marker triplet into the acoustic lattice
only when `physical_single_marker_pitch_strength > 0`.  The exact helper and
marker flags remain available for tests/diagnostics.  With the production
strength at zero, all 22 M29 WAVs are byte-identical to M28.

## 6. Golden-corpus experiment

The exact ordinary-marker path was tested separately rather than silently
enabled.  A small blend can improve one objective while regressing another.
Representative measurements:

| single `<` strength | terminal strength | mean active corr | F0 contour MAE |
|---:|---:|---:|---:|
| 0.000 | 0.016 | 0.1953980 | 13.4680% |
| 0.001 | 0.015 | 0.1963091 | 13.4982% |
| 0.003 | 0.018 | 0.1949435 | 13.4416% |

The first acoustic candidate raises raw alignment correlation but worsens F0;
the second improves F0 but loses correlation.  Tiny declination retunes were
also checked and did not yield a robust joint win.  Production M29 therefore
keeps the new ordinary-marker blend at **0.0**.

## 7. Remaining uncertainty

The ordinary single-`<` path itself is now substantially closed.  The largest
remaining physical-prosody uncertainties are no longer k1-vs-k3; they are:

- exact span construction / marker-bearing-word state for every proprietary
  exception and inserted marker family;
- the remaining `<<` / `<<<` interactions and context paths;
- how the legacy authoring records interact with the still-reconstructed
  M24/M25 global F0 backbone;
- PSOLA join/phase parity, which now dominates raw sample correlation.

## Next

M30 should stop treating the surviving physical nodes as weak corrections and
recover the **full authoring-span state machine** around `0x10214f40` (including
`<<`, `<<<`, exception spans and exact per-span base-slot counts).  Once that
state is faithful, the physical lattice can be evaluated as a replacement for
more of the reconstructed M24/M25 contour instead of merely blended into it.
