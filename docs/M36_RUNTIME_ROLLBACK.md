# M36 runtime rollback and source-selection recovery

This note records exact state and source-selection behavior recovered statically
from the known original `mtsyc32.dll` around `0x10107c20`, `0x10107f65`, the
ordinary `0x101083b0` path, initial `0x101086c0` transition and the first
recoverable layers of `0x10108cf0`. It remains deliberately separate from
renderer integration: production and `StatefulTdsM34` do not call these
primitives yet.

## Per-interval write routing

After `0x10109800` produces a step record, the dispatcher at
`0x10107d54..0x10107ebe` applies this exact priority:

```text
if step.count == 0:
    dropped-step path
else if cross_descriptor_pending:
    0x10108cf0 cross-descriptor transition
else if no positive-count write has started yet:
    0x101086c0 initial transition
else if interval_index == node_count - 2:
    defer terminal handling until after the loop
else:
    save checkpoint and call 0x101083b0 ordinary/repeated path
```

`legacy_runtime_route_m36()` models this dispatch without assigning speculative
higher-level names to the caller gates.

## Checkpoint and rollback

Before the ordinary grain path (`0x10107e9f..0x10107ebb`), the original saves:

```text
state +0x48 -> +0x54   cursor
state +0x4c -> +0x58   selection A
state +0x50 -> +0x5c   selection B
```

A rewind occurs after `step.count == 0` only when:

```text
caller gate A == 0
interval_index == node_count - 2
caller/source gate B == 0
```

When the gate passes:

```text
old_cursor = state +0x48
new_cursor = state +0x54
delta      = new_cursor - old_cursor

+0x48 = +0x54
+0x3c += delta
+0x40 += delta
+0x4c = +0x58
+0x50 = +0x5c
```

Both clocks clamp to zero if negative. If saved cursor is below `+0x80`, then
`+0x80` becomes saved cursor. The branch also performs:

```text
WORD +0x28 -> +0x2e
WORD +0x24 -> +0x2c
DWORD +0x30 = 0
WORD +0x26  = 1
```

If rewind conditions fail, the common dropped tail still writes `+0x26 = 1`.

## Ordinary-path dropped-source bridge

For an ordinary interval `i`, no preceding drop uses:

```text
left_source_position = source_position[i]
left_interval_width  = source_position[i+1] - source_position[i]
```

When `state +0x26 != 0`, saved dropped index `k = +0x2a` changes the left side:

```text
left_source_position = source_position[k+1]
left_interval_width  = source_position[k+2] - source_position[k+1]
```

The right source still belongs to the current interval. With first period `P`:

```text
left_window_length   = min(left_interval_width, P)
right_window_length  = min(current_interval_width, P)
right_source_position = source_position[i+1] - right_window_length
```

So a dropped interval changes the actual source PCM of the next grain; it is not
only a clock bookkeeping event.

## Initial transition (`0x101086c0`) first grain

Let:

```text
k = state.word2e
b = buffered_step.interval_index
P = buffered_step.first_period
```

Right side:

```text
right_interval_width  = source_position[b+1] - source_position[b]
right_window_length   = min(right_interval_width, P)
right_source_position = source_position[b+1] - right_window_length
```

Left source always starts at:

```text
source_position[k+1]
```

For non-terminal `k`, left support is the following interval:

```text
source_position[k+2] - source_position[k+1]
```

For terminal `k`, it falls back to its own interval width:

```text
source_position[k+1] - source_position[k]
```

`legacy_runtime_initial_source_selection_m36()` contracts this ownership.

## Cross transition (`0x10108cf0`): zero-flag branch

When the descriptor-side byte tested by `0x10108cf0` is zero, the original:

1. saves the current output cursor;
2. renders through ordinary `0x101083b0`;
3. looks up a window of `first_period` samples;
4. multiplies the first newly written period by that window **in reverse order**.

For newly written sample `i`:

```text
pcm[i] = arithmetic_shift_right(
    pcm[i] * window[first_period - 1 - i], 15)
```

This is a descriptor-boundary fade-in. It is implemented by
`legacy_runtime_cross_zero_fade_m36()` using the recovered M36 window lookup.

## Cross transition: nonzero-flag support geometry

The other `0x10108cf0` branch constructs two temporary transition buffers.
Their source/support ownership and exact PCM mixing loops are now recovered;
the formulas and portable primitive are documented in
`M36_CROSS_TRANSITION.md`.

Previous descriptor interval selection:

```text
if state.word2c != 0:
    previous_index = state.word2e + 1
else:
    previous_index = buffered_step.interval_index
```

Then:

```text
previous_width = prev[k+1] - prev[k]
previous_left_width = (k > 0) ? prev[k] - prev[k-1] : previous_width
previous_window = min(previous_left_width, previous_width)
```

For current descriptor interval `i`:

```text
current_width = cur[i+1] - cur[i]
current_window = min(current_width, previous_width)
current_next_width =
    (i < last_interval) ? cur[i+2] - cur[i+1] : current_width
```

The source starts used for the temporary blend geometry are:

```text
previous_source_start = max(0, prev[k]   - previous_window)
current_source_start  = max(0, cur[i+1] - current_window)
```

The forward-aligned secondary buffer additionally starts at:

```text
previous_forward_source_start = prev[k]
current_forward_source_start =
    (i < last_interval) ? cur[i+1] : cur[i]
```

`legacy_runtime_cross_geometry_m36()` contracts these values and whether the
saved-state override selected `word2e+1` instead of the buffered interval.

## Portable contracts

`tests/legacy_runtime_state_test.cpp` now covers:

- exact dispatcher priority;
- checkpoint/rollback and clock clamps;
- ordinary dropped-source bridging;
- initial-transition source ownership;
- zero-flag cross-transition reverse-window fade;
- nonzero cross-transition support geometry with both buffered and saved-state
  previous-interval selection;
- exact primary/secondary temporary PCM materialization, including both
  shoulder owners, zero regions and window directions;
- exact post-write cursor/selection rotation and transactional execution of all
  four nonzero cross writer phases;
- invalid saved-index rejection.

## Remaining transition work

The large unknown area is now narrower:

- route-level executor composition around the completed nonzero cross path;
- repeated-grain movement after first writes in `0x101083b0` / `0x101086c0`;
- post-loop terminal path around `0x10108210`;
- live Gate A/B capture validation and x87 last-bit window validation.

Exact window cache topology and the guarded `1..400` lookup are documented in
`M36_WINDOWS.md`. No recovered runtime component is promoted into production
synthesis yet.
