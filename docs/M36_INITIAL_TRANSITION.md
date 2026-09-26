# M36 initial-transition source bridge

This note records the statically proven source geometry inside original
`mtsyc32.dll` function `0x101086c0`. Production synthesis remains unchanged.

## First grain

The first initial-transition grain uses the buffered step record at runtime
`state + 0x74` and the saved interval index at `state + 0x2e`.

For source positions `p[]`:

```text
saved = state.word2e
buffered = buffered_step.word0
period = buffered_step.first_period

saved_right_width =
    saved < node_count - 2
      ? p[saved + 2] - p[saved + 1]
      : p[saved + 1] - p[saved]

current_width = p[buffered + 1] - p[buffered]

left_len  = min(saved_right_width, period)
right_len = min(current_width, period)

left_source_start  = p[saved + 1]
right_source_start = p[buffered + 1] - right_len
```

This is exposed by `legacy_runtime_initial_source_selection_m36()`.

## Repeated grains

The repeated loop at `0x10108892..0x10108a1c` uses the same WORD period
progression recovered for the ordinary path:

```text
period_WORD = first_period_WORD + sar11(delta_q11 * ordinal + 1024)
```

Its current interval is the buffered interval. The right side depends on
runtime `state + 0x30`.

### Staying inside the buffered descriptor

```text
current_width = prev[buffered + 1] - prev[buffered]
next_width    = prev[buffered + 2] - prev[buffered + 1]

left_len  = min(current_width, period)
right_len = min(next_width, period)

left_source_start  = prev[buffered + 1] - left_len
right_source_start = prev[buffered + 1]
```

### Crossing into the next descriptor

When `state + 0x30` is nonzero, the original switches the right source to the
first interval of the next descriptor while retaining the old buffered boundary
on the left:

```text
current_width = prev[buffered + 1] - prev[buffered]
next_width    = next[1] - next[0]

left_len  = min(current_width, period)
right_len = min(next_width, period)

left_source_start  = prev[buffered + 1] - left_len
right_source_start = next[0]
```

The two source pointers therefore come from different PCM bases in the writer
call. This is a real cross-diphone bridge, not a waveform-correlation join.

`legacy_runtime_initial_repeated_grain_m36()` exposes this proven geometry.

## Remaining initial-path work

The source coordinates, widths, period arithmetic and window ownership are now
separate portable contracts. The remaining work before renderer integration is
to preserve the exact descriptor/PCM-base state around this geometry and to
combine it with the nonzero `0x10108cf0` cross-transition mixer.
