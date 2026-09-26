# M36 continuation — original phone-feature builder

Branch: `m36-original-phone-features`.
Base: merged M35 on `main` at `a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

## Current checkpoint

M36 now has two layers:

1. a proven scalar duration helper from earlier tracing;
2. a broader static reconstruction of original `0x101a2780`, including the
   recovered feature-record layout, pitch-anchor repair, descriptor lane walk,
   voicing-forced unity, `+3` boundary tolerance and terminal next-left pitch
   position rule.

Production synthesis remains unchanged. The broader reconstruction is not yet
promoted because it still needs a direct original-DLL runtime oracle pass.

The exact local original has been recovered from the installer and identified
as:

```text
mtsyc32.dll
SHA256 f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7
PE timestamp 0x412a0cb4
image base 0x10000000
image size 0x797000
```

The local voice DB extracted alongside it has SHA256
`471bf1266c913784187dae25a2e5784a6ec309162e7dee2167ca9887c253e74d`.
Neither file belongs in Git.

## Next work, in order

1. Run the new probe in a **Win32 x86** developer shell:

```powershell
cmake -S . -B build_m36_win32 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m36_win32 --target nicolai_m36_phone_probe
.\build_m36_win32\nicolai_m36_phone_probe.exe C:\path\to\mtsyc32.dll
```

The promotion gate for the static reconstruction is exactly:

```json
{"portable_cases":259,"original_matches":259}
```

Any mismatch means fix the portable control flow/arithmetic first; do not tune
corpus audio to hide it.

2. After the synthetic oracle passes, capture **real runtime feature records**
   from the original PC text path around the call at `0x1010ce79`. Record only
   scalar fields needed for analysis. Keep raw proprietary memory local.

3. Extend `audit_phone_features_m36.py` from scalar duration evidence to a
   versioned record format containing:

```text
feature interval durations
repaired pitch anchors
previous/next source positions
voicing WORDs
observed duration Q11 lane
observed pitch Q11 lane
```

Validate real no-feature, multi-anchor, missing-anchor and terminal cases
against the already oracle-checked builder.

4. Only after real coefficient-lane parity, wire an **opt-in** path from those
   authentic lanes into `StatefulTdsM34`. Do not replace the stable production
   renderer yet. Rerun the same 22 PC golden phrases and the M35 clock audit.

The first timing gate after substitution is to bring stateful target+flush
MAE down from the M35 shared value (~10.68%) toward or below the stable baseline
(~3.60%) without a global correction factor.

5. Once authentic coefficient lanes are no longer the timing blocker, move to
   the original state oracle around `0x10107c20` / `0x10107f65`:

- dropped-node selection;
- last-node rollback;
- saved positions `+0x54/+0x58/+0x5c`;
- cursor/clock restoration;
- boundary/final-flush conditions.

6. Then recover source transitions `0x101086c0` / `0x10108cf0` and exact window
   lookup/construction `0x10109be0` / `0x1010a020`.

## Validation expected at every checkpoint

- all host C++ tests pass on Windows and Ubuntu;
- `nicolai_m36_phone_probe_contract` passes without proprietary data;
- M35 and M36 Python contracts remain green;
- original-DLL probes are rerun locally when applicable;
- stable production WAV SHA identity remains unchanged until explicit
  promotion;
- no phrase-specific rule or global duration correction is accepted as a
  substitute for unresolved original behavior.
