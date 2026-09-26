# M34 — exact window writer and stateful execution checkpoint

Base: merged M33, commit `3ae35cf`. This is a reproducible reverse-engineering
checkpoint, **not a completed bit-exact PC synthesizer or production promotion**.
Stable M31/M32/M33 synthesis stays selected by default.

## Corrected pipeline ownership

Full tracing corrects the M33 continuation's description of `0x1010d330`:
it packages an already-regulated descriptor into a queued record; it is not
the source-selection regulator. The relevant split is:

| Original address | Observed role |
| --- | --- |
| 0x101a2780 | Build coefficient lanes over previous-right + next-left phone support |
| 0x101a2c00 | Integer reciprocal pitch-ordinate interpolation |
| 0x1010d330 | Package coefficient lanes, source positions, flags and split |
| 0x10107c20 | Stateful interval loop, step records, dropped-node selection and carry |
| 0x101083b0 | Normal/repeated grain write path |
| 0x101086c0 / 0x10108cf0 | Initial / cross-descriptor transition paths |
| 0x10109980 | Q15 left/right windowed PCM writer |
| 0x10109be0 / 0x1010a020 | Window lookup / construction |

In the regulated descriptor, source positions begin at +0x14, voicing WORDs
at **+0xfb4**, duration Q11 at **+0x1784**, pitch Q11 at **+0x1f54**.
The no-feature branch of 0x101a2780 writes unity 2048 to the two coefficient
lanes, not to the voicing flags. 0x1010d330 copies N-1 entries of both lanes,
N source positions and N flags. Runtime 0x10107c20 passes duration through
descriptor +0x24 and pitch through +0x28 to the M33 step primitive.

0x101a2780 uses combined source support
`(previous.last - previous.split) + (next.split - next.first)`.
Its duration coefficient is total feature duration shifted by 11, divided
by that support, with a zero-result fallback to 1. It also normalizes missing
pitch anchors in place, maintains a segment cursor across both halves, and
has a terminal next-left interval adjustment. **The full builder is not
ported here.** The opt-in shared-duration adapter mirrors ownership using
existing portable phone targets; it does not claim an exact feature producer.

## Exact recovered primitives

`legacy_reciprocal_pitch_m34(a, length, b, x)` implements 0x101a2c00:

```text
start = trunc(0x10000000 / a)
slope = trunc((trunc(0x10000000 / b) - start) / length)
result = start + slope * x
```

The positive ordinate's upstream units are deliberately not named Hz.
Arithmetic matches the original on **400/400** supported-domain cases.
This primitive is available for the later exact feature builder; the adapter
still samples the established three-point pitch-scale contour directly.

`legacy_tds_write_m34` implements the positive caller domain of 0x10109980.
For period P, left length L, right length R and output index i:

```text
left_product  = left[i] * left_window[i]                   if i < L, else 0
j             = i - (P - R)
right_product = right[j] * right_window[R - 1 - j]         if j >= 0, else 0
output[i]     = signed_WORD(arithmetic_shift_right(left_product + right_product, 15))
```

Products are added **before** shifting; separate product rounding is wrong.
There is no weight-sum normalization, amplitude clipping, or correlation
search in this primitive. If L+R<P it writes a zero gap. It overwrites P
samples at runtime +0x48, adds P to +0x3c, and leaves +0x48 unchanged: the
caller advances the cursor. Prefix/suffix sentinels and both state fields
were checked against the DLL on **1024/1024** cases, including zero windows,
overlaps and gaps. Safety checks reject unsupported lengths before writing.

The M33 Q11 primitive additionally matches **1000/1000 sequential records**
with each call consuming the preceding signed-WORD carry, rather than
resetting carry for independent test cases. M33's original 2665 source
timelines and 683 isolated step checks were rerun successfully.

## Experimental execution adapter

`StatefulTdsM34` owns carry and diagnostics for one utterance. Source intervals
come from exact M33 SEG nodes; negative node flags leave local pitch at unity.
Each interval consumes its left/right phone duration coefficient and the
three-point F0 contour through Q11 step calculation. Grain spacings follow
`first_period + arithmetic_shift_right(ordinal * delta_q11 + 1024, 11)`.
Phone-local energy remains live with a 5-ms blend around the source split.

The adapter appends its interval writes on one integer clock, without old
run joins or correlation-trim diphone joins. No phrase text/ID is consulted.
An optional shared phone coefficient owns both neighboring half-diphones;
it is not renormalized back to a per-diphone duration.

These pieces are still **approximations**:

- Analytic M14 half-Hann windows, not the exact PC lookup/construction policy.
- Current-interval front/tail source selection, not PC's buffered previous
  records, transition selection, cross-descriptor windows or rewind.
- Direct contour-to-Q11 mapping, not the complete 0x101a2780 feature builder.
- Boundary phone targets and terminal flush remain existing portable policy.

Accordingly this path is opt-in only. Synthetic contracts and golden metrics
must not be presented as proof of full PC synthesis equivalence.

## Evidence and local-only inputs

Known original DLL: mtsyc32.dll, SHA256
`f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`,
PE timestamp `0x412a0cb4`, image base `0x10000000`, image size `0x797000`.
The optional Win32 probe loads original executable code only after checking
the three PE identifiers. Check SHA256 yourself first; do not use arbitrary
DLLs. The DLL, disassembly dumps, voice-data and all WAVs remain local.

Committed evidence: oracle counts, per-phrase JSON/CSV, aggregate gate,
per-phrase deltas, stable-output SHA256 identity and source/input manifest.
See [parity report](M34_PC_PARITY_REPORT.md) and
[reproduction / M35 boundary](M34_CONTINUATION.md).
