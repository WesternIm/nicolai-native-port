# M36 continuation — original phone-feature builder

Branch: `m36-original-phone-features`.
Base: merged M35 on `main` at `a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

Production synthesis remains unchanged. M36 now has a static portable builder,
a guarded direct-DLL synthetic oracle, a real Win32 runtime capture debugger and
an independent Python replay audit.

The exact local original recovered from the installer is:

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

## Gate A — direct synthetic original oracle

Run on Windows from the repository root:

```powershell
.\tools\run_m36_original_probe.ps1 -Dll C:\path\to\mtsyc32.dll
```

The gate is exactly:

```json
{"portable_cases":259,"original_matches":259}
```

CI proves the same executable builds and runs as Win32 x86, but cannot supply
the proprietary DLL; its expected no-DLL result is 259 portable cases and zero
original matches.

Any original mismatch means fix the builder first. Do not tune audio around it.

## Gate B — real installed-engine records

M36 no longer needs manual WinDbg memory capture. With the original Nicolai
installed and its Acapela engine usable through 32-bit SAPI5, run:

```powershell
.\tools\run_m36_runtime_capture.ps1
```

The wrapper:

1. warms up Nicolai through 32-bit SAPI5;
2. locates the active `ettsengine.exe`;
3. builds `nicolai_m36_runtime_capture` for Win32 x86;
4. attaches via the Windows Debug API;
5. records pre/post `0x101a2780` structures while the canonical 22 phrases are
   synthesized;
6. cleanly restores breakpoints and detaches;
7. runs `audit_phone_features_m36.py` over the resulting JSONL.

Expected output directory:

```text
metrics-work/m36/runtime-capture/
  phone-records.jsonl
  phone-records-audit.json
  capture.stdout.log
  capture.stderr.log
  trigger-wavs/
```

The runtime audit must report zero mismatched and zero invalid records before any
coefficient lane is connected to the portable stateful renderer.

If original SAPI itself fails during warm-up, resolve the installed voice/server
first; that is distinct from a builder parity failure.

## Gate C — opt-in authentic-lane timing experiment

Only after Gate A and Gate B pass:

1. add an opt-in path that feeds the recovered original duration/pitch ownership
   rules into `StatefulTdsM34`;
2. keep stable production untouched;
3. rerun the same 22 PC golden phrases and all M35 clock diagnostics;
4. compare target+flush and actual total duration separately.

The first timing target is to move stateful target+flush MAE from the M35 shared
~10.68% toward or below the stable ~3.60% baseline without a global scale or
phrase-specific rule.

## After coefficient timing is no longer the blocker

Recover the original execution state oracle around `0x10107c20` /
`0x10107f65`:

- dropped-node selection;
- last-node rollback;
- saved positions `+0x54/+0x58/+0x5c`;
- cursor/clock restoration;
- boundary/final-flush conditions.

Then recover source transitions `0x101086c0` / `0x10108cf0` and exact window
lookup/construction `0x10109be0` / `0x1010a020`.

## Validation expected at every checkpoint

- all host C++ tests pass on Windows and Ubuntu;
- Win32 x86 probe/capture tools compile in CI;
- `nicolai_m36_phone_probe_contract` passes without proprietary data;
- M35/M36 Python contracts remain green;
- original-DLL and installed-engine evidence is kept local as scalar/JSONL
  artifacts only;
- stable production WAV output remains unchanged until explicit promotion;
- no phrase-specific rule or global duration correction substitutes for missing
  original behavior.
