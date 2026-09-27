# M36 caller-owned step/marker bookkeeping

This note records the portable contract recovered from the caller loop around
original `mtsyc32.dll` function `0x10107c20`. It is a caller-state checkpoint,
not production-renderer integration: `StatefulTdsM34` and the stable renderer
remain unchanged.

## Step-buffer ownership

The interval loop builds a five-WORD record on its local stack, then calls one
of the PCM paths. The first WORD is the interval index; the remaining four are
the existing M33 step fields (`count`, `first_period`, `delta_q11`, `carry`).
After a successful positive path, the caller writes that complete record into
both state step slots at `+0x74` and `+0x78`. The same two-slot update also
happens for a dropped interval once a positive write has already started.

The cross and initial branches perform an early slot swap/copy before joining
the common `0x10107ece` swap/copy block. The final observable state is the same:
both slots contain the current interval record. A dropped step before the first
positive write, or while a descriptor crossing is pending, does not touch the
slots.

`LegacyRuntimeStepBuffersM36` models the two records without exposing pointers
or proprietary object layout. Its `valid` bit means both portable slots have
been synchronized by the contract.

## Interval-marker rotation

For every successful non-terminal write, including positive initial/cross paths
and a started dropped interval, the caller applies:

```text
word24 <- word26
word28 <- word2a
word2a <- interval_index
word26 <- 0
```

For a positive deferred-terminal step, or for a started dropped terminal step,
the caller instead preserves the current quartet and saves the previous marker
pair:

```text
word2c <- word26
word2e <- word2a
```

The pre-start/pending-cross dropped branch only performs `word26 <- 1`.
Rollback of cursor/clocks and its `word2c/word2e/field30` cleanup remain the
separate `legacy_runtime_drop_rollback_m36()` contract.

## Portable boundary

`legacy_runtime_bookkeep_m36()` stages state and both step slots and commits
only after route and interval validation. It consumes the existing
`legacy_runtime_route_m36()` priority, so it does not guess whether a caller
gate means “cross” or “started”. The booleans are explicit inputs until a live
capture proves their higher-level ownership.

`tests/legacy_runtime_state_test.cpp` covers ordinary, initial, cross,
deferred-terminal, started-dropped, pre-start dropped, pending-cross dropped,
and invalid transactional cases. The helper is linked into `nicolai_port` but
is not called by production synthesis or the current A/B adapter.

The existing Win32 phone-feature oracle was rerun after this checkpoint:
`portable_cases=259`, `original_matches=259`. The scalar result is recorded in
`docs/metrics/m36-caller-bookkeeping-probe-20260927.json`; the reference DLL
and its dependent files remain local under `Elan` and are not committed.

## Next boundary

The remaining route work is to bind these proven marker/step transitions to the
extended previous/current PCM descriptor ownership needed by the nonzero
cross-descriptor executor. Until that source context is captured, promotion
would still be speculative even though this caller bookkeeping is now exact.
