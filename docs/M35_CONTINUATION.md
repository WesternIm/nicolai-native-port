# M35 recovery and M36 continuation

Repository: https://github.com/WesternIm/nicolai-native-port (private).
Checkout `m35-voicing-clock-audit` for this checkpoint. M35 is stacked on
`m34-stateful-phone-regulator`, commit
`a88a2a25b2fb57ffcb2b722e6ef29e4b9ceb0076`; do not discard the M34 source
when recovering from the PR. Once merged, the equivalent main commit is fine.
Nothing in portable build/test requires conversation history or proprietary
data. This checkpoint does not promote stateful synthesis or physical [l]/[e].

## Local-only inputs

Place legally available `nicolai16.dat`, `exc_rus.txt`, `abb_rus.txt` in
`voice-data/`. Supply the same PC `reference_pack_20260924_222948` directory,
including 001–022 WAVs and manifest. Check raw input hashes against
`docs/metrics/m35-run-manifest.json`. Voice data and golden WAVs are intentionally
not recoverable from GitHub. Do not commit them or derived feature caches.
The corpus text is already tracked in tools/parity_corpus_22.tsv.

## Portable tests and voice-free metric contracts

From an x64 Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build_m35 -G "NMake Makefiles" -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m35 --clean-first
ctest --test-dir build_m35 --output-on-failure
python -m pip install -r tools/requirements-parity.txt
python tools/test_m35_metrics.py
```

Expected: 20 C++ tests, 6 separate Python contracts. Multi-config builds need
`--config Debug`, `ctest -C Debug` and the executable directory as BuildDir.
CI runs portable tests on Windows/Ubuntu and voice-free metrics on Ubuntu.
Corpus metrics additionally require local inputs; CI does not validate them.
Recorded numeric environment: Python 3.12.14, NumPy 2.5.3, librosa 1.0.0,
SciPy 1.18.1, numba 0.67.0. Requirements give supported minimums; use recorded
versions when reproducing scalar artifacts exactly.

## Full six-setting audit

Use a fresh shell without inherited NICOLAI_* tuning variables. Supply absolute
input/build/Python paths where convenient:

```powershell
.\tools\run_m35_audit.ps1 -BuildDir .\build_m35 -VoiceDataDir .\voice-data `
  -ReferencePack C:\path\to\reference_pack_20260924_222948 `
  -OutputRoot .\metrics-work\m35 -Python C:\path\to\python.exe
```

Optional `-PriorM34Root C:\path\to\m34-final` verifies SHA256 identity for all
six prior settings. Optional `-MetricsOnly` reuses already rendered M35 outputs
with CLOCK/TDS diagnostics; it still runs the six Python contracts, clock
audit, both pitch-window analyses and input/code manifest. A M34 log without
CLOCK records is not sufficient for the new clock audit.

The wrapper calls the unchanged M34 sweep, with 22 renders for each of baseline,
stateful-unit, stateful-shared, and shared plus physical length/energy/both at
0.002. It retains historical parity JSON/CSV and adds `clock-audit.json`,
`parity-v2-1024.json`, `parity-v2-2048.json`, `run-manifest.json`.
All raw audio and features stay in ignored metrics-work/. No metric score
alters synthesis or authorizes an experimental promotion automatically.

For direct calls see `tools/audit_clock_m35.py --help` and
`tools/measure_parity_v2.py --help`. Versioned definitions are in their JSON
reports. Read coverage/null phrases together with conditional F0 scores.

## Original-DLL probes

The exact DLL SHA256 is in M35_FINDINGS.md; verify it **before loading**.
Use an x86 Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build_m35_win32 -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_m35_win32 --target nicolai_m34_probe nicolai_m33_probe
.\build_m35_win32\nicolai_m34_probe.exe "C:\Program Files (x86)\Elan\mtsyc32.dll"
.\build_m35_win32\nicolai_m33_probe.exe .\voice-data\nicolai16.dat `
  "C:\Program Files (x86)\Elan\mtsyc32.dll"
```

Expected: 1024 writer, 400 reciprocal, 1000 sequential step matches; 2665
source timelines and 683 isolated step matches. Do not interpret an x64
probe's zero original-match count as a DLL oracle pass. Logs in docs/metrics
contain scalar results, not DLL content.

## M36: recover targets, not a global time correction

1. Capture **actual original phone feature records and coefficient lanes**
   around `0x101a2780`. Use source support previous-right + next-left,
   duration at descriptor +0x1784, pitch +0x1f54, voicing +0xfb4. Existing
   portable phone hints are not feature ground truth. Prove units, positive
   feature-duration sum, pitch-anchor repair, interval ownership and terminal
   next-left handling on a guarded synthetic harness before substituting lanes.
2. Obtain original pitch/voicing labels or independently annotated speech to
   validate the estimator. Pure tones and self-identity are not enough. Keep
   both window sensitivities and coverage; never optimize admission parameters
   to make the preferred synthesis path win. Use common validated frames when
   comparing F0, with missing/false-positive voicing still penalized separately.
3. Recover runtime drop/last-node rollback around `0x10107f65` and buffered
   source selection in `0x10107c20`, including saved positions +0x54/+0x58/+0x5c.
   Fresh static tracing shows a conditional rewind to saved +0x54 when the
   dropped interval is node_count−2 and the relevant boundary/final-flush flags
   permit it: cursor +0x48, clocks +0x3c/+0x40 and selections are restored.
   Do not bolt a rewind onto the current adapter without an original state
   oracle. M35's small local writer residual does not validate this behavior.
4. Source transitions `0x101086c0` / `0x10108cf0` and exact window construction
   `0x1010a020` / `0x10109be0` are still missing. Preserve proven Q15 writer,
   Q11 step and three-point contour; no normalized-OLA substitution or phrase
   rules to hide unresolved parity.
5. Repeat 20 tests, voice-free metric contracts, original probes, the same 22
   WAVs in every selected setting, historical and diagnostic metrics, and
   fallback SHA checks. Publish the next checkpoint in its own GitHub branch/PR
   with scalar artifacts and exact limitations. No promotion on sparse F0 or
   waveform correlation alone; seek stable timing/shape/energy improvements.

Current actionable evidence: target+flush shared duration MAE 10.6806% versus
actual 10.5941%; baseline 3.5987%. The writer-minus-target average is 2.47 ms.
That points the next supported implementation effort upstream while leaving
the stable renderer untouched.
