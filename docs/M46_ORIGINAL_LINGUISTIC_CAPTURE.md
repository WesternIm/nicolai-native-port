# M46 — optional original linguistic capture, not an acoustic change

M45 identified an unconnected original phrase-boundary route in the port.
M46 provides a bounded way to collect its real inputs from the user's working
original-SAPI selection, without a separate debugger EXE or original-DLL edits.
M46b now obtains **complete real-text captures on the development host**.
Thirty normal/trace original PCM pairs are equal. This is still a diagnostic,
not an acoustic change or complete original NLP parity. Earlier M46/M46a
observations below are historical; the M46b section records the corrected host
route, current evidence, reproducibility and remaining integration gaps.

## Earlier M46 observations (superseded by M46b)

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

The state reader captures one-based words, historical 20-byte candidate windows,
punctuation, raw separator bytes and four-byte separator slots. Annotated words
and punctuation are hex-encoded CP866, avoiding locale-dependent conversion.
The last stage also captures each word's 32-byte source-phone records. Pitch,
energy and duration bytes retain their original raw representations: this
checkpoint does **not** guess floating/integer units or assert a PCM clock.
It does not capture grammatical selector tables at state +0x3a34 and beyond.

M51 corrects a four-byte layout offset: legacy `candidates_hex` remains an
unchanged window starting at block+20*c. Actual payloads start at block+20*c+4,
and are now added separately as `candidate_payloads_hex` with an explicit
layout tag. The legacy leading DWORD is the preceding record ID, not a
context score. Old captures/audits remain readable. Opt-in seven-stage
analysis capture has its own schema/audit; see [M51](M51_ANALYSIS_SELECTION.md).

Limits: 0–256 words; at most 70 candidates per word; null-terminated word strings
within 4096 bytes, punctuation within 12 bytes and codes within four bytes;
at most 128 phones per word and 4096 phones per call. A failed read emits no
partial JSON row and increments the worker's skipped count. Worker success
requires complete four-stage calls in order, no skipped reads, no pending call
and exactly four rows per completed call. The separate audit also checks the
record shape and split model; worker exit alone is not parity evidence.
Real zero-word service calls emit four ordered empty-word states without
dereferencing unused arrays. The audit counts them separately, not as spoken
linguistic calls or nontrivial scan-model matches. Negative counts remain errors.

## Process isolation and cancellation

The trace refuses any pre-existing ettsengine server. M46b performs normal SAPI
output binding, then selects only its actual direct server child, before Speak
submits any text. Target/parent handles are retained and process creation times
and direct ownership checked before attaching. The worker is launched hidden
from the same EXE. Speak proceeds only after worker READY with `hooks=ready`.
The earlier lazy same-EXE render path remains an isolated debugger contract,
but is no longer the talker's target-selection fallback: the supported S5 core
is out of process. `M46_HOOKS_READY pinned_original=1` establishes armed hooks.
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
the proprietary original is now verified for normal capture/detach; abrupt
cancellation additionally has the synthetic owner-lifetime regression below.

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

Initial M46 validation: x86 Release full CTest 35/35; synthetic snapshot
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

## Historical M46a correction (2026-10-02)

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

## M46b: working out-of-process oracle (2026-10-02)

The user confirmed normal M46a original SAPI speech. A fresh successful GUI job
produced a 136,412-byte WAV. A controlled hidden Windows-managed launch produced
the same WAV hash. Read-only stacks and inspected S5 SDK code showed that
`SetOutput` starts `ettsengine.exe /nogui /autoexit` and waits on the SDK ready
event. The actual `mtsyc32.dll` belongs to that server, not the test render.
Attaching to the render before output binding cannot observe the linguistic
route. M46b binds output first, verifies its direct server child, arms the
pinned hooks, then submits text. No activation/helper interface is invoked.

The inherited background launch can exit its server with code 0 before ready,
leaving the old SDK's unbounded event wait behind. Explicit Windows-managed
launches work reproducibly. The exact inherited attribute causing that legacy
exit is **not established**. The diagnostic runner offers this local alternative
explicitly; it never changes job limits, permissions, installed files, registry
or licensing. WMI/child job behavior is described in
[Windows job objects](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects);
the launch result alone does not prove jobs caused the legacy exit.

