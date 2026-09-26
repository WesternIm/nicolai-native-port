# M36 — original phone-feature recovery

Base: merged M35 on `main`, merge commit
`a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

M36 still does **not** promote a new renderer. Production synthesis and Android
behavior remain unchanged. This checkpoint recovers the original
`0x101a2780` phone-feature/coefficient builder, surrounding runtime primitives,
and now exposes a guarded local M36 A/B renderer profile for measurement.

## Local original recovered and identified

The original Acapela/Elan Nicolai installer was recovered locally and its
embedded MSI cabinet stream was extracted without committing proprietary bytes.
The exact original DLL used by earlier reverse work is present:

- `mtsyc32.dll` SHA256
  `f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`
- PE timestamp `0x412a0cb4`
- preferred image base `0x10000000`
- image size `0x797000`
- `nicolai16.dat` SHA256
  `471bf1266c913784187dae25a2e5784a6ec309162e7dee2167ca9887c253e74d`

These inputs remain local-only. Git contains only portable code, scalar facts,
probes and capture tooling.

## Exact static boundary of 0x101a2780

The original function begins at VA `0x101a2780` and the synthesis caller of
interest calls it at `0x1010ce79`, returning to `0x1010ce7e`. It has three
cdecl arguments:

1. phone feature record;
2. previous regulated descriptor;
3. next regulated descriptor.

The feature record fields observed by the function are:

```text
+0x00  int32 count
+0x04  signed WORD interval_duration[count - 1]
+0x18  signed WORD pitch_anchor[count]
```

The descriptor fields used by the function are:

```text
+0x0c  int32 node_count
+0x10  int32 split_index
+0x14  int32 source_position[node_count]
+0xfb4  WORD  voicing[]
+0x1784 WORD  duration_q11[]
+0x1f54 WORD  pitch_q11[]
```

The no-feature branch (`count <= 0`) writes unity `2048` into duration and
pitch lanes over previous-right and next-left. It does not modify voicing.

## Recovered positive feature path

The source support is exactly:

```text
(previous.last - previous.split) + (next.split - next.first)
```

Feature duration is the signed-WORD sum of `count - 1` interval durations.
The common duration coefficient is:

```text
duration_q11 = (feature_duration_sum << 11) / combined_source_support
```

with x86 integer truncation and a zero-result fallback to `1`.

Each feature interval is converted back into source coordinate with:

```text
feature_width = (feature_interval_duration << 11) / duration_q11
```

The builder walks previous-right first and then next-left, carrying the feature
coordinate across the descriptor boundary. Assignment uses the original `+3`
tolerance.

## Pitch-anchor repair and pitch lane

Before using a pair of pitch anchors, the original repairs one missing endpoint
in place:

```text
if left == 0 and right != 0: left = right
if left != 0 and right == 0: right = left
```

If both anchors are zero, or the descriptor voicing WORD for that source
interval is zero, pitch Q11 is forced to `2048`. Otherwise the original calls
`0x101a2c00` and performs the observed 32-bit multiply/logical shift:

```text
reciprocal = reciprocal_pitch(left_anchor, feature_width,
                              right_anchor, feature_position)
pitch_q11  = low32(reciprocal * source_interval_width) >> 17
```

The integer truncation is observable: a constant 80-sample anchor and an
80-sample source interval produces `2047`, not idealized `2048`.

The final next-left interval has a special branch when it is also on the final
feature interval. Its interpolation position is:

```text
feature_width - source_interval_width
```

instead of the normal running coordinate.

## Portable reconstruction and runtime primitives

`legacy_phone_features_m36()` models the statically recovered positive runtime
domain. The branch also contains portable contracts for:

- runtime route priority and dropped rollback;
- ordinary first/repeated grain source geometry;
- initial-transition first/repeated grain geometry;
- deferred-terminal single-grain/fade behavior;
- zero/nonzero cross-transition geometry and transactional PCM executors;
- original packed window-cache topology and guarded lookup for lengths 1..400;
- exact post-write cursor/selection rotation.

None of those primitives is production-selected by default.

## Runnable local M36 A/B profile

`NICOLAI_M36_TRANSITION_EXECUTOR=1` now switches the established stateful
batch-render caller into `resynthesize_stateful_m36_experimental()`.
`run_ab_compare.bat -CandidateProfile m36` therefore produces a real stable-vs-
M36 candidate corpus instead of a blocked or fake stable-vs-stable run.

The first runnable profile intentionally integrates only behavior whose portable
source ownership exists inside one diphone PCM slice:

- recovered initial path;
- recovered ordinary path;
- recovered deferred-terminal path;
- exact M36 window lookup/source selection;
- existing shared-phone duration/Q11 step policy.

The nonzero cross-descriptor executor is **not** wired into this first A/B.
Its local arithmetic is recovered, but caller step-buffer ownership and source
context outside one portable diphone slice are not yet proven. In particular,
we do not invent `deferred terminal == cross buffered step` just to make the
branch run. The A/B result is therefore a directional local-transition
experiment, not a complete-PC-renderer claim.

`stateful_tds_m36_test` locks that this path is executable, differs from the old
M34 analytic/front-tail adapter, hits recovered initial/terminal logic on a
synthetic descriptor, and rejects unsupported phone-local energy instead of
silently changing the experiment.

## Synthetic original oracle

`nicolai_m36_phone_probe` builds a deterministic 259-case corpus. On Win32 x86,
when the known original DLL is supplied, it calls original RVA `0x1a2780` and
compares repaired anchors plus descriptor duration/pitch/voicing fields.

One-command local run on a machine where Nicolai is already installed:

```powershell
.\tools\run_m36_original_probe.ps1
```

Promotion gate:

```json
{"portable_cases":259,"original_matches":259}
```

CI intentionally has no proprietary DLL. It configures a real Win32 x86 build,
compiles the same probe and runs all portable cases.

## Real-runtime capture path

`tools/nicolai_m36_runtime_capture.cpp` attaches to the installed original
engine with the Windows Debug API, validates remote `mtsyc32.dll`, captures real
`0x101a2780` entry/return structures and writes runtime JSONL. The canonical
22-phrase trigger plus `audit_phone_features_m36.py` form Gate B.

Expected use:

```powershell
.\tools\run_m36_runtime_capture.ps1
```

## Current boundary / next evidence

Still not proven/promoted:

- the required local 259/259 direct-original result on the user's Windows host;
- successful real 22-phrase Gate B capture with zero audit mismatches;
- upstream producer parity for feature records / voicing;
- caller step-buffer ownership and extended source context needed to compose
  nonzero cross transitions in the portable chain;
- post-terminal descriptor metadata/event finalization;
- exact x87 last-bit window oracle;
- any production or Android promotion.

The next practical action is to run the same 22-phrase A/B bundle with the now
executable local M36 profile. Use the result to decide whether recovered local
grain/window behavior moves timing and normalized shape in the right direction
before adding cross-descriptor caller state.