# M36 initial-transition source bridge

This note records the statically proven source geometry inside original
`mtsyc32.dll` function `0x101086c0`. Production synthesis remains unchanged.

`left` and `right` below follow the actual arg2/arg3 source order of
`0x10109980`, not chronological position on the source waveform.

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

left_len  = min(saved_right_width, period) # writer arg7
right_len = min(current_width, period)     # writer arg9

left_source_start  = p[saved + 1]                      # writer arg2
right_source_start = p[buffered + 1] - right_len      # writer arg3
```

This is exposed by `legacy_runtime_initial_source_selection_m36()`.
The three interval differences above are signed WORD subtractions from the
DWORD position array, matching the original positive runtime domain.

## Repeated grains

The repeated loop at `0x10108892..0x10108a1c` uses the same WORD period
progression recovered for the ordinary path:

```text
period_WORD = first_period_WORD + sar11(delta_q11 * ordinal + 1024)
```

Its past/current interval is the buffered interval. Runtime `state + 0x30`
decides where the writer-left/future input comes from.

### Staying inside the buffered descriptor

```text
current_width = prev[buffered + 1] - prev[buffered]
next_width    = prev[buffered + 2] - prev[buffered + 1]

left_len  = min(next_width, period)
right_len = min(current_width, period)

left_source_start  = prev[buffered + 1]
right_source_start = prev[buffered + 1] - right_len
```

### Crossing into the next descriptor

When `state + 0x30` is nonzero, writer arg2 switches to the next descriptor's
first PCM position while writer arg3 remains the buffered previous tail:

```text
current_width = prev[buffered + 1] - prev[buffered]
next_width    = next[1] - next[0]

left_len  = min(next_width, period)
right_len = min(current_width, period)

left_source_start  = next[0]
right_source_start = prev[buffered + 1] - right_len
```

The two writer source pointers therefore come from different PCM bases in the
cross-descriptor case. This is a real diphone bridge, not a
waveform-correlation join.

`legacy_runtime_initial_repeated_grain_m36()` exposes this proven geometry.

## Remaining initial-path work

The source coordinates, widths, period arithmetic and writer-window ownership
are now separate portable contracts. Renderer integration still needs to carry
the exact descriptor/PCM-base identity alongside these integer coordinates and
combine the result with the nonzero `0x10108cf0` cross-transition mixer.
