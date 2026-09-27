# M36 deferred terminal PCM path

This note records the statically recovered post-loop terminal sequence around
original `mtsyc32.dll` function `0x10108210`. Production synthesis and
`StatefulTdsM34` remain unchanged.

## Dispatch and checkpoint

The main interval loop defers a positive-count last interval. After the loop,
`0x1010801b..0x1010807b` verifies that terminal work is pending, checkpoints
runtime cursor/selections and calls `0x10108210` with interval
`node_count - 2` and the last computed step record.

## Single terminal grain

`0x10108286..0x1010839d` repeats the ordinary first-grain source selection:

- current support is the terminal interval;
- when `state.word26 != 0`, writer-left source/support comes from the interval
  after saved dropped index `state.word2a`;
- otherwise writer-left uses the terminal interval itself;
- writer-right is the terminal interval tail;
- both supports are independently clamped to `step.first_period`.

Interval widths use the original signed WORD subtraction from DWORD source
positions; full positions remain the PCM coordinates used to form pointers.

The function performs exactly one `0x10109980` write and one cursor/selection
rotation. It does not enter the ordinary repeated-grain loop, even if the
positive `step.count` is greater than one.

## Caller-side fade-out

After `0x10108210` succeeds, `0x1010808c..0x10108104` obtains the emitted
period from the new cursor and previous cursor-selection, requests the exact
window and multiplies the new grain in stored order:

```text
output[start + i] = sar15(output[start + i] * window[i])
```

M36 windows are stored descending, so this is a terminal fade-out. It is the
opposite orientation from the zero-flag cross transition, which reads the same
window backwards to form a fade-in.

## Portable transaction

`legacy_runtime_execute_terminal_m36()` composes the checkpoint, terminal
source selection, exact writer windows, one PCM write, post-write state
rotation and descending terminal fade. Output and runtime state commit only
after the complete sequence validates.

The subsequent original calls that finalize descriptor metadata/event state,
copy descriptor text and reset bookkeeping words remain outside this PCM
contract. They must be recovered before a route-level executor can claim full
descriptor completion.

No proprietary DLL or voice-data bytes are stored in the repository.
