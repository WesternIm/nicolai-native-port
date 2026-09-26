# M36 — original phone-feature recovery

Base: merged M35 on `main`, merge commit
`a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

M36 still does **not** promote a new renderer. Production synthesis, Android
behavior, stateful source selection, drop/rewind and window construction remain
unchanged. This checkpoint moves the original `0x101a2780` phone-feature /
coefficient builder from a duration-only hypothesis toward an executable,
original-oracle-testable reconstruction.

## Local original recovered and identified

The original Acapela/Elan Nicolai installer was recovered locally and its
embedded MSI cabinet stream was extracted without committing any proprietary
bytes. The exact original DLL used by earlier reverse work is present:

- `mtsyc32.dll` SHA256
  `f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`
- PE timestamp `0x412a0cb4`
- image base `0x10000000`
- image size `0x797000`
- `nicolai16.dat` SHA256
  `471bf1266c913784187dae25a2e5784a6ec309162e7dee2167ca9887c253e74d`

These inputs remain local-only. The repository contains only scalar facts,
portable code and a guarded probe.

## Exact static boundary of 0x101a2780

The original function begins at VA `0x101a2780` and is called at
`0x1010ce79`. The call has three arguments:

1. phone feature record;
2. previous regulated descriptor;
3. next regulated descriptor.

The caller constructs the first argument inside a per-phone working record and
the callee accesses it as:

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
pitch lanes over previous-right and next-left. It does not write unity into the
voicing flags.

## Recovered positive feature path

The source support is exactly:

```text
(previous.last - previous.split) + (next.split - next.first)
```

Feature duration is the signed-WORD sum of the `count - 1` interval durations.
The common duration coefficient is:

```text
duration_q11 = (feature_duration_sum << 11) / combined_source_support
```

with integer truncation and a zero-result fallback to `1`.

The builder then converts each feature interval duration back into source
coordinate using:

```text
feature_width = (feature_interval_duration << 11) / duration_q11
```

and walks source intervals first over previous-right and then next-left. The
coordinate comparison uses the original `+3` tolerance before assigning an
interval to the current feature interval.

## Pitch-anchor repair and pitch lane

Before using a pair of pitch anchors, the original repairs one missing endpoint
in place:

```text
if left == 0 and right != 0: left = right
if left != 0 and right == 0: right = left
```

If both feature anchors are zero, or the descriptor voicing WORD for that
source interval is zero, pitch Q11 is forced to unity `2048`.

Otherwise the original calls the already recovered reciprocal interpolation at
`0x101a2c00`, then performs 32-bit multiply and logical shift:

```text
reciprocal = reciprocal_pitch(left_anchor, feature_width,
                              right_anchor, feature_position)
pitch_q11  = low32(reciprocal * source_interval_width) >> 17
```

The early integer reciprocal truncation is observable. For example, a constant
80-sample anchor and 80-sample source interval produces `2047`, not idealized
`2048`.

The final next-left interval has a special branch when it is also the final
feature interval. Its interpolation position is:

```text
feature_width - source_interval_width
```

instead of the normal running coordinate. This is the terminal next-left rule
that M35 left unresolved.

## Portable reconstruction

`legacy_phone_features_m36()` now models the statically recovered positive
runtime domain. It:

- preserves the previous duration-only helper as an independent primitive;
- mutates missing pitch anchors in place;
- carries feature-coordinate state across the previous/next descriptor split;
- preserves descriptor voicing;
- writes only the owned previous-right and next-left duration/pitch lanes;
- reproduces x86 WORD truncation, 32-bit multiply wrap and logical `>> 17`
  where those operations are visible in the original;
- rejects malformed portable inputs instead of emulating divide faults or
  unbounded original memory access.

The implementation is linked into `nicolai_port` but remains unused by
`Engine` and `StatefulTdsM34`.

## Tests and guarded original oracle

`legacy_phone_features_test` now covers:

- no-feature unity ownership;
- duration Q11 construction;
- integer reciprocal truncation;
- missing-anchor repair;
- voicing-forced unity;
- unequal source widths, feature-boundary carry and the terminal path;
- malformed portable records.

`nicolai_m36_phone_probe` builds a deterministic 259-case synthetic corpus. On
all hosts it exercises the portable builder. On a **Win32 x86** build, when the
known original DLL is supplied, it loads only the validated image and calls
RVA `0x1a2780`, then compares repaired pitch anchors plus all relevant voicing,
duration and pitch lane WORDs.

Local Win32 use:

```powershell
cmake -S . -B build_m36_win32 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m36_win32 --target nicolai_m36_phone_probe
.\build_m36_win32\nicolai_m36_phone_probe.exe C:\path\to\mtsyc32.dll
```

The desired oracle result is:

```json
{"portable_cases":259,"original_matches":259}
```

Until that exact original-DLL run is recorded, the expanded builder is a
**static reconstruction**, not yet a proven exact port.

The no-DLL portable corpus is also registered in CTest so Windows/Linux CI
compiles and executes the same 259 supported cases without proprietary data.

## Existing scalar audit

`tools/audit_phone_features_m36.py` remains useful for runtime/corpus captures.
It independently checks source support and duration Q11 from JSON scalar
records. Raw original memory, the DLL, disassembly and voice data remain
untracked.

## Still outside this checkpoint

M36 does not yet prove or implement:

- runtime-produced real phone feature records from full PC text synthesis;
- whether all real feature records stay inside the guarded portable domain;
- stateful drop/last-node rollback around `0x10107f65`;
- buffered source selection in `0x10107c20`;
- source-transition paths `0x101086c0` / `0x10108cf0`;
- exact window lookup/construction `0x10109be0` / `0x1010a020`;
- promotion of the recovered coefficient lanes into production synthesis.

The next gate is the 259/259 original-DLL synthetic oracle, followed by capture
of real original feature records and only then an opt-in stateful substitution.
