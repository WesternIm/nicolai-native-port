# M33 — verified PC SEG timeline and TDS step arithmetic

Base: merged M32, Git commit `a4501dc`. M33 separates exact recovered
primitives from an experimental adapter to the existing portable renderer.
It does **not** claim a complete or bit-exact Windows synthesis port.

## Exact source timeline

Static tracing reached `mtsyc32.dll:0x101a30b0` through `0x101a2c30`.
The PCM/A-law branch (coding 1/2) expands a SEG record as follows:

| Node | Source sample | Voicing flag |
| --- | --- | --- |
| 0 | 0 | SEG mode >= 0 |
| each unvoiced slot | previous sample + sample_rate / 100 | 0 |
| each period slot | previous sample + abs(signed_period) | signed_period >= 0 |
| N+1 | PCM sample count - 1 | signed ANA end >= 0 |

There are **N+2 nodes**, not M14's N+1 or M15's N approximation. The phone
split selects node `SEG split + 1`. At 16 kHz the unvoiced slot width is exactly
160 samples. This replaces neither the waveform decoder nor the authoring
rules; it names the source-coordinate lattice consumed by the Windows engine.

M15 allocated remaining PCM samples proportionally across unvoiced slots and
placed all-voiced marks using symmetric slack. Those were useful approximations
but are not this PC expansion. A negative first period is a zero **node voicing
flag**, not evidence of a phase reset. The historical `signed_period_reset`
field is retained for source compatibility and old baseline behavior.

`source_timeline_seg_m33` is verified by calling the original function in a
32-bit local probe: all node positions, flags, counts, and split indexes match
for **2665/2665** database diphones. Nothing from the DLL is copied into Git.

## Exact synthesis-step primitive

`0x10107c20` calls `0x10109800` to construct a five-WORD step record.
`legacy_tds_step_m33` ports the positive supported arithmetic domain:

- source period divided by Q11 pitch coefficient;
- source support multiplied by Q11 duration coefficient;
- zero-target fallback to one period;
- `4 * budget < 3 * period` grain-drop decision;
- grain-repeat loop while `3 * accumulated < 2 * budget`;
- signed Q11 period slope and saturated per-record delta;
- signed-WORD residual carry at record +8.

This is not equivalent to independent floating-point hops reset per run.
The local Win32 probe checks **683/683** synthetic input records against the
original function, including dropped/repeated grains, carry, varying periods,
pitch coefficients, and duration coefficients. Invalid inputs are rejected
instead of reproducing divide faults or pathological WORD-overflow loops.
The recovered helper is not yet wired into production: the shared half-phone
regulator must supply the correct coefficients and carry ownership first.

## Execution experiments, not exact PC claims

`layout_seg_runs_m33` adapts the exact source nodes to existing run slicing.
Eligible voiced nodes become explicit source marks, negative-node flags remain
effective, and the exact split feeds M32's independent phone-side duration and
energy map. The full start/middle/end F0 contour is preserved.

The adapter still crops source support at run boundaries, uses the existing
portable grain windows, and rejoins rendered buffers. Windows instead carries
state between units and writes windowed halves through `0x10109980`; callers
include `0x10108210`, `0x101086c0`, and `0x10108cf0`. That join machinery has
not been completely ported. Therefore exact source nodes alone do not prove
better output parity.

A second independent ablation disables M13's sample-correlation phase search.
It removes its extra left/right trims, but retains the raised-cosine overlap.
This is **not** advertised as the Windows join. Both experiments are opt-in;
their disabled controls preserve M32 sample-for-sample.

No phrase-specific cases or new corpus-fitted constants were introduced.
See `M33_PC_PARITY_REPORT.md` for promotion decisions and measured results.

## Local oracle identity

The inspected original DLL is 7,950,336 bytes, PE timestamp `0x412a0cb4`,
preferred image base `0x10000000`, image size `0x797000`, SHA256:

`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`

The probe refuses incompatible PE timestamp/base/size before calling internal
addresses. Use only this known original DLL, locally, with its dependencies.
The SHA256 must be checked before running it; the probe is not a sandbox for
untrusted DLLs.
