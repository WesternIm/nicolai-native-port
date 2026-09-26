# M36 ordinary repeated-grain geometry

This note isolates the statically proven part of the repeated-grain loop inside
original `mtsyc32.dll` function `0x101083b0`. It is deliberately separate from
renderer promotion and from still-unproven source-pointer offsets.

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

## Portable contract

`legacy_runtime_ordinary_grain_m36()` reproduces the proven arithmetic and
geometry only. It intentionally does not name the later source-pointer
adjustments until those offsets are fully traced.

`legacy_runtime_grain_m36_test` locks:

- positive Q11 period progression;
- negative arithmetic-shift behavior;
- terminal reuse of current width;
- WORD subtraction of source positions;
- WORD wrap of the final period add.

The helper is linked into `nicolai_port` but is not yet used by
`StatefulTdsM34` or production synthesis.

## Integration boundary

Do not promote this helper by itself. The first-grain bridge selection,
repeated-grain source pointers, cross-descriptor transition path and recovered
window cache must agree as one state machine before an opt-in audio experiment.
