# M46 — optional original linguistic capture, not an acoustic change

M45 identified an unconnected original phrase-boundary route in the port.
M46 provides a bounded way to collect its real inputs from the user's working
original-SAPI selection, without a separate debugger EXE or original-DLL edits.
It is an experimental diagnostic: **no complete real-text capture has yet been
obtained on the development host**. Synthetic contracts do not establish that
the real server exposes every assumed record at these sites.

## What is established

The user confirmed using original SAPI in the existing M44 test EXE. Local
TestRuns independently contain successful `enumerate-voices`, `select-voice`,
`set-rate`, `speak`, `complete` sequences with non-empty WAVs. Two inspected
successful outputs were 41,208 bytes; a third was 51,260 bytes. Those private
logs and recordings are not committed. A separate fresh GUI startup probe still
stalled at `set-rate`, before Speak, with an empty WAV; its owned render/UI
processes were cleaned up. Direct owned server startup loaded the original
module and dependencies, then exited normally. This does not identify a cause
and does not contradict the user's successful runs. No licensing, installation
or registry intervention was performed.

## One-EXE use

Build the x86 Release `NicolaiTalker` with static MSVC runtime, then optionally
package `Trace-Original.cmd` using `-IncludeOriginalTraceLauncher`. It merely
starts the same EXE with `--original-trace`. The normal EXE remains a ten-profile
player; the trace window has a distinct M46 title and starts on original SAPI.

For each short, non-personal test phrase, click Speak then Open result folder.
A successful trace adds these PRIVATE files to the existing per-job directory
under `%LOCALAPPDATA%\NicolaiNativePort\TestRuns`:

- `linguistics-m46.jsonl`: bounded raw intermediate snapshots, not PCM.
- `linguistics-m46-worker.log`: READY and capture count/error information.
- `linguistics-m46.stop`: stop handshake, not a result to interpret.

The existing `input.txt`, `render.log` and `result.wav` remain there too.
Nothing is uploaded automatically. Never commit these text/record dumps or WAVs.
If capture fails, retain the logs and use the normal EXE for listening.

## Capture sites and representation

Each site is an inspected instruction boundary in the pinned original DLL;
EBP holds the same linguistic state. RVA addresses relocate with its module base.

| Stage | Original VA | Observation |
| --- | --- | --- |
| before_split | 0x101a17ed | Before raw separator producer/splitter 0x10216c70 |
| after_split | 0x101a17f3 | Before context markers 0x10216970 |
| after_markers | 0x101a17ff | After fallback markers, before phonetic records 0x1019e350 |
| after_authoring | 0x101a1811 | After physical authoring 0x10214f40, before runtime prosody 0x10212df0 |

The state reader captures one-based words, 20-byte morphology candidate records,
punctuation, raw separator bytes and four-byte separator slots. Annotated words
and punctuation are hex-encoded CP866, avoiding locale-dependent conversion.
The last stage also captures each word's 32-byte source-phone records. Pitch,
energy and duration bytes retain their original raw representations: this
checkpoint does **not** guess floating/integer units or assert a PCM clock.
It does not capture grammatical selector tables at state +0x3a34 and beyond.

Limits: 1–256 words; at most 70 candidates per word; null-terminated word strings
within 4096 bytes, punctuation within 12 bytes and codes within four bytes;
at most 128 phones per word and 4096 phones per call. A failed read emits no
partial JSON row and increments the worker's skipped count. Worker success
requires complete four-stage calls in order, no skipped reads, no pending call
and exactly four rows per completed call. The separate audit also checks the
record shape and split model; worker exit alone is not parity evidence.

## Process isolation and cancellation

The trace refuses any pre-existing ettsengine server. After normal SAPI setup,
it selects either a direct server child of the render or the render itself.
The latter requires target PID = parent PID, the worker's actual direct parent,
an identical executable path and identical process creation times. It never
substitutes a differently parented COM server. Target/parent handles are retained
and process creation times checked against PID reuse. The worker is launched
hidden from the same EXE. SAPI output binding/Speak proceeds after worker READY.
If the original module is not loaded, READY says `hooks=waiting-for-module`:
this means **attached and waiting**, not a successful capture. The owned render's
`LOAD_DLL` event allows guarded hooks to be installed before its threads resume.
Only `M46_HOOKS_READY pinned_original=1` establishes that the hooks are armed.
A SHA256 check of the loaded DLL's file, PE identity checks and the four
`push ebp` instruction-byte guards precede any write. Supported original SHA256:

`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`

Only temporary in-memory INT3 bytes in this owned target are written; no disk
binary, voice file or registry entry is changed. The existing single-step route
debugger restores breakpoints during a debugger stop before detaching. It watches
the retained render-parent handle for GUI cancellation, imposes a 90-second
capture deadline, and bounds stop-file draining to three seconds. Parent-side
READY/detach waits are seven seconds. Exceptional detach or an unresponsive
worker can terminate **only the verified owned target**, to avoid leaving live
breakpoints behind. A rejected image before any write leaves the target alive
and detached. Module unloading fails closed instead of restoring to unmapped
addresses. Shared/existing servers are rejected, not shut down.

