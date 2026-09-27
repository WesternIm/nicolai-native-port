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

## Current-step entry

After the complete buffered step, the original block at
`0x10108a22..0x10108ba0` emits the first grain of the current step. Let `i` be
the current interval, `b` the buffered interval and `P` the current first
period. The interval differences remain signed WORD subtractions.

```text
cross = state.field30 != 0
current_width = cur[i + 1] - cur[i]

if cross:
    left_width  = cur[1] - cur[0]
    left_source = cur[0]
else:
    left_width  = prev[b + 2] - prev[b + 1]
    left_source = prev[b + 1]

left_len  = min(left_width, P)
right_len = min(current_width, P)

right_source = cur[i + 1] - right_len
```

The right source always belongs to the current descriptor. Only writer-left
changes PCM ownership with `state + 0x30`. The portable contract is
`legacy_runtime_initial_current_entry_m36()`.

## Current-step repeats

The loop at `0x10108bb8..0x10108ccf` uses the ordinary repeated-grain geometry
on the current descriptor, including the same WORD/Q11 period progression and
boundary-centred source starts. Both writer sources therefore belong to the
current descriptor after the entry grain.

## Transactional executor

`legacy_runtime_execute_initial_m36()` composes the complete proven
`0x101086c0` PCM sequence:

1. buffered entry;
2. buffered repeats;
3. current entry;
4. current repeats.

It carries previous/current PCM-base identity explicitly, applies the recovered
windows and writer for every grain, and performs the original post-write
cursor/selection rotation. All geometry and source ranges are validated before
commit; a failure in a later current phase exposes neither earlier buffered PCM
nor partial runtime-state changes.

## Remaining caller work

The initial-transition PCM body is complete as a portable contract. The
remaining boundary is caller-owned interval markers and buffered/current step
rotation around `0x10107c20`; production integration stays deferred until that
route-level bookkeeping is equally proven.
