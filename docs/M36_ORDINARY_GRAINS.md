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

## Window lengths

For every repeated grain:

```text
left_window_length  = min(current_width, period)
right_window_length = min(next_width, period)
```

These two lengths are independently passed through the recovered window lookup
path before the Q15 writer. This differs from the old M34 adapter, which used a
single `support = min(current_width, period)` for both sides.

## Repeated-grain source coordinates

The writer call at `0x1010862f..0x10108652` proves that repeated grains are
centred on the source boundary at `pos[i + 1]`:

```text
left_source_start  = pos[i + 1] - left_window_length
right_source_start = pos[i + 1]
```

The assembly obtains the left coordinate as:

```text
pos[i] - left_window_length + current_width
```

which is exactly `pos[i+1] - left_window_length` in the positive source domain.
This is intentionally different from the *first* grain of the interval, whose
bridge/front-tail selection is handled by `legacy_runtime_normal_source_selection_m36()`.

That difference matters acoustically: the old experimental stateful adapter
reselected `[interval_start ...]` and `[..., interval_end]` for every grain,
whereas the original moves repeated grains to the shared interval boundary.

## Portable contract

`legacy_runtime_ordinary_grain_m36()` now reproduces:

- Q11 period progression;
- WORD-wrap period semantics;
- current/next interval widths;
- terminal next-width reuse;
- independent left/right window lengths;
- repeated-grain left/right source starts.

`legacy_runtime_grain_m36_test` locks positive and negative Q11 cases, terminal
behavior, WORD source-position subtraction, period wrap and explicit source
coordinates.

The helper is linked into `nicolai_port` but is not yet used by
`StatefulTdsM34` or production synthesis.

## Integration boundary

Do not promote this helper by itself. The first-grain bridge selection,
repeated-grain geometry, cross-descriptor transition path and recovered window
cache must agree as one state machine before an opt-in audio experiment.
