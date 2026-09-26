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

## Exact temporary PCM materialization

The loops at `0x10108f2d..0x10109328` are now represented by
`legacy_runtime_cross_buffers_m36()`. The primitive consumes the source spans
selected by `legacy_runtime_cross_geometry_m36()`, resolves the exact recovered
M36 windows and returns both buffers without touching renderer state.

For the primary buffer, let `W=previous_width`, `A=previous_window` and
`B=current_window`. The previous contribution starts at `W-A`, the current
contribution starts at `W-B`, and both source spans advance forward while their
windows are read backwards. Therefore the buffer is exactly:

```text
zero prefix -> longer-source SAR15 shoulder -> wrapped-sum SAR16 overlap
```

The primary source spans are:

```text
previous: prev[k]   - A .. prev[k]   - 1
current:  cur[i+1]  - B .. cur[i+1]  - 1
```

For the secondary buffer:

```text
P = min(previous_width, current_width)
C = min(current_width, current_next_width)
```

Both contributions begin at output offset zero and both windows are read
forward. The previous source starts at `prev[k]`. The current source starts at
`cur[i+1]`, except at the terminal current interval where it starts at
`cur[i]`. Its exact order is:

```text
wrapped-sum SAR16 overlap -> longer-source SAR15 shoulder -> zero suffix
```

Single-source shoulders use signed multiply followed by arithmetic `SAR 15`.
The overlap uses the distinct wrapped 32-bit sum and `SAR 16` above. Source
bounds and the recovered `1..400` exact-window domain are guarded; unsupported
inputs fail without partially returning a buffer.

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

## Post-write state and transactional executor

After every one of the four writer phases, the original performs the same
rotation:

```text
selection_b <- selection_a
selection_a <- old_cursor
cursor      <- old_cursor + period
```

This is exposed by `legacy_runtime_post_write_m36()`. The complete positive
nonzero path is composed by `legacy_runtime_execute_cross_m36()`, which resolves
all plan sources/windows, executes the buffered entry/repeats and current
entry/repeats, and stages both PCM and runtime state until every phase succeeds.
Details and static addresses are in `M36_CROSS_EXECUTOR.md`.

## Current integration boundary

The nonzero cross path no longer has unknown temporary-buffer samples, writer
source ownership, region lengths, PCM writes or cursor/selection rotation in
the statically traced positive domain.

The helper `0x10109fc0` following each write only releases non-cached temporary
window storage; portable vector ownership replaces that resource operation.
The earlier helper near `0x1010a930` appears to service a side metadata/event
object and remains outside the audio executor until that ownership is proven.

No recovered M36 executor is wired into production or `StatefulTdsM34` yet.

The zero-byte branch is also composed transactionally by
`legacy_runtime_execute_zero_cross_m36()`: it executes the recovered ordinary
writer sequence and applies the reverse-window fade to the first emitted period
only. Caller-owned route bookkeeping remains outside that PCM contract.
