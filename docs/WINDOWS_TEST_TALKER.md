# Original PC voice and native port in one Windows executable

`NicolaiTalker.exe` is a test frontend, not an installer or a replacement SAPI
driver. It gives one text box, engine/profile selection, asynchronous synthesis,
playback/stop/replay, Save WAV and access to per-job logs. It is built as x86 so
the original installed 32-bit Nicolai token is visible, and runs on x86/x64
Windows. The static Release runtime avoids a separate VC++ runtime install.

M46 adds an [optional original linguistic capture](M46_ORIGINAL_LINGUISTIC_CAPTURE.md),
not a new acoustic profile. `Trace-Original.cmd` (when included) starts this same
EXE with `--original-trace`; normal launch and audio defaults remain unchanged.
M46a can wait for a lazy original DLL in the verified render itself, or capture
its fresh verified server child. Capture is not yet live-validated
on this host. It saves private records locally, never uploads them automatically.

Ten selections are exposed: stable native port (default), experimental M36
local, experimental M36 chain, experimental M38 connected-word timing,
experimental M40 transient repair, experimental M41 short-feature protection
and fractional fallback sampling, experimental M42 join-pitch reconciliation,
experimental M43 word-budget timing/limited dictionary-form stress,
experimental M44 original-lexicon stem-stress forms,
and the installed original Nicolai via SAPI5.
M41 keeps M40's coverage-edge repair, but does not enable the rejected internal
run-protection ablation. See [M41 evidence and limits](M41_SOUND_PRESERVATION.md).
The port modes use three external voice files directly and do not load Elan
DLLs. The original mode uses only a matching Nicolai/Elan Russian token; it
never silently substitutes another installed voice or falls back to the port.
It requires a working original voice installation, not just loose data files.

M42 inherits M41, uses local run contour coordinates for internal joins,
and reconciles screened shared-phone endpoint periods at strength 0.5. It
does not implement a shared phase clock or fix lexical stress. Evidence is
mixed, so stable remains the default. See [M42 evidence and limits](M42_JOIN_PERIOD_CONTINUITY.md).

M43 inherits M42, blends word-budget timing at 0.25 and enables limited
fixed-stem `-ика` dictionary-form stress. Old modes do not enable either
change. The isolated `акустика` already has correct lexical stress; the
missing inflected `акустику` is corrected, without claiming that acoustic
prominence on `с` is solved. See [M43 evidence and limits](M43_WORD_RHYTHM_STRESS.md).

M44 inherits M43 and reads original stem/suffix/stress resources directly from
EDAT. It accepts only the implemented unambiguous stem-stress subset, with exact
exceptions and explicit acute taking precedence. Ending-stress, full grammatical
selectors and phrase prosody remain open. See [M44 evidence and limits](M44_LEXICON_STRESS.md).

Every synthesis job is an owned child of the same EXE. User text travels in an
explicit UTF-8 file, not interpolated shell commands; paths use wide Win32 APIs
and quoted argv. Each port child sets both M36 flags explicitly, so inherited
experimental flags cannot leak into stable. The child uses the existing A/B
frontend, coefficients, output gain and terminal silence. Acoustic renderer
defaults remain unchanged; the shared frontend correction below applies to all
port callers. Cancellation/timeout kills only that owned child, never
an existing original server. The window remains responsive when SAPI stalls.
The opt-in M46 debugger may attach only to its verified same-EXE render parent
or that render's newly created server. An exceptional detach after breakpoint
writes can terminate only that owned target; the worker exits on render death.
pre-existing/shared servers are refused and left untouched.

Original SAPI is intermittent on this host. User-owned GUI jobs have produced
original WAVs, while later independent bounded runs stalled at
`original_sapi_stage=set-rate` before Speak. A fresh original job must succeed
before treating a new comparison as paired evidence. Port playback is
independent of this original installation issue.

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
runtime and PE x86 machine type. EXE, README and commit/hash metadata are
included, optionally with `Trace-Original.cmd` via `-IncludeOriginalTraceLauncher`:
no voice data, original DLLs, installer, WAVs or captured arrays.
The `windows-test-package` CI job uploads the tested ZIP as
`Nicolai-Test-win32` (30-day Actions artifact retention). Sources remain in Git.

For local reference-only verification (not available in CI):

```powershell
python tools/test_windows_talker.py --exe build-talker/Release/NicolaiTalker.exe `
  --voice C:\path\to\Elan --batch build-talker/Release/nicolai_batch_render.exe `
  --check-original
```

