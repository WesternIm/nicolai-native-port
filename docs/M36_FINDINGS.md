# M36 — original phone-feature recovery

Base: merged M35 on `main`, merge commit
`a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

M36 still does **not** promote a new renderer. Production synthesis, Android
behavior, stateful source selection, drop/rewind and window construction remain
unchanged. This checkpoint recovers and instruments the original `0x101a2780`
phone-feature/coefficient builder.

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

## Portable reconstruction

`legacy_phone_features_m36()` models the statically recovered positive runtime
domain. It preserves the independent duration primitive, repairs anchors,
carries coordinate state across descriptors, reads voicing as an input gate,
writes only owned duration/pitch lanes and reproduces visible x86 WORD/32-bit
integer behavior. Malformed portable inputs are rejected instead of emulating
original divide faults or unbounded memory access.

The implementation is linked into `nicolai_port` but remains unused by
`Engine` and `StatefulTdsM34`.

## Synthetic original oracle

`legacy_phone_features_test` covers no-feature ownership, duration Q11,
2047 reciprocal truncation, missing-anchor repair, voicing-forced unity,
unequal source widths, boundary carry and terminal behavior.

`nicolai_m36_phone_probe` builds a deterministic 259-case corpus. On Win32 x86,
when the known original DLL is supplied, it calls original RVA `0x1a2780` and
compares repaired anchors plus descriptor duration/pitch/voicing fields.

One-command local run:

```powershell
.\tools\run_m36_original_probe.ps1 -Dll C:\path\to\mtsyc32.dll
```

Promotion gate:

```json
{"portable_cases":259,"original_matches":259}
```

CI intentionally has no proprietary DLL. It nevertheless configures a real
Win32 x86 build, compiles the same probe and runs all 259 portable cases. The
recorded no-DLL output is `{"portable_cases":259,"original_matches":0}`.

Until the local 259/259 DLL run is recorded, the expanded builder remains a
**static reconstruction**, not a proven exact port.

## Real-runtime capture path

M19 established that the installed original voice synthesizes through 32-bit
SAPI5 and the out-of-process `ettsengine.exe`. M36 now adds a non-injected
runtime debugger:

`tools/nicolai_m36_runtime_capture.cpp`

It attaches to the original engine with the Windows Debug API, validates the
remote `mtsyc32.dll` PE identifiers, and uses software breakpoints at:

```text
builder entry  module + 0x1a2780
known return   module + 0x10ce7e
```

At entry it snapshots the three real arguments. At return it snapshots repaired
feature anchors and previous/next descriptor lanes. Other threads are manually
suspended during the single instruction used to restore/reinsert each software
breakpoint, reducing the usual breakpoint race. The debugger then writes one
`nicolai-m36-runtime-record-v1` JSON object per completed call. No code is
injected into `ettsengine`.

`tools/m36_sapi_trigger.ps1` reuses the canonical 22-phrase corpus through
32-bit SAPI5. `tools/run_m36_runtime_capture.ps1` orchestrates warm-up, process
selection, Win32 build, debugger attach, the 22-phrase run, clean detach and
post-capture audit.

Expected use from the repository root on the installed PC voice:

```powershell
.\tools\run_m36_runtime_capture.ps1
```

The result stays under ignored `metrics-work/m36/runtime-capture/`, including
`phone-records.jsonl` and `phone-records-audit.json`.

## Independent runtime audit

`tools/audit_phone_features_m36.py` still accepts the original scalar duration
fixtures, and now also accepts runtime JSONL. Its runtime mode independently
replays the recovered x86 arithmetic/control flow and compares:

- repaired declared pitch anchors;
- previous/next source positions and voicing preservation;
- duration Q11 lanes;
- pitch Q11 lanes;
- descriptor counts/splits.

This gives an implementation independent of the C++ builder. CI contracts cover
both the `2047` integer fingerprint and a missing-anchor repair case.

## Still outside this checkpoint

M36 does not yet prove or implement:

- the required local 259/259 original-DLL result;
- a successful real 22-phrase runtime capture from the installed engine;
- the upstream producer that creates feature records and descriptor voicing;
- stateful drop/last-node rollback around `0x10107f65`;
- buffered source selection in `0x10107c20`;
- source-transition paths `0x101086c0` / `0x10108cf0`;
- exact window lookup/construction `0x10109be0` / `0x1010a020`;
- promotion of recovered lanes into production synthesis.

The next evidence gate is original runtime data, not audio tuning.
