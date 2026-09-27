# M36 transactional nonzero cross executor

This note records the opt-in portable composition of original
`mtsyc32.dll` nonzero cross path `0x10108cf0`. Production synthesis and
`StatefulTdsM34` remain unchanged.

## Recovered post-write state rotation

Every successful `0x10109980` call in the four cross writer phases is followed
by the same state transform. It appears at `0x10109485..0x101094a7`,
`0x1010958b..0x101095a5`, `0x10109681..0x1010969b` and
`0x10109786..0x101097a0`:

```text
old_cursor = state.cursor
state.selection_b = state.selection_a
state.selection_a = old_cursor
state.cursor = old_cursor + period
```

The cursor addition is a 32-bit x86 add. Portable
`legacy_runtime_post_write_m36()` preserves that rotation and wrap behavior.

The immediately following helper `0x10109fc0` checks the two temporary window
objects and frees backing pointers that fall outside the shared cache range. It
does not rotate PCM/runtime state. Portable windows own their vectors, so this
resource-lifetime helper has no executor-side equivalent.

## Executor composition

`legacy_runtime_execute_cross_m36()` composes the already-separated contracts:

1. nonzero cross geometry;
2. exact primary and secondary temporary PCM buffers;
3. buffered entry writer plan;
4. every buffered repeated-grain plan;
5. current entry writer plan;
6. every current repeated-grain plan;
7. exact recovered window lookup;
8. positive-domain `0x10109980` PCM writer;
9. the post-write state rotation above.

The two step records use the existing `LegacyTdsStepM33` representation. Their
`count`, `first_period` and `delta_q11` fields drive the same phase loops as the
original. Boundaries remain explicit absolute coordinates in their respective
PCM vectors, so descriptor identity cannot be silently lost.

`legacy_runtime_execute_cross_route_m36()` adds the caller transaction around
that executor: it binds the previous/current descriptor source context, stages
the four PCM phases, then applies `legacy_runtime_bookkeep_m36()` to synchronize
both step slots and interval markers. Output, runtime state and step buffers are
committed together only after both layers succeed.

## Transaction boundary

Before writing, the executor validates:

- both step records and positive counts;
- temporary-buffer materialization;
- every phase plan and period;
- every raw/temp source slice;
- every exact window request;
- final cursor range.

PCM output and `LegacyRuntimeStateM36` are staged in copies. They are committed
only after all phases succeed. A late failure therefore exposes neither a
partial waveform nor partial cursor/selection rotation.

## Deliberate integration boundary

This closes the portable nonzero cross PCM/state plus caller transaction, but it
still has no production caller. A full route-level M36 experiment needs to
combine dropped rollback, initial, ordinary, zero-cross and this nonzero-cross
route with the completed deferred-terminal PCM path. Live descriptor capture,
post-terminal metadata/event finalization and acoustic promotion remain
explicit boundaries.

The `m36-chain` profile now provides a bounded experimental caller with explicit
previous/current PCM and deferred-step ownership. Its route/flush policy is not
the full original caller: see `M36_CHAIN_ACOUSTICS.md` for the measured limits.

No proprietary DLL or voice-data bytes are stored in the repository.