The GUI-job path synthesizes the actual startup phrase in each of the nine
port modes. Child-render WAVs for ten synthetic phrases (single word, simple,
startup, initial-vowel, initial-u, number/transient and plain/marked noun/verb
stress regressions) are byte-identical to the established batch renderer for
each port profile: ninety WAV comparisons. The optional `--baseline-exe`
compares eighty old-profile WAVs with the previous M43 EXE; all remain identical.
For M46, `--baseline-includes-m44` compares all ninety pairs with the existing
M44 EXE, as recorded in the M46 scalar report.
Stable differs from M36 local, M38 and M40; M41 differs from M40, M42 differs
from M41, M43 differs from M42 and M44 differs from M43,
so the profile selector is not comparing stable output against itself. This
is wrapper/profile parity, not new acoustic progress or a 22-phrase oracle.
Missing input, invalid profiles, Cyrillic/space-containing input/output paths,
inherited flag isolation and refusal to overwrite old WAVs are also checked.

Voice-free contracts inspect profile policies and GUI controls/Unicode input.
The M44 local Win32 Release CTest run passed 33/33; M46 adds two contracts and
the original fresh full run passed 35/35. M46a adds a lazy-load/stop/reject/cancel
contract (36 tests total); see the capture notes for current validation. The executable's imports
are Windows system DLLs only (no VCRUNTIME/MSVCP DLL requirement). A screenshot
rendered by the app's own UI smoke mode was inspected for clipped controls and
readability. The test frontend adds no networking or registry mutations.

## Startup phrase regression (2026-09-27)

The first package failed on `Привет! Это Николай. Проверяем голос и акустику.`
with `missing_diphone_#_a3`. The failure is in shared frontend selection, not
voice installation: initial unstressed а/о was treated like an interior remote
pretonic vowel. The local voice graph has `# -> a1`, but not `# -> a3`.

```powershell
nicolai_m10_probe.exe C:\path\to\Elan\nicolai16.dat "#" a1
# present=yes, exit 0
nicolai_m10_probe.exe C:\path\to\Elan\nicolai16.dat "#" a3
# present=no, exit 1
```

The shared frontend now selects initial reduced `a1`, preserving interior `a3`
(e.g. молоко), stressed vowels and disabled reduction. Generic explicit-stress
contracts cover акустику, аппарат, оборона, огород and атом without relying on
external dictionaries. They remain active in Release and report failures to
stderr with a normal nonzero exit instead of an interactive CRT abort dialog.
The GUI job regression uses its default edit-control text, not an easier
replacement phrase. Local x64 Debug 30/30 and x86 Release 31/31 passed after the
fix, as did the three real GUI jobs and nine CLI/batch WAV comparisons. All 22
stable WAVs from the existing UTF-8 A/B corpus remain byte-identical to the
pre-fix baseline. This fixes synthesis coverage; it does not establish closer
acoustic parity with the original. See the newer M37 note for the initial-u
regression, join instrumentation and paired gap audit.

## M37 acoustic triage (2026-09-28)

The shared frontend now selects a catalog-supported `# -> u1` for an initial
unstressed у instead of unavailable `# -> u4`. It removes the synthesis error
on the user's longer phrase without changing the 22 existing stable corpus
WAVs. The stable renderer reports final-PCM join coordinates and overlap/trim/
correlation diagnostics; these are read-only and do not change samples. The
GUI discards its current-WAV association when the engine is changed and uses
profile-specific suggested Save WAV names, preventing accidental mislabeled
A/B recordings. `--ui-job-test` covers this reset.

The analysis method, three small paired results, and explicit limits are in
[`M37_ACOUSTIC_TRIAGE.md`](M37_ACOUSTIC_TRIAGE.md). This checkpoint improves
coverage and diagnosis, **not yet audible acoustic parity**. Do not smooth or
remove every quiet interval: some are expected stop closures or word pauses.

## M38 opt-in connected-word trial (2026-09-28)

The M38 profile keeps the stable diphone renderer but redistributes each plain
internal word-boundary diphone's calibrated target from its `#` half to its
spoken half. Punctuation and utterance edges are untouched; the stable profile
remains byte-identical. It is an A/B candidate, not a proven replacement. On
10 multiword reference phrases, aligned excess quiet at word joins fell from
1971 to 1144 ms while duration MAE stayed about 59.7 ms. MFCC-DTW improved in
8/10 phrases, but two worsened, and one of three user short pairs also worsened.
See [`M38_BOUNDARY_TRIAL.md`](M38_BOUNDARY_TRIAL.md) for exact method, scalar
evidence and limitations. The pitch/intonation gap remains open.

## User data and feedback

Input, logs and result WAVs remain in fresh directories under
`%LOCALAPPDATA%\NicolaiNativePort\TestRuns`. History is not silently deleted.
The GUI locates Elan beside the EXE, one level above it, or in the conventional
Program Files (x86) install directory; Browse allows another location.
The source-controlled Russian `WINDOWS_TEST_TALKER.txt` is packaged as README.
The binary is unsigned: no instruction to disable Windows protection is given.