Real capture also exposed valid zero-word dispatcher calls between sentences.
They are now represented without array reads and audited separately; rejection
is not silently ignored. Worker success still requires all four stages, zero
skipped reads and no pending call. Owner death/server exit cannot count as
successful linguistic capture. The controller releases SAPI before reaping
only its retained diagnostic server, including failure paths; normal SAPI
listening does not adopt this diagnostic-only cleanup policy.

Evidence from the fresh Win32 static Release build:

- Full CTest **36/36**; snapshot guards include negative counts and zero-word
  states with null arrays. Voice-free audit: four positive, seven negative,
  one mismatch-accounting cases. Frontend comparison has five negative guards.
- [22 original corpus pairs](metrics/m46b-original-corpus22-20261002-fresh.json)
  and [eight extra phrase pairs](metrics/m46b-original-extra-20261002-fresh.json):
  **30/30 original PCM pairs equal**, 112 complete four-stage calls, comprising
  39 linguistic and 73 zero-word calls; 560 source-phone records, zero skipped
  reads; 39/39 nonempty scan-model comparisons match. These establish capture
  and scan behavior, not full morphology or port sound parity.
- [Nine-profile PCM isolation](metrics/m46b-wrapper-isolation-20261002-fresh.json)
  retains the existing stable/M36–M44 renderer behavior.
- [Actual M44 frontend vs 22 originals](metrics/m46b-frontend22-20261002-fresh.json)
  and [extra phrase comparison](metrics/m46b-frontend-extra-20261002-fresh.json)
  align 25/30 cases exactly. The other five are excluded, not positionally
  compared after different normalization. Of 74 original marked words,
  69 stress positions agree and five differ; all five differences use the
  port's last-vowel heuristic. Eight original unmarked words have a resolved
  port lexical stress, and 41 words carry multiple original angle markers.
  A lexical stress index is not a perceived-prominence measurement.

The five observed mismatches include irregular/adjective/adverb/contextual
lanes that M44 deliberately does not resolve: for example, the original uses
the first vowel in the test's "будет" and "длинное", while M44 falls back to the
last. In "работу голоса", contextual genitive stress differs from the fallback.
The "акустика" controls agree in M44. No word-specific overrides were added.
Original context/marker selection and the raw separator producer must be
ported as general, oracle-tested primitives before acoustic integration.
The port authoring projection currently derives markers from resolved lexical
stress and spans from frontend punctuation; it does not consume the captured
original marker/lattice states. These are verified pipeline gaps, not proof
that a single gap explains every audible click or rushed word.

### Local reproduction

The optional `--render-trace` CLI takes the same arguments as `--render`, but
requires `original-sapi`. Other profiles are rejected before synthesis.
`run_original_capture.ps1` requires a fresh output directory, redirects private
logs, retains its render handle, bounds the render to 45 seconds, and reaps only
a direct original server whose lifetime/ownership match that render. It refuses
pre-existing servers rather than stopping the user's original player.

```powershell
./tools/run_original_capture.ps1 -Exe build/NicolaiTalker.exe `
  -TextFile C:\private\text.txt -OutputDir C:\private\fresh-trace -Trace
# Only when needed for the inherited background context:
# add -WindowsManagedLaunch
python tools/test_original_capture.py --exe build/NicolaiTalker.exe `
  --baseline-exe C:\private\previous\NicolaiTalker.exe `
  --corpus tools/parity_corpus_22.tsv --output C:\private\fresh-corpus `
  --report C:\private\fresh-counts.json --windows-managed-launch
python tools/compare_original_frontend.py --exe build/NicolaiTalker.exe `
  --voice C:\private\Elan --capture-root C:\private\fresh-corpus `
  --output C:\private\fresh-port --report C:\private\fresh-frontend-counts.json
```

Raw JSONL, text, voice resources and WAVs remain private. The committed reports
contain scalar counts/identifiers/hashes only. CI checks synthetic contracts,
script syntax and package behavior without proprietary inputs; it does not
replace the local installed-original oracle.
