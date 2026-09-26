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
Q15 writer call. It constructs two temporary transition buffers and later feeds
them through four distinct writer phases.

## Cross geometry

`legacy_runtime_cross_geometry_m36()` identifies the previous source interval
and current source interval, including the saved-index branch controlled by
runtime `word2c/word2e`. It recovers previous/current widths, adjacent widths,
window extents and source starts.

## Primary temporary buffer

The first temporary buffer has total length `previous_interval_width` and is
partitioned in this order:

```text
zero prefix -> one-source shoulder -> two-source overlap
```

For previous/current window extents `L` and `R`:

```text
zero_prefix = previous_width - max(L, R)
shoulder    = abs(L - R)
overlap     = min(L, R)
```

The shoulder belongs to Previous when `L > R`, Current when `R > L`, and is
absent when equal. This is exposed by `legacy_runtime_cross_primary_layout_m36()`.

## Secondary temporary buffer

The second temporary buffer has total length `current_interval_width`. Define:

```text
A = min(previous_width, current_width)
B = min(current_width, current_next_width)
```

Its region order is the mirror image:

```text
two-source overlap -> one-source shoulder -> zero suffix
```

with:

```text
overlap     = min(A, B)
shoulder    = abs(A - B)
zero_suffix = current_width - max(A, B)
```

Shoulder ownership follows the larger extent. This is exposed by
`legacy_runtime_cross_secondary_layout_m36()`.

## Cross overlap arithmetic

The two-source central overlap uses a different final scale from the ordinary
writer:

```text
sum = wrap32(left_sample * left_q15 + right_sample * right_q15)
out = WORD(sar16(sum))
```

`legacy_runtime_cross_overlap_sample_m36()` preserves this exact wrapped
32-bit + `SAR 16` behavior. It must not be replaced by the ordinary Q15 writer.

## Four exact writer phases

After the temporary buffers are constructed, the tail of `0x10108cf0` makes
four families of calls to original writer `0x10109980`. Portable M36 represents
each call as `LegacyRuntimeCrossWriterPlanM36`, where left/right mean actual
writer arg2/arg3 ownership.

### 1. Cross entry / buffered first grain

```text
period = buffered_first_period
len = min(previous_width, period)

left  = PreviousPcm at previous_boundary, window len
right = PrimaryTemp at previous_width - len, window len
```

Portable: `legacy_runtime_cross_entry_plan_m36()`.

### 2. Buffered-step repeated grains

```text
period = WORD(buffered_first_period +
              sar11(buffered_delta_q11 * ordinal + 1024))

left_len  = min(current_width, period)
right_len = min(previous_width, period)

left  = SecondaryTemp at 0, window left_len
right = PrimaryTemp at previous_width - right_len, window right_len
```

Portable: `legacy_runtime_cross_buffered_repeat_plan_m36()`.

### 3. Current-step entry grain

```text
period = current_first_period
len = min(current_width, period)

left  = SecondaryTemp at 0, window len
right = CurrentPcm at current_boundary - len, window len
```

Portable: `legacy_runtime_cross_current_entry_plan_m36()`.

### 4. Current-step repeated grains

```text
period = WORD(current_first_period +
              sar11(current_delta_q11 * ordinal + 1024))

left_len  = min(current_next_width, period)
right_len = min(current_width, period)

left  = CurrentPcm at current_boundary, window left_len
right = CurrentPcm at current_boundary - right_len, window right_len
```

Portable: `legacy_runtime_cross_current_repeat_plan_m36()`.

The fourth phase converges on the same writer orientation recovered for ordinary
repeated grains.

## Current integration boundary

The nonzero cross path no longer has unknown writer source ownership or region
lengths in the statically traced positive domain. Remaining executor work is to
materialize both temporary PCM buffers with the recovered region arithmetic,
resolve source objects from each writer plan, use exact recovered windows, and
apply the existing output/runtime state transitions around those calls.

The helper near `0x1010a930` appears to service a side metadata/event object,
not the PCM mixer itself; it is intentionally kept outside the audio executor
until that ownership is independently proven.

No recovered cross primitive is wired into production or `StatefulTdsM34` yet.
