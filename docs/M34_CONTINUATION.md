# M34 reproduction and M35 continuation

Clone `https://github.com/WesternIm/nicolai-native-port.git` and select the
M34 PR branch `m34-stateful-phone-regulator` (or main after it is merged).
Portable code and all 20 tests require no proprietary binaries or chat files.
Audio evaluation additionally needs the three legally available local inputs
`nicolai16.dat`, `exc_rus.txt`, `abb_rus.txt` and the same PC pack
`reference_pack_20260924_222948` with WAVs 001–022 and its manifest.
Use the fingerprints in `docs/metrics/m34-run-manifest.json` to establish
identity. Neither the pack nor voice data is recoverable from this repository.

## Build and evaluate

Run in a fresh Visual Studio x64 Developer PowerShell, without inherited
`NICOLAI_*` tuning variables:

```powershell
cmake -S . -B build_m34 -G "NMake Makefiles" -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m34 --clean-first
ctest --test-dir build_m34 --output-on-failure
python -m pip install -r tools/requirements-parity.txt
.\tools\sweep_m34_execution.ps1 -BuildDir .\build_m34 -VoiceDataDir .\voice-data `
  -ReferencePack C:\path\to\reference_pack_20260924_222948 `
  -OutputRoot .\metrics-work\m34 -Python C:\path\to\python.exe
python tools/assess_m33_candidates.py metrics-work/m34 --json metrics-work/m34/gate.json
python tools/compare_parity.py metrics-work/m34/baseline/parity.json `
  metrics-work/m34/stateful-shared/parity.json `
  --json metrics-work/m34/shared-delta.json --csv metrics-work/m34/shared-delta.csv
python tools/write_parity_manifest.py voice-data C:\path\to\reference_pack_20260924_222948 `
  --json metrics-work/m34/run-manifest.json
python tools/audit_parity_metrics.py C:\path\to\reference_pack_20260924_222948 `
  metrics-work/m34 --json metrics-work/m34/metric-audit.json
```

Multi-config builds need `--config Debug`, `ctest -C Debug` and the executable
directory (e.g. build_m34/Debug) as BuildDir. Use Python 3.12.14, NumPy 2.5.3,
librosa 1.0.0, SciPy 1.18.1, numba 0.67.0 for numerical reproduction.
Metric definitions are unchanged from M32. The strict M33 gate is reused,
not relaxed to accept the new experiments.

Six settings: baseline; stateful per-unit duration; stateful shared-phone
duration; shared plus physical length, energy or both at 0.002. Each renders
22 WAVs. The wrappers restore their changed environment variables on exit.
Batch render logs include `TDS id intervals grains dropped final_carry`
when stateful synthesis is selected. Direct diagnostic controls:

```text
NICOLAI_STATEFUL_TDS=1
NICOLAI_SHARED_PHONE_DURATION=1
```

Both are 0 by default; physical length/energy strengths remain 0. No runtime
DLL loading is needed or used by the portable synthesis library.

Verify stable fallback against locally rendered M33 baseline:

```powershell
python tools/verify_corpus_identity.py C:\path\to\m33-baseline `
  metrics-work/m34/baseline --json metrics-work/m34/identity.json
```

## Original-DLL oracle

Check the exact DLL SHA256 in M34_FINDINGS.md first. Use an **x86** Visual
Studio Developer PowerShell:

```powershell
cmake -S . -B build_m34_win32 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m34_win32 --target nicolai_m34_probe nicolai_m33_probe
.\build_m34_win32\nicolai_m34_probe.exe "C:\Program Files (x86)\Elan\mtsyc32.dll"
.\build_m34_win32\nicolai_m33_probe.exe .\voice-data\nicolai16.dat `
  "C:\Program Files (x86)\Elan\mtsyc32.dll"
```

Expected M34: 1024 writer, 400 reciprocal and 1000 sequential step matches,
no mismatches. Expected M33: 2665 timeline and 683 step matches. An x64 M34
probe without arguments exercises the supported domains without loading a DLL;
its original-match counts are zero, **not an oracle validation**.

## Next boundary: M35

Do not retune global duration or add phrase rules to hide the adapter's clock
error. Preserve stable output while replacing missing PC behavior:

1. Port `0x101a2780` into a testable shared-phone feature/coefficient builder.
   Duration is +0x1784, pitch +0x1f54, voicing +0xfb4. Capture supported 2/3-point
   feature records from PC; prove anchor units and terminal interval handling.
2. Build a narrow guarded Win32 harness for state `0x10107c20`: two buffered
   step records at +0x74/+0x78, selection flags/indices +0x24..+0x2e, saved
   output positions +0x54/+0x58/+0x5c and carry +0x7c. Audit drop/last-node
   rollback around 0x10107f65 before changing the portable clock.
3. Recover 0x101086c0 / 0x10108cf0 source selection across descriptor boundaries,
   and exact window construction/lookup 0x1010a020 / 0x10109be0. Reuse the
   verified writer; replacing it with normalized OLA would lose proven parity.
4. Re-run 20 tests, exact oracle probes, the same 22 WAV metrics and SHA256
   stable-fallback checks. Promote only an objective multi-metric improvement
   after a per-phrase audit; this corpus is not held-out generalization evidence.

The metric audit also belongs in the next step: historical F0 ranking reverses
when fmax changes from 150 to 400 Hz, whole-WAV RMS includes silence, and MFCC
includes level effects. Preserve historical reports while adding independently
validated pitch/voicing, aligned energy and level-normalized spectral checks.

The earlier M33 suggestion that 0x1010d330 itself is the regulator is superseded
by the full tracing documented in M34_FINDINGS.md.
