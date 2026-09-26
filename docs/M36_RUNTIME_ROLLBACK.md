# M36 runtime rollback recovery

This note records the exact state mutation recovered statically from the known
original `mtsyc32.dll` around `0x10107c20` / `0x10107f65`. It is deliberately
kept separate from renderer integration: production and `StatefulTdsM34` do not
call this primitive yet.

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

## Portable contract

`include/nicolai/legacy_runtime_state.hpp` and
`src/legacy_runtime_state.cpp` expose this transform without connecting it to
audio synthesis. `tests/legacy_runtime_state_test.cpp` covers checkpointing,
non-dropped records, non-terminal drops, both gate blockers, clock clamping,
selection restore, `+0x80` rewind and the common `+0x26` dropped marker.

This narrows the remaining runtime work: the rollback mutation itself is no
longer unknown. What still needs an original runtime oracle is when the upstream
caller produces each gate/state combination and how buffered source selection
around `0x101086c0`, `0x10108cf0` and the ordinary `0x101083b0` path chooses the
samples/windows that accompany those state changes.
