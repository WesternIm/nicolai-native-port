# Original PC voice and native port in one Windows executable

`NicolaiTalker.exe` is a test frontend, not an installer or a replacement SAPI
driver. It gives one text box, engine/profile selection, asynchronous synthesis,
playback/stop/replay, Save WAV and access to per-job logs. It is built as x86 so
the original installed 32-bit Nicolai token is visible, and runs on x86/x64
Windows. The static Release runtime avoids a separate VC++ runtime install.

Four selections are exposed: stable native port (default), experimental M36
local, experimental M36 chain, and the installed original Nicolai via SAPI5.
The port modes use three external voice files directly and do not load Elan
DLLs. The original mode uses only a matching Nicolai/Elan Russian token; it
never silently substitutes another installed voice or falls back to the port.
It requires a working original voice installation, not just loose data files.

Every synthesis job is an owned child of the same EXE. User text travels in an
explicit UTF-8 file, not interpolated shell commands; paths use wide Win32 APIs
and quoted argv. Each port child sets both M36 flags explicitly, so inherited
experimental flags cannot leak into stable. The child uses the existing A/B
frontend, coefficients, output gain and terminal silence without changing the
production renderer. Cancellation/timeout kills only that owned child, never
an existing original server. The window remains responsive when SAPI stalls.

The original mode is implemented, but successful original audio is NOT proven
on this host: a bounded real run enumerated/selected Nicolai then timed out at
`original_sapi_stage=set-rate` before Speak, reproducing the earlier startup
problem. Do not call this an original-versus-port PCM parity result. Port
playback is independent of that original installation failure.

## Build and package

```powershell
cmake -S . -B build-talker -A Win32 -DNICOLAI_STATIC_RUNTIME=ON -DBUILD_TESTING=ON
cmake --build build-talker --config Release --parallel
ctest --test-dir build-talker -C Release --output-on-failure
python tools/test_windows_talker.py --exe build-talker/Release/NicolaiTalker.exe
.\tools\package_windows_talker.ps1 -BuildDir build-talker
python tools/test_windows_talker.py --exe build-talker/Release/NicolaiTalker.exe `
  --zip out/Nicolai-Test-win32.zip
```

Existing NMake builds use a matching x86 developer environment without `-A`.
Package output paths must be fresh. The package script checks Release, static
runtime and PE x86 machine type. Only EXE, README and commit/hash metadata are
included: no voice data, original DLLs, installer, WAVs or captured arrays.
The `windows-test-package` CI job uploads the tested ZIP as
`Nicolai-Test-win32` (30-day Actions artifact retention). Sources remain in Git.

For local reference-only verification (not available in CI):

```powershell
python tools/test_windows_talker.py --exe build-talker/Release/NicolaiTalker.exe `
  --voice C:\path\to\Elan --batch build-talker/Release/nicolai_batch_render.exe `
  --check-original
```

The GUI-job and child-render paths produce real speech in each of the three
port modes; the resulting WAV for the synthetic phrase is byte-identical to
the established batch renderer for that profile. Stable differs from M36 local,
so the profile selector is not comparing stable output against itself. This
is wrapper/profile parity, not new acoustic progress or a 22-phrase oracle.
Missing input, invalid profiles, Cyrillic/space-containing input/output paths,
inherited flag isolation and refusal to overwrite old WAVs are also checked.

Voice-free contracts inspect profile policies and GUI controls/Unicode input.
The full local Win32 Release CTest run passed 31/31. The executable's imports
are Windows system DLLs only (no VCRUNTIME/MSVCP DLL requirement). A screenshot
rendered by the app's own UI smoke mode was inspected for clipped controls and
readability. The test frontend adds no networking or registry mutations.

## User data and feedback

Input, logs and result WAVs remain in fresh directories under
`%LOCALAPPDATA%\NicolaiNativePort\TestRuns`. History is not silently deleted.
The GUI locates Elan beside the EXE, one level above it, or in the conventional
Program Files (x86) install directory; Browse allows another location.
The source-controlled Russian `WINDOWS_TEST_TALKER.txt` is packaged as README.
The binary is unsigned: no instruction to disable Windows protection is given.
