# M36 caller-owned step/marker bookkeeping (corrected 2026-09-27)

This note records the portable contract recovered from the caller loop around
original `mtsyc32.dll` function `0x10107c20`. It is a caller-state checkpoint,
not production-renderer integration: `StatefulTdsM34` and the stable renderer
remain unchanged.

## Step-buffer ownership

The interval loop builds a five-WORD record on its local stack, then calls one
of the PCM paths. The first WORD is the interval index; the remaining four are
the existing M33 step fields (`count`, `first_period`, `delta_q11`, `carry`).
An ordinary or deferred positive path swaps +0x74/+0x78 once and writes the
new +0x74, retaining old +0x74 at +0x78. No drop writes either slot.

The cross and initial branches perform an early slot swap/copy before joining
the common `0x10107ece` swap/copy block. ONLY these two routes end with both
slots containing the current record. The previous version incorrectly extended
this to ordinary/deferred paths and to started drops.

`LegacyRuntimeStepBuffersM36` models the two records without exposing pointers
or proprietary object layout. Its `valid` bit means both portable slots have
received a positive +0x74 record; it does not imply both slots are equal.

## Interval-marker rotation

The common non-terminal positive tail at `0x10107f19` applies:

```text
word24 <- word26
word28 <- word2a
word2a <- interval_index
word26 <- 0
```

Initial/cross ALSO perform this rotation early (`0x10107d9b` / `0x10107e38`).
A non-terminal initial/cross therefore finishes with `24=0,28=i,2a=i,26=0`.
For any positive terminal step the common tail instead saves the marker pair
AFTER any early rotation:

```text
word2c <- word26
word2e <- word2a
```

Thus terminal initial/cross save `2c=0,2e=i`. No drop reaches the positive
marker tail. Pending-cross drops do NOTHING (`0x10107f65` jumps directly to
loop advancement). Other drops only perform `word26 <- 1` after optional rewind.
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

The existing Win32 phone-feature oracle was rerun after the earlier checkpoint:
`portable_cases=259`, `original_matches=259`. The scalar result is recorded in
`docs/metrics/m36-caller-bookkeeping-probe-20260927.json`; the reference DLL
and its dependent files remain local under `Elan` and are not committed.
That oracle tests `0x101a2780`, NOT this caller loop: it did not validate the
earlier incorrect bookkeeping. This corrected contract has static-dataflow
and synthetic test evidence, not a live original PCM oracle.

## Next boundary

The new opt-in chain experiment uses descriptor-owned PCM and the positive
deferred +0x74 record. Cross phase 1 starts at prev[k], so real previous PCM
does not need an invented extended tail. Original route/flush gates, rollback
integration and live PCM comparison still block promotion. See
`M36_CHAIN_ACOUSTICS.md`.
