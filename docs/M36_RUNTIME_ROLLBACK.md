# M36 runtime rollback and source-selection recovery

This note records exact state and source-selection behavior recovered statically
from the known original `mtsyc32.dll` around `0x10107c20`, `0x10107f65` and the
ordinary `0x101083b0` write path. It remains deliberately separate from renderer
integration: production and `StatefulTdsM34` do not call these primitives yet.

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

## Checkpoint state

Before the ordinary grain path (`0x10107e9f..0x10107ebb`), the original saves:

```text
state +0x48 -> +0x54   cursor
state +0x4c -> +0x58   selection A
state +0x50 -> +0x5c   selection B
```

`legacy_runtime_checkpoint_m36()` reproduces exactly those copies.

## Dropped-step gate

The branch at `0x10107f65` is entered only after the step record reports
`count == 0`. A rewind occurs only when all of the following are true:

```text
caller gate A == 0
interval_index == node_count - 2
caller/source gate B == 0
```

The two gate WORDs are intentionally not given semantic names yet; static code
proves the tests but not their complete upstream meaning.

If any rewind condition fails, the common dropped-step tail still stores
`1` to state `+0x26`.

## Exact rewind transform

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

Both clocks use 32-bit x86 arithmetic and are clamped to zero if negative after
the rewind.

If the restored cursor is below the existing `+0x80` marker, the original calls
`0x1010a860`. That helper returns state `+0x48`, and the caller stores the return
at `+0x80`; therefore the observable state transform is exactly:

```text
if saved_cursor < end_cursor:
    end_cursor = saved_cursor
```

The rollback then copies:

```text
WORD +0x28 -> +0x2e
WORD +0x24 -> +0x2c
DWORD +0x30 = 0
WORD +0x26  = 1
```

## Exact ordinary-path source bridge

The first write inside `0x101083b0` proves why the dropped marker matters for
audio rather than only bookkeeping.

For a normal interval `i` with no preceding drop:

```text
left_source_position  = source_position[i]
left_interval_width   = source_position[i+1] - source_position[i]
right_interval_width  = source_position[i+1] - source_position[i]
```

When state `+0x26 != 0`, the original instead uses the previously stored
`+0x2a` interval index `k` for the left source:

```text
left_source_position = source_position[k+1]
left_interval_width  = source_position[k+2] - source_position[k+1]
```

The right source still belongs to the current interval. With the step record's
first period `P`, both source widths are clamped independently:

```text
left_window_length  = min(left_interval_width, P)
right_window_length = min(current_interval_width, P)

right_source_position = source_position[i+1] - right_window_length
```

So a dropped interval causes the next ordinary first grain to bridge from the
right edge of the stored dropped interval to the tail of the current interval.
This is a concrete source-selection change, not merely a clock rewind.

`legacy_runtime_normal_source_selection_m36()` reproduces these source
coordinates and lengths. It intentionally stops before `0x10109be0`; exact
window table lookup/construction is still a separate recovery boundary.

## Portable contracts

`include/nicolai/legacy_runtime_state.hpp` and
`src/legacy_runtime_state.cpp` expose the recovered transforms without
connecting them to synthesis. `tests/legacy_runtime_state_test.cpp` covers:

- dispatch priority: drop / cross / initial / deferred terminal / ordinary;
- checkpointing;
- non-dropped records and non-terminal drops;
- both rollback gate blockers;
- clock clamping and selection restore;
- `+0x80` rewind;
- the common `+0x26` dropped marker;
- ordinary source ownership;
- exact dropped-interval source bridging;
- independent left/right period clamping.

## Remaining source-transition boundary

The ordinary first-grain bridge is no longer unknown. The remaining high-value
static/runtime work is concentrated in:

- `0x101086c0`: initial transition using buffered prior step/source state;
- `0x10108cf0`: cross-descriptor transition and descriptor/base selection;
- repeated-grain source movement after the first `0x101083b0` write;
- the post-loop terminal path around `0x10108210`;
- exact `0x10109be0` / `0x1010a020` window lookup/construction.

These must be recovered before wiring rollback/source bridging into
`StatefulTdsM34`, because a correct rewind combined with the wrong transition
source would still produce incorrect audio.