This may reject a working original that is hosted in a shared or differently
parented server. That is an explicit diagnostic limitation, not a reason to
change the user's installation. Live breakpoint/cancellation behavior against
the proprietary original remains unverified until a successful capture exists.

## Reproduction and scope

```powershell
cmake -S . -B build-m46 -A Win32 -DNICOLAI_STATIC_RUNTIME=ON -DBUILD_TESTING=ON
cmake --build build-m46 --config Release --parallel
ctest --test-dir build-m46 -C Release --output-on-failure
python tools/audit_m46_linguistics.py --self-test
./tools/package_windows_talker.ps1 -BuildDir build-m46 -IncludeOriginalTraceLauncher
```

For NMake, start in an x86 developer shell, omit `-A Win32`, configure
`-DCMAKE_BUILD_TYPE=Release` and use executables at the build root.

After a real trace is available, validate its shape and compare captured split
codes with the independent M45 scan model:

```powershell
python tools/audit_m46_linguistics.py --input C:\private\linguistics-m46.jsonl `
  --report C:\private\fresh-scalar-report.json
```

Only scalar counts are printed/written. Raw separator production, complete
morphology and acoustic parity are outside that audit. A mismatch remains a
reported mismatch, never silently reclassified as a pass.

Validation in this checkpoint: x86 Release full CTest 35/35; synthetic snapshot
fields/atomicity plus six negative guards; an owned synthetic child exercises
attach, remote break with the actual M46 process-access mask, and detach while
leaving the fixture alive (then cleaning up that test child); audit three positive, six negative
and one mismatch-accounting cases. Local wrapper PCM isolation is recorded in
[the scalar report](metrics/m46-wrapper-isolation-20261001-fresh.json): nine real
GUI startup jobs, 90 EXE/batch PCM pairs and 90 unchanged pairs against the user's
previous M44 EXE, including M44 itself. CI cannot
perform real original-SAPI capture without proprietary inputs and a working
installation.

The final local package is built in a fresh `build_m46_talker` directory. The
older localized MSVC/NMake dependency cache had tracked only `.cpp` files and
missed changed `.inc` includes. CMake now declares those dependencies explicitly
for the embedded/standalone debugger, snapshot tests and talker controller.
This prevents a diagnostic include edit from silently shipping stale code;
fresh-build tests and PCM isolation are repeated on the final executable.
The remote-stop mechanism uses the Windows
[DebugBreakProcess interface](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-debugbreakprocess).
This synthetic lifecycle check still does not prove proprietary-server
breakpoint restoration or real linguistic capture.
The fixture exposed premature closure of debug-event process/thread handles:
detach intermittently failed with Win32 error 5. The shared route debugger and
fixture now use the same image-file cleanup helper, leaving process/thread
handles to the Windows debug-event lifecycle. After that correction, twenty
consecutive fixture detach runs pass. See
[ContinueDebugEvent lifecycle](https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-continuedebugevent).

## M46a correction (2026-10-02)

The user's M46 error after `set-rate` came from demanding an owned server before
SAPI had activated its actual engine. A read-only host probe showed no core DLL
or server at that point; the SAPI wrapper appeared only during later output
setup. This is a capture-order defect, not evidence that the user's working
original selection is broken. M46a adds the guarded lazy-render path above,
and `bind-output` / `set-output` stages to distinguish those calls from Speak.

The synthetic lifecycle test uses a real same-EXE worker/parent attach, Unicode
paths, graceful stop without a module, SHA256 rejection and a real `LOAD_DLL`
event for a deliberately untrusted synthetic DLL. It also verifies that killing
an owned synthetic render ends its worker. No original code/data is in this
fixture. Normal port/GUI PCM isolation is rerun against the preserved M44 EXE.
The DLL event's file handle is used only for a wide, normalized path, not the
optional remote image-name pointer. Windows' documented
[LOAD_DLL event](https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-load_dll_debug_info)
and [attach lifecycle](https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-debugactiveprocess)
define the stopped-thread and handle contracts. If the event cannot identify
the supported module, the worker never arms guessed addresses.

A bounded local original probe now reaches READY waiting for the core; its
render loads `sapi.dll` and `ettsengines5.dll`, but not `mtsyc32.dll`. It still
stalls before Speak with zero WAV bytes and zero records. Cancelling only that
retained render also ends the worker. **Actual complete original captures remain
zero**; in-process hosting of the working original remains unproven. Do not
interpret a waiting READY or an empty JSONL as progress on acoustic parity.

M46a validation: fresh Win32 static Release CTest **36/36**; the new lifecycle
contract passes **20 consecutive repeats**, including parent cancellation.
Audit self-tests remain 3 positive, 6 negative and 1 mismatch-accounting case.
The final EXE hash and zero-capture result are recorded in
[the M46a scalar contract report](metrics/m46a-capture-contracts-20261002.json).
All nine profiles are checked against batch and the preserved M44 executable
in [the fresh PCM isolation report](metrics/m46a-wrapper-isolation-20261002-fresh.json).

Next: obtain four complete stages from the working original GUI path; reconcile
actual morphology/markers/raw boundaries; only then implement and oracle-test
general rules for port spans and phone authoring. Stable, all nine port synthesis
profiles and the user's older M44 package remain unchanged. M46 is not an
intonation fix, a new acoustic profile or a claim of original parity.
