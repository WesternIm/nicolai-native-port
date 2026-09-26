# M36 continuation — original phone-feature and runtime state recovery

Branch: `m36-original-phone-features`.
Base: merged M35 on `main` at `a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

Production synthesis remains unchanged. M36 now has a static portable
phone-feature builder, guarded original oracles, recovered rollback/state
primitives, exact guarded window-cache reconstruction, increasingly exact
source-transition geometry, and a reproducible one-click A/B measurement path.

The local original remains:

```text
mtsyc32.dll
SHA256 f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7
PE timestamp 0x412a0cb4
preferred image base 0x10000000
image size 0x797000

nicolai16.dat
SHA256 471bf1266c913784187dae25a2e5784a6ec309162e7dee2167ca9887c253e74d
```

Neither proprietary file belongs in Git.

## Remaining live evidence gates

Gate A, on Windows with the original installed DLL:

```powershell
.\tools\run_m36_original_probe.ps1
```

Expected direct-original result:

```json
{"portable_cases":259,"original_matches":259}
```

Gate B, on the installed original engine:

```powershell
.\tools\run_m36_runtime_capture.ps1
```

The resulting phone-record audit must contain nonzero records with zero invalid
and zero mismatched records. CI cannot execute these two proprietary-runtime
gates but does build the Win32 x86 tooling and run all portable contracts.

## One-click A/B measurement path

The Windows entry point is now:

```bat
run_ab_compare.bat
```

It delegates to `tools/run_ab_compare.ps1`, builds `nicolai_batch_render`,
derives the exact 22-phrase corpus from the PC reference `manifest.json`, renders
stable and candidate directories, and produces historical parity metrics,
phrase-by-phrase comparison CSV/JSON, M35 2048/1024 diagnostic reports and a
compact summary under a timestamped `metrics-work/ab-*` directory.

Current executable candidate profiles are `m34-unit` and `m34-shared`. The
`m36` profile is intentionally reserved and blocked until the route-level M36
audio path actually honors `NICOLAI_M36_TRANSITION_EXECUTOR`; this prevents an
unknown environment variable from silently producing a fake stable-vs-stable
comparison. See `M36_AB_RUNNER.md`.

## Static runtime state now recovered

The branch now has portable contracts for:

- route selection between dropped / cross / initial / ordinary / deferred-terminal paths;
- checkpoint and last-dropped rollback state;
- ordinary first-grain source selection including dropped-bridge ownership;
- ordinary repeated-grain Q11 period, WORD arithmetic, left/right window lengths
  and exact boundary-centred source starts;
- transactional ordinary writer composition and the complete zero-byte cross
  wrapper with its first-period fade-in;
- initial-transition first-grain source selection;
- initial-transition repeated grains both within one descriptor and while
  crossing to the next descriptor PCM base;
- nonzero cross-transition geometry and its distinct central SAR16 overlap
  arithmetic;
- exact nonzero cross-transition primary/secondary temporary PCM buffers,
  including reverse/forward window orientation and both shoulder cases;
- exact per-write cursor/selection rotation and a transactional nonzero cross
  executor composing both buffers with all four writer phases;
- exact deferred-terminal single-grain PCM write, checkpoint and descending
  fade-out around `0x10108210`;
- zero-branch cross fade;
- original packed window-cache topology and guarded lookup for lengths 1..400.

Relevant notes:

- `M36_RUNTIME_ROLLBACK.md`
- `M36_ORDINARY_GRAINS.md`
- `M36_INITIAL_TRANSITION.md`
- `M36_WINDOWS.md`
- `M36_AB_RUNNER.md`

## Why the M34 stateful renderer is still untouched

The current `StatefulTdsM34` still treats every grain as a simplified
current-interval front/tail selection and uses analytic half-Hann windows.
Static tracing now proves that this is not how the PC renderer works: first,
repeated, initial and cross-descriptor grains use different source ownership and
the nonzero cross mixer even uses a different final shift.

Do not partially promote only one recovered primitive. The next implementation
checkpoint is an opt-in M36 transition executor that composes the proven route,
source geometry, rollback and window contracts while preserving the M34 path as
an exact fallback for comparison.

## Next implementation checkpoint

1. Build a route-level opt-in M36 experiment beside `StatefulTdsM34`; do not
   replace it. Reuse the completed transactional executor for nonzero cross and
   expose it through the reserved `NICOLAI_M36_TRANSITION_EXECUTOR` switch.
2. Compose dropped rollback and initial transition with the completed ordinary,
   zero-cross, nonzero-cross and deferred-terminal PCM executors.
3. Recover the post-terminal descriptor metadata/event finalization after
   `0x10108210` and keep an explicit fallback until it is proven.
4. Run `run_ab_compare.bat -CandidateProfile m36` against the same 22 PC golden
   phrases; inspect headline parity plus both M35 diagnostic windows.
5. Require improvement without a global duration scale or phrase rules before
   considering any production promotion.

The practical first gate for that executor is to stop the experimental path
from being worse than the stable M31 baseline: stateful total-duration MAE must
move down from ~10.59% toward the stable ~3.60%, while normalized spectral shape
must improve from the current stateful ~39.50 toward or below baseline ~37.19.
