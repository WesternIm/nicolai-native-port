# M31 reverse findings — physical length/energy authoring and dynamic span split

All addresses and table interpretations below refer to the supplied Nicolai / SpeechCube 5.1 Windows MSI and its `mtsyc32.dll` / `nicolai16.dat`.

## 1. `[l%d]` is a signed phone-duration percentage

The annotation parser stores `[l%d]` in source feature byte `+0x1d`. During record construction the byte is sign-extended and copied to the runtime per-phone dword at `state + unit*0x80 + 0x1d4 + phoneIndex*4`.

The runtime consumer around `0x10226f87` loads that integer, converts it to floating point, multiplies it by the embedded `0.01` constant and by the base duration, then adds the base duration. The effective formula is therefore:

```
phone_duration = base_duration * (1 + l_percent / 100)
```

This closes the semantic identity of `[l%d]`; it is not an arbitrary timing index.

## 2. Exact `physical.int` length profile selection

The shared authoring pass uses two five-byte length profiles:

- bytes `3..7` when the span marker-bearing-word count is `<= wordstr[24]`;
- bytes `8..12` when the count is `> wordstr[24]`.

For Nicolai, `wordstr[24] == 2.0`.

Within the selected profile, the writer chooses:

- current marker `>` -> element 0;
- otherwise, if the marker tail contains `<<` and the word contains `<<<` -> element 1;
- otherwise, if the marker tail contains `<<` -> element 2;
- otherwise -> element 3.

If the current marker ordinal is the last marker in the span, element 4 is added to the selected value.

Portable M31 implements this as `legacy_physical_length_percent_pc()` with the original signed-byte arithmetic.

## 3. Exact `physical.int` energy profile selection

`[e%d]` is parsed into source feature byte `+0x1c` and copied to the runtime per-phone byte at `+0x1cc`. The authoring values come from `physical.int` bytes `13..17`.

The starting selector is:

- current marker `>` -> q0;
- no `<<<` in the word -> q3;
- `<<<` plus `<<` from the current marker -> q1;
- `<<<` without that `<<` condition -> q2.

For a one-marker span the selected start value is emitted directly. For multiple markers, the Windows writer multiplicatively declines that start value toward terminal factor q4:

```
factor = 100 - (100 - q4) * (ordinal - 1) / (count - 1)
energy = round(start * factor / 100)
```

The positive-value rounding path is the observed `+0.5` followed by legacy `_ftol`. Portable M31 implements this as `legacy_physical_energy_percent_pc()`.

For the canonical class-1 record (`q = 70,80,90,100,30`), an ordinary three-marker span therefore yields `100, 65, 30`.

## 4. Lower-level dynamic authoring span splitter

M30 established that `0x10214f40` consumes `(/)`-bounded authoring spans. M31 moves upstream and isolates the splitter at `0x10216c70`.

The effective run count includes an item when either:

- its annotated text contains `<`; or
- its per-item legacy skip-count flag is clear.

Two split passes are visible:

1. a minor pass triggers at effective count `>= 5`, searches empty-code candidates carrying raw separator `' '`, and writes the lighter `/` code;
2. a hard pass triggers at effective count `> 6`, searches empty-code candidates carrying raw separator `'_'`, and writes the full `(/)` terminator.

Candidate selection is midpoint-nearest among eligible empty-code items. `legacy_authoring_split_decision_m31()` exposes this recovered primitive for diagnostics/tests. The producer that decides whether a raw candidate receives `' '` versus `'_'`, and the exact semantic name of the legacy skip-count flag, are still not fully named; production therefore does not invent those upstream states.

## 5. Why length/energy remain disabled in production

The PC applies `[l]` and `[e]` to individual phone feature records. The current portable synthesis path still renders a diphone as the principal timing/energy unit. M31 includes an experimental general projection of phone values onto the two sides of a diphone, but that projection is not yet equivalent to the PC runtime.

Golden-corpus sweeps show the expected warning sign:

- physical length strength 0.0005 changes correlation only in the fifth decimal while quantized duration is effectively unchanged;
- 0.001 and above begin to worsen waveform correlation / active-duration MAE;
- energy strengths from 0.002 to 0.02 do not produce a stable multi-metric improvement and progressively worsen RMS-amplitude error.

Accordingly production defaults stay:

```
physical_length_strength = 0.0
physical_energy_strength = 0.0
```

This is deliberate. The exact writer is now implemented, but it will not be enabled until the renderer can consume phone-local duration/energy state with the same granularity as Windows.

## 6. New tests

M31 extends `legacy_prosody_test` with direct checks for:

- short/long length-profile selection;
- ordinary, `>`, `<<`, `<<<`, and terminal-addition branches;
- energy decline (`100 -> 65 -> 30` for the canonical class-1 ordinary path);
- midpoint dynamic split selection for both `>=5` and `>6` passes.

All 20 host tests pass.

## Next

M32 should move the portable renderer from whole-diphone duration/energy projection toward **phone-local feature consumption while retaining M24+ three-point F0**. That is the missing execution-side match required before the now-exact `[l]`/`[e]` authoring values can be enabled at meaningful strength. In parallel, the upstream producer of the raw `' '` / `'_'` split-candidate lattice remains a reverse target for long-text parity.
