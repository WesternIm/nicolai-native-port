# M32 findings — phone-local duration and energy execution

M32 moves the portable execution side to the granularity already proven on the
Windows Nicolai runtime in M31. The `physical.int` authoring values are still
produced per phone; they are no longer collapsed into one weighted correction
for the complete diphone.

## Execution changes

For diphone `phone[i] -> phone[i+1]`, the recovered SEG transition splits the
source PCM into a left and right phone support. M32 now applies:

```text
left_duration  = base_left  * (1 + strength * l[i]   / 100)
right_duration = base_right * (1 + strength * l[i+1] / 100)

left_gain  = 1 + strength * (e[i]   / 100 - 1)
right_gain = 1 + strength * (e[i+1] / 100 - 1)
```

The two duration scales are consumed by an invertible piecewise-linear source
to synthesis-time map. Voiced runs continue to use TD-PSOLA; unvoiced runs use
the existing duration-only Hann OLA. Energy is applied to the corresponding
rendered phone side with a bounded 5 ms transition around the SEG boundary.

## Three-point F0 preservation

The old M23 phone-side experiment accepted only one pitch ratio and therefore
collapsed M24+'s start/middle/end contour whenever left and right duration
scales differed. M32 accepts the complete `TdPsolaConfig`, restricts the
three-point contour to each voiced SEG run, and evaluates the local pitch ratio
for every synthesis hop. Phone-local duration no longer disables the pitch
contour.

## Exact fallback

Equal side durations and unity energy dispatch to the existing M15/M24+
renderer. With production `physical_length_strength=0` and
`physical_energy_strength=0`, M32 therefore keeps the M31 execution path rather
than re-rendering through a numerically similar replacement.

`hybrid_psola_test` covers:

- byte-identical zero-effect dispatch with a live three-point F0 contour;
- unequal left/right duration with a contour that remains observably different
  from a flat-pitch render;
- left-only energy attenuation without changing the right-side tail.

All 20 host tests pass on MSVC 19.51. CMake now selects `/utf-8` under MSVC so
the UTF-8 Russian `char32_t` literals build without relying on the host code
page.

## Reproducible parity tooling

`tools/measure_parity.py` reports the complete M32 scorecard from one run:

- active waveform correlation;
- active-duration ratio and MAE;
- total-duration MAE;
- normalized 12-bin F0-contour MAE;
- 12-coefficient MFCC-DTW;
- RMS ratio and RMS-ratio MAE.

`tools/run_m32_parity.ps1` renders and measures one candidate, while
`tools/sweep_m32_physical.ps1` evaluates the baseline and a configurable
length/energy strength grid. A reference-vs-itself validation over all 22
golden WAVs returns correlation 1.0, duration/F0/RMS MAE 0, MFCC-DTW 0, and RMS
ratio 1.0.

## Production decision

The current source handoff contains the golden reference pack and M31 source,
but not the proprietary `nicolai16.dat`, `exc_rus.txt`, or `abb_rus.txt` needed
to render a portable corpus. M32 therefore does **not** invent a winning
strength: both production strengths remain zero until the original local voice
inputs are restored and the committed sweep is executed. See
`M32_CONTINUATION.md` for the exact continuation point.
