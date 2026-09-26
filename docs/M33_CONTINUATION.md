# M33 continuation and reproduction

Recover the repository from GitHub; no chat-only code or proprietary binary is
needed for the portable build and 20 host tests. Evaluation additionally needs
the local three voice files and the same 22-WAV PC golden pack, as documented
in `M32_CONTINUATION.md`. Neither voice data nor reference/rendered WAVs belong
in a commit.

## Build and evaluate

In a Visual Studio x64 Developer Command Prompt/PowerShell:

```powershell
cmake -S . -B build_m33 -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m33 --clean-first
ctest --test-dir build_m33 --output-on-failure
python -m pip install -r tools/requirements-parity.txt
.\tools\sweep_m33_execution.ps1 -BuildDir .\build_m33 `
  -VoiceDataDir .\voice-data -ReferencePack C:\path\to\reference_pack_20260924_222948 `
  -OutputRoot .\metrics-work\m33-execution -Python C:\path\to\python.exe
```

For multi-config generators use the executable directory (e.g. build_m33/Debug)
as BuildDir and `--config Debug` / `ctest -C Debug`. The sweep has seven settings:
M32 baseline, PC timeline, no phase search, both execution changes, and timeline
with physical length/energy/both at 0.002. It writes 22 WAVs per setting and the
unchanged M32 metric definitions. Comparison:

```powershell
python tools/compare_parity.py metrics-work/m33-execution/baseline/parity.json `
  metrics-work/m33-execution/pc-timeline/parity.json `
  --json metrics-work/m33-execution/timeline-delta.json `
  --csv metrics-work/m33-execution/timeline-delta.csv
python tools/assess_m33_candidates.py metrics-work/m33-execution `
  --json metrics-work/m33-execution/gate.json
python tools/write_parity_manifest.py voice-data C:\path\to\reference_pack_20260924_222948 `
  --json metrics-work/m33-execution/run-manifest.json
```

To verify the stable fallback, render M32 from its merge commit `a4501dc` into
a separate local directory, then compare it to M33's baseline:

```powershell
python tools/verify_corpus_identity.py C:\path\to\m32-baseline `
  metrics-work/m33-execution/baseline --json metrics-work/m33-execution/identity.json
```

The committed run manifest records source/input SHA256 and the exact local
Python dependency versions. For numerical reproduction use Python 3.12.14,
NumPy 2.5.3, librosa 1.0.0, SciPy 1.18.1, and numba 0.67.0; newer dependency
versions can change metric values without changing rendered audio.

## Verify exact primitives against the original DLL

First check DLL SHA256 against `M33_FINDINGS.md`. This runs original executable
code; only use the known original local installation, never an arbitrary DLL.
In an **x86** Visual Studio Developer Command Prompt:

```powershell
cmake -S . -B build_m33_win32 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m33_win32 --target nicolai_m33_probe
.\build_m33_win32\nicolai_m33_probe.exe .\voice-data\nicolai16.dat `
  "C:\Program Files (x86)\Elan\mtsyc32.dll"
```

Expected: 2665 valid timelines, zero rejected, 2665 original timeline matches,
683 step cases and 683 original step matches. The 64-bit portable probe accepts
the database alone and validates the entire timeline without loading the DLL.
Optional read-only disassembly uses `tools/disassemble_legacy.py` and packages
in `tools/requirements-reverse.txt`; generated disassembly remains local.

## Next implementation boundary (M34)

Do not promote the isolated timeline adapter merely because its source lattice
is exact. Port the stateful half-phone regulator around `0x1010d330` and its
source-node selection first. Connect its Q11 coefficient lanes to the tested
`legacy_tds_step_m33`, retaining the +8 carry across the correct boundaries.
Then port the windowed left/right writes at `0x10109980` and callers rather
than resetting phase and correlation-trimming independently rendered buffers.
Keep M32's three-point F0 and phone-local [l]/[e] interface. Re-run the same
metrics and only enable a candidate after a multi-metric, per-phrase audit.

The PCM/A-law source timeline is verified. Compressed coding 0x50 has additional
block/offset handling in the original function and is **not** claimed here.
