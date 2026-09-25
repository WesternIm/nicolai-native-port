# M23 findings — Nicolai pitch path

M23 moves the portable core from “preserve the database recording pitch” toward
the pitch model used by the original Windows Russian engine.

## `pitch.par` is a scalar parser, not a contour table

The Russian voice database contains the static path
`rusvox\\data\\pitch.par`, but its static target is zero.  The original DLL
contains the runtime parser at `mtsyc32.dll:0x10106880`.

The parser:

1. opens `pitch.par`;
2. reads one line (`fgets(..., 0x78, ...)`);
3. finds `=`;
4. calls `atoi` on the value;
5. writes the integer to runtime state `+0x81264`.

The engine initialization path writes `0x53` (decimal **83**) to that same
runtime field.  Therefore 83 is the proven built-in Russian pitch base when no
external override is active.

## Recovered PC pitch arithmetic

Function `0x101a2250` reads runtime `+0x81264`, subtracts it from an integer
pitch value, multiplies the deviation by the float at `wordstr + 0x70`, rounds,
and adds the base back.

For Nicolai, `wordstr + 0x70` is index 28 and equals **0.95**:

```
adjusted_pitch = base_pitch + (input_pitch - base_pitch) * 0.95
base_pitch = 83
```

Function `0x101a2310` independently normalizes an integer pitch by the same
base and clamps the ratio between `wordstr[20]` and `wordstr[19]`.  Nicolai's
values are:

```
wordstr[19] = 1.00
wordstr[20] = 0.90
```

so that path returns a ratio in **[0.90, 1.00]**.  M23 documents this branch but
does not yet feed it back into portable timing because the destination field's
full semantic role is still being recovered.

## Percent representation in the main prosody path

At `0x102272c0` the PC engine converts a per-item integer to absolute pitch as:

```
pitch = base_pitch * (1 + 0.01 * item_value)
```

The constant at `0x1023a318` is the double `0.01`.  This establishes that the
legacy prosody layer carries pitch movement as percentage offsets around the
runtime base rather than simply replaying the recorded database F0.

## Portable M23 reconstruction

The exact upstream generator that produces those percentage contour values is
not fully recovered yet.  M23 therefore separates two stages:

1. reconstruct a contour input from the robust median SEG period of each
   diphone, retaining `5/19` of the recorded deviation from 83 Hz;
2. apply the recovered PC `wordstr[28] = 0.95` transform exactly.

`(5/19) * 0.95 = 0.25`, so the current portable target retains one quarter of
the raw database F0 deviation from the 83-Hz centre.  `5/19` is a single
golden-corpus calibration coefficient for the missing upstream stage; it is
not claimed to be a recovered DLL constant and there are no phrase-specific
exceptions.

The public helper `legacy_pitch_target_f0_m23()` keeps this distinction explicit
and is covered by the host test suite.

## Rejected M23 experiments

A phone-side / half-diphone duration redistributor was implemented and remains
available as experimental infrastructure.  It can reduce duration error in
some settings, but all tested production strengths reduced golden-corpus
waveform alignment.  Production therefore keeps `phone_side_strength = 0`.

Likewise, broad CC/CV/VC/VV duration factors were tested and left at 1.0.

A simple global `pitch_scale = 0.90` improved F0 medians but was inferior to the
83-Hz centred per-unit model: it does not distinguish units recorded above and
below the legacy pitch centre.

## Next

M24 should recover the upstream percentage-contour generator and the exact
meaning of the pitch-normalized `[0.90,1.00]` branch, replacing the `5/19`
conformance reconstruction with the original contextual rule sequence.
