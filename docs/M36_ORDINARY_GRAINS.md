# M36 ordinary repeated-grain geometry

This note isolates the statically proven repeated-grain loop inside original
`mtsyc32.dll` function `0x101083b0`. It remains separate from renderer promotion.

## Original loop

After the first grain, ordinal starts at 1. The loop at
`0x10108564..0x1010869e` reconstructs each later period from the step record:

```text
correction = sar11(delta_q11 * ordinal + 1024)
period_WORD = first_period_WORD + correction
```

The final add is a 16-bit `add di, WORD [...]`, not an idealized integer add.
Therefore signed-WORD wrap is part of the original behavior.

## Source interval widths

The original computes interval widths with WORD subtraction from the DWORD
source-position array:

```text
current_width = WORD(pos[i + 1]) - WORD(pos[i])
```

For non-terminal intervals:

```text
next_width = WORD(pos[i + 2]) - WORD(pos[i + 1])
```

For the terminal interval (`i == node_count - 2`) the next width is not read
past the array; the original reuses `current_width`.

## Writer-oriented window lengths

`left` and `right` here mean the actual arg2/arg3 inputs of `0x10109980`, not
chronological sides of the source interval. The writer call proves:

```text
left_window_length  = min(next_width, period)     # writer arg7
right_window_length = min(current_width, period)  # writer arg9
```

These two lengths are independently passed through the recovered window lookup
path. This differs from the old M34 adapter, which used one
`support = min(current_width, period)` for both inputs.

## Repeated-grain writer source coordinates

The writer call at `0x10108616..0x10108652` proves:

```text
boundary = pos[i + 1]

left_source_start  = boundary
right_source_start = boundary - right_window_length
```

So writer arg2 begins at the following/future side of the boundary, while
writer arg3 is the tail of the current/past interval. The right input is then
placed at the end of the output period by `legacy_tds_write_m34()` and its
window is consumed in reverse, matching the original Q15 writer contract.

This distinction is easy to mislabel because the chronological past source is
physically left of the boundary but is the writer's **right** input. The M36
portable fields intentionally follow writer argument order.

That source ownership differs from the old experimental stateful adapter,
which reselected interval-front and interval-tail slices for every grain.

## Portable contract

`legacy_runtime_ordinary_grain_m36()` reproduces:

- Q11 period progression;
- WORD-wrap period semantics;
- current/next interval widths;
- terminal next-width reuse;
- exact writer-left / writer-right window lengths;
- exact writer arg2 / arg3 source starts.

`legacy_runtime_grain_m36_test` locks positive and negative Q11 cases, terminal
behavior, WORD source-position subtraction, period wrap and explicit writer
source coordinates.

The helper is linked into `nicolai_port` but is not yet used by
`StatefulTdsM34` or production synthesis.

## Integration boundary

Do not promote this helper by itself. The first-grain bridge selection,
repeated-grain geometry, cross-descriptor transition path and recovered window
cache must agree as one state machine before an opt-in audio experiment.
