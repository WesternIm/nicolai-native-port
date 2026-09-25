# M32 continuation point

The implementation and tests are complete; the remaining work is the local
golden-corpus selection pass.

## Required local inputs (never commit)

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

The convenience copies are written beneath `work\msi-extracted\flat`; use the
directory containing the three required files as `VoiceDataDir`.

## Build and test on Windows

Run from a Visual Studio Developer PowerShell or Developer Command Prompt:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected result: 20/20 tests.

## Run the M32 grid

Install the parity-only Python dependencies from
`tools/requirements-parity.txt`, then run:

```powershell
.\tools\sweep_m32_physical.ps1 `
  -BuildDir .\build `
  -VoiceDataDir .\voice-data `
  -ReferencePack C:\path\to\reference_pack_20260924_222948 `
  -OutputRoot .\metrics-work\m32-sweep `
  -Python C:\path\to\python.exe
```

The final comparison is written to `sweep-summary.csv` and
`sweep-summary.json`; every candidate directory also contains its 22 rendered
WAVs plus per-phrase CSV/JSON. These output directories are ignored by Git.

## Acceptance rule

Do not select a non-zero production strength for a fifth-decimal correlation
movement alone. Prefer a candidate that improves waveform correlation and at
least two independent error metrics (timing, F0, MFCC, RMS) without a material
regression in any remaining metric. Inspect per-phrase rows for outliers before
changing the defaults in `LegacyTimingPolicy`.

After selection, commit only the metric JSON/CSV summaries and update
`M32_PC_PARITY_REPORT.md`; keep golden/portable WAVs and proprietary voice data
outside Git.
