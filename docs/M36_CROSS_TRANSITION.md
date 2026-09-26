# M36 nonzero cross-transition mixer

This note records the statically proven architecture of original
`mtsyc32.dll` function `0x10108cf0`. Production synthesis remains unchanged.

## Dispatch split

The byte tested at `0x10108d13..0x10108d19` selects two fundamentally different
paths.

When it is zero, the original calls the ordinary renderer `0x101083b0` and then
applies a reverse recovered window over the just-written grain. This is exposed
by `legacy_runtime_cross_zero_fade_m36()`.

When it is nonzero, the original does **not** reduce the operation to one normal
Q15 writer call. It constructs transition PCM in temporary buffers and later
feeds the resulting material through the normal output state.

## First nonzero geometry

The recovered `legacy_runtime_cross_geometry_m36()` identifies the previous
source interval and current source interval, including the saved-index branch
controlled by runtime `word2c/word2e`. It recovers:

- previous interval width;
- previous-left width;
- current width;
- current-next width;
- previous/current window lengths;
- clamped source starts on both PCM bases.

The first temporary buffer is allocated as signed 16-bit PCM with a source
extent tied to the previous transition width.

## Three arithmetic regions

Static tracing of `0x10108f11..0x101090f7` proves three regions in the first
transition buffer:

1. optional zero padding;
2. one-source windowed shoulders using signed product `SAR 15`;
3. central two-source overlap using **wrapped 32-bit product sum followed by
   `SAR 16`**.

The central kernel is now portable as:

```text
legacy_runtime_cross_overlap_sample_m36(
    left_sample, left_window_q15,
    right_sample, right_window_q15)
```

Its arithmetic is intentionally different from `legacy_tds_write_m34()`. A
pair of equal near-unity Q15 products at sample value 10000 yields 9999 here,
not the roughly doubled value an ordinary Q15 overlap would produce.

The kernel also preserves x86 32-bit overflow before the arithmetic shift.

## Later cross stages

The remainder of `0x10108cf0` builds a second temporary transition region,
performs additional recovered window lookups, writes the transition into the
runtime output state and then advances later grains with the same signed-WORD
Q11 period progression used elsewhere.

The exact source/index ownership of every later temporary-buffer span is the
remaining static boundary before the nonzero cross path can be promoted into a
single portable executor.

## Integration rule

Do not substitute the ordinary Q15 writer for this cross mixer. Doing so changes
both source ownership and amplitude arithmetic. M36 keeps the recovered cross
geometry/kernel isolated until all buffer spans are named and tested.
