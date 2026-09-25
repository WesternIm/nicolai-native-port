# M32 continuation point

M32 is complete as an execution-architecture milestone. Phone-local duration
and energy are implemented and tested, the full PC corpus selection pass is
committed, and production correctly retains zero strengths because no non-zero
candidate passed the multi-metric gate.

## Local inputs for reproduction (never commit)

Place these files in one directory, for example `voice-data/`:

```text
nicolai16.dat   10,961,460 bytes
exc_rus.txt      2,960,349 bytes
abb_rus.txt          3,636 bytes
```

If only the original Acapela/ELAN Nicolai 5.1 MSI is available, extract it
without executing installer custom actions:

```powershell
python tools/extract_msi.py path\to\nicolai.msi work\msi-extracted
```

## Build and test on Windows

Run from a Visual Studio Developer PowerShell or Developer Command Prompt:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected result: 20/20 tests.

## Reproduce the M32 sweeps

Install the parity-only dependencies from `tools/requirements-parity.txt`, then
run a grid. The selected Python must contain NumPy and librosa.

```powershell
.\tools\sweep_m32_physical.ps1 `
  -BuildDir .\build `
  -VoiceDataDir .\voice-data `
  -ReferencePack C:\path\to\reference_pack_20260924_222948 `
  -OutputRoot .\metrics-work\m32-micro-sweep `
  -Python .\.venv\Scripts\python.exe `
  -Strengths @(0.0005,0.001,0.002,0.005,0.01)
```

Use `-Modes length` for a length-only grid. Compare a candidate phrase by
phrase with:

```powershell
python tools\compare_parity.py `
  metrics-work\m32-length-fine\baseline\parity.json `
  metrics-work\m32-length-fine\length-0.0005\parity.json `
  --json metrics-work\comparison.json `
  --csv metrics-work\comparison.csv
```

Candidate directories contain WAVs and logs and remain ignored. Commit only
summary/per-phrase JSON and CSV artifacts that contain no proprietary data.

## Recommended M33 direction

Do not tune M32 strengths further around the isolated `0.0005` alignment
spike. The larger remaining parity limit is the synthesis/join path: recover
the Windows pitch-mark, phase, and half-diphone join semantics, then re-evaluate
the already implemented phone-local `[l]`/`[e]` authoring values without adding
phrase-specific rules.
