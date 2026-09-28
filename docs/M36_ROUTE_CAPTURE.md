# M36 original caller route capture (2026-09-27)

This checkpoint adds diagnostic infrastructure, NOT an acoustic correction.
Production rendering and both experimental executors are unchanged. No original
22-phrase route capture, PCM parity result, or acoustic gain is claimed.

## Original startup boundary

On this host, the 32-bit Nicolai SAPI token is present and voice selection
returns. The bounded trigger then stalls inside `SAPI.SpVoice.Rate = 0`.
It never reaches volume, stream setup, or `Speak`. Warm-up text now comes from
the explicitly UTF-8 corpus instead of a Cyrillic literal in a BOM-less
Windows PowerShell 5.1 script.

`ettsengine.exe /nogui` launched from its installed working directory loads
`mtsyc32.dll`, its dependencies and `ELP.dll`, then exits with code 0 at 250 ms.
The trace contains no unhandled exception; its only exception is the normal
initial debugger breakpoint. A separate `/nogui /autoexit` launch also exits 0.
The server's reason for terminating is NOT established. Do not infer a missing
dependency, acoustic mismatch, or licensing fault from the module list.

Static inspection of the installed SAPI adapter's startup routine
`0x10003123..0x1000330b` identifies the unbounded wait: after successful
`CreateProcessA`, it waits for `ETTSSDK Ready` with an infinite timeout and does
not check child-process death. This explains why an early normal server exit
can strand SAPI initialization, but does not establish why the server exited.
No installation files, registry settings or license data were changed.

## Bounded startup trace

Build with an x86 compiler environment and an existing x86 CMake build:

```powershell
cmake -S . -B build-win32 -A Win32 -DBUILD_TESTING=ON
cmake --build build-win32 --config Debug --target nicolai_legacy_startup_trace
.\build-win32\Debug\nicolai_legacy_startup_trace.exe `
  'C:\Program Files (x86)\Elan\ettsengine.exe' `
  'metrics-work\server-startup-fresh.jsonl' 15 '/nogui'
```

The output directory must exist; the output file must not exist. The tracer
starts a hidden, owned child with the Windows Debug API, makes no code patches,
logs module/debug/exception/exit events, and terminates only that child on
timeout (1..60 seconds). It never attaches to or terminates an existing server.
Exit codes: 0 normal child exit, 1 tool error, 2 usage, 3 timeout, 4 nonzero child
exit. Exit 0 says nothing about voice readiness.

## Route instrumentation

```powershell
.\tools\run_m36_runtime_capture.ps1 -CaptureMode route `
  -BuildDir build-win32 -OutputRoot metrics-work/m36/route-fresh
```

`-CaptureMode features` remains the default. All runs now require a fresh
output root and preserve earlier evidence. Existing NMake caches no longer
receive the incompatible `-A Win32` option. Trigger timeouts identify the last
completed/entered SAPI stage.

After a successful warm-up, the route mode attaches to `mtsyc32.dll` in the
original server and guards its PE timestamp, preferred base and image size.
These identifiers are not a cryptographic hash of the loaded module.
It instruments the pinned image's instruction boundaries:

| RVA | Event |
| --- | --- |
| `0x107c20` | caller entry, context flush gate and both runtime descriptors |
| `0x107d54` | interval decision, generated step, pending-cross and started flags |
| `0x107d8b` / `0x107e28` / `0x107ebe` | actual cross / initial / ordinary call site |
| `0x108004` | interval-end state and step-slot snapshots |
| `0x10801b` / `0x108076` | post-loop and actual terminal-flush call site |
| `0x10808b` / `0x10820d` | failed-write / normal caller return |

Records carry thread/call identifiers, six WORD markers, fields `+30..+40`,
cursors/selections `+48..+5c`, end cursor, carry, and five-WORD records at the
two caller-owned slot pointers. Runtime descriptors are pointer-backed, unlike
the inline feature-builder descriptors: flags `+10/+14`, node-count pointer
`+18`, duration/pitch pointers `+24/+28`, positions `+30`, PCM address `+4c`.
PCM addresses are provenance only: this mode does NOT dump PCM samples.

The caller's cross-pending local at stack `+48` is not the descriptor flag at
`+14`. Do not label the latter as a proved flush/rollback gate. The generated
step's index WORD is assigned after the decision breakpoint, so its captured
index is taken from ESI rather than uninitialized stack bytes.

Rows are staged before writing, preventing partial JSON on failed memory reads.
Skipped/incomplete captures return nonzero. Normal route-mode detach requests
a debugger break and restores instructions while the target is stopped, then
releases manually suspended peers. Breakpoint changes affect only process
memory and are restored; no proprietary file is modified.

## Audit and evidence limits

`audit_m36_routes.py` verifies complete interval sequences and compares observed
writer calls with the compiled `legacy_runtime_route_m36` primitive through
`nicolai_m36_route_probe`. A zero-count interval without a writer is a drop;
a positive terminal interval without a writer is deferred. Empty, orphan,
duplicate-writer, reordered and truncated traces are rejected. Original
failed-write returns and route disagreements prevent success.

```powershell
python tools/audit_m36_routes.py metrics-work/m36/route-fresh/caller-route.jsonl `
  --probe build-win32/Debug/nicolai_m36_route_probe.exe `
  --output metrics-work/m36/route-fresh/manual-audit-fresh.json
```

The audit does NOT yet replay PCM, validate flush policy or compare state/slot
bookkeeping against portable output. Those snapshots are captured for the next
comparison stage, not silently treated as validated. Raw descriptor arrays,
audio, startup logs and captures stay under ignored `metrics-work/` locally.

Validated locally: full x64 CTest 28/28; targeted x86 CTest 4/4 including
synthetic memory snapshot contracts; 14 voice-free route-auditor contracts;
startup-debugger synthetic-child checks for normal/nonzero exit, timeout,
debug-string escaping and refusal to overwrite prior traces. These are NOT
live original route-oracle passes. Win32 CI builds/runs the new contracts.

The real route runner was attempted with a fresh root and a 5-second warm-up
limit: it stopped at `M36_SAPI_STAGE set-rate`, produced zero route records,
and cleaned up its trigger child. This failure is preserved locally and is not
an audit pass. The next dependency is a successfully initialized original voice
session; only then capture routes, replay bookkeeping/rollback, and repeat the
exact 22-phrase acoustic A/B before changing any default.
