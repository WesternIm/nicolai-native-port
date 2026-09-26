# M36 continuation — original phone-feature and runtime state recovery

Branch: `m36-original-phone-features`.
Base: merged M35 on `main` at `a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

Production synthesis remains unchanged. M36 now has a static portable
phone-feature builder, guarded original oracles, recovered rollback/state
primitives, exact guarded window-cache reconstruction, source-transition
geometry, and a runnable **local** M36 A/B renderer profile.

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

The Windows entry point is:

```bat
run_ab_compare.bat -CandidateProfile m36
```

It delegates to `tools/run_ab_compare.ps1`, builds `nicolai_batch_render`,
derives the exact 22-phrase corpus from the PC reference `manifest.json`, renders
stable and candidate directories, and produces historical parity metrics,
phrase-by-phrase comparison CSV/JSON, M35 2048/1024 diagnostic reports and a
compact summary under a timestamped `metrics-work/ab-*` directory.

`nicolai_batch_render` now honors `NICOLAI_M36_TRANSITION_EXECUTOR=1`. The
switch enables the established stateful/shared-phone caller and the adapter
dispatches into the runnable M36 local transition experiment rather than M34.

## Current runnable M36 scope

The first executable M36 profile intentionally integrates only behavior whose
source ownership is available inside one portable diphone PCM slice:

- initial-transition first/repeated grain geometry;
- ordinary first/repeated grain source selection;
- deferred-terminal single-grain/fade behavior;
- exact M36 window-cache lookup;
- original Q15 writer plus recovered post-write rotation;
- existing shared-phone duration/Q11 step policy.

The recovered nonzero cross-descriptor executor is **not** called yet. Static
recovery proves its local PCM math, but the current portable diphone slice ends
at its final source node while cross entry can require caller-owned source
context beyond that slice. The exact caller step-buffer ownership is also still
outside proof. Do not invent `deferred terminal == cross buffered step` merely
to make the branch execute.

The runnable profile therefore isolates local M36 grain/window/terminal effects
for directional A/B correction. Its score is not a claim of a complete PC
renderer.

## Static runtime state now recovered

The branch has portable contracts for:

- route selection between dropped / cross / initial / ordinary / deferred-terminal paths;
- checkpoint and last-dropped rollback state;
- ordinary first-grain source selection including dropped-bridge ownership;
- ordinary repeated-grain Q11 period, WORD arithmetic, left/right window lengths
  and exact boundary-centred source starts;
- transactional ordinary writer composition and the zero-byte cross wrapper;
- initial-transition first/repeated grain source geometry;
- nonzero cross-transition geometry, buffers, four writer phases and SAR16
  overlap arithmetic;
- exact deferred-terminal single-grain write and descending fade-out;
- original packed window-cache topology and guarded lookup for lengths 1..400.

Relevant notes:

- `M36_RUNTIME_ROLLBACK.md`
- `M36_ORDINARY_GRAINS.md`
- `M36_INITIAL_TRANSITION.md`
- `M36_WINDOWS.md`
- `M36_AB_RUNNER.md`

## Next checkpoint

1. Run `run_ab_compare.bat -CandidateProfile m36` on the same 22 PC golden
   phrases.
2. Compare historical duration/waveform/MFCC/RMS metrics and M35 normalized
   shape/energy diagnostics phrase by phrase.
3. If local M36 moves in the right direction, recover/capture caller
   step-buffer/source ownership needed by nonzero cross transitions.
4. Integrate cross only after that ownership is proven; keep M34 fallback intact.
5. Re-run the exact same A/B bundle before considering any production promotion.

The useful first gate is directional improvement relative to old stateful
(~10.59% total-duration MAE and ~39.50 normalized shape) toward the stable
(~3.60% / ~37.19) reference. No global duration scale or phrase-specific rule
is allowed as a substitute for missing runtime state.