# M35 — calibrated diagnostics and duration attribution

Base: M34 commit `a88a2a25b2fb57ffcb2b722e6ef29e4b9ceb0076`.
The M35 branch is stacked over the open M34 branch, not over main. It changes
diagnostics and tests, **not synthesis policy, coefficients or audio**.
It is a measurement/reverse checkpoint, not a completed PC renderer.

## What the duration evidence actually proves

M35 records three clocks in the experimental `StatefulTdsM34` adapter:

| Clock | Definition |
| --- | --- |
| target | Sum of integer interval width × selected duration Q11 / 2048; existing zero-target period fallback |
| budget consumed | Sum of previous carry + target − next carry |
| emitted | Sum of periods actually written by the Q15 writer |

The batch diagnostic is `CLOCK id pre_flush_frames target budget emitted
clamped_delta_records`; existing `TDS` lines retain the final carry.
Counters are accumulated in transactional state and do not feed the algorithm.
The audit asserts `target - final_carry == budget`, `emitted == pre_flush_pcm`
and `wav_frames == pre_flush_pcm + 4800` for all 22 phrases of each experiment.

Shared-phone totals: target 453138 frames, emitted 452959 frames. Maximum
absolute writer-minus-budget discrepancy is 31 frames (1.94 ms); maximum
writer-minus-target is 108 frames (6.75 ms). Mean absolute writer-minus-target
is 2.46875 ms per phrase. Target plus flush already has **10.680579%** total
duration MAE against PC; the actual output has **10.594057%**. Thus the large
observed duration mismatch is already present in the portable target policy,
before writer rounding/carry. No proportional correction was applied.

This does **not** prove that every PC runtime clock operation is implemented.
In particular, the adapter still lacks cross-descriptor selection/rewind.
Nor does it establish which missing feature rule causes each target mismatch.
The portable shared target comes from existing phone-duration hints, not a
captured original coefficient lane. The original `0x101a2780` builder remains
the next exact boundary. Both target correctness and rollback must be tested
against original runtime records rather than inferred from a corpus optimum.

There are 315 saturated Q11 delta records in the shared run. Their presence
is reported, not blamed for the whole duration error: actual clock residuals
are small. Exact step tests still match the DLL. M35 does not remove saturation
or change rounding to improve corpus metrics.

## New F0 measurements are not automatically trustworthy

Historical metrics remain unchanged and reproducible. M34 established that
the historical YIN 150-Hz ceiling can create octave-down errors on known tones;
raising it to 400 Hz can reverse candidate rankings. M35 does not silently
replace the old metric with an equally unvalidated new score.

`tools/measure_parity_v2.py`, version `m35-v2.2`, adds:

- pYIN 55–400 Hz with its Viterbi voiced flags; no percentile RMS admission gate.
- A separately implemented normalized-autocorrelation pitch check. The earliest
  local peak within 0.02 of the best and at least 0.70 is used, with parabolic
  lag interpolation. It is reported both on the pYIN-matched subset and on its
  own finite subset; neither estimator is authoritative PC ground truth.
- Shape DTW using globally RMS-normalized audio and MFCC 1–12 (excluding C0),
  FFT 512, hop 160, band radius 0.20. Removing C0 alone would not guarantee gain
  independence; normalization and half-gain contracts check that explicitly.
- Raw, unnormalized 1024-frame RMS energy on the same alignment, in dB, for
  reference RMS >0.003. Pitch-window sensitivity does not change shape/energy.
- Conditional F0 cents error, >20% gross error, near-octave error, V/UV mismatch,
  missing/false-positive counts, and per-phrase/pooled matched coverage.
- Many-to-one DTW path entries collapsed to one vote per reference frame.
- Cache identity includes code/parameter/dependency versions, raw duration,
  active bounds and cropped samples, so silence padding cannot reuse a stale
  cached duration. Feature caches remain local and ignored.

Both 1024- and 2048-sample pitch windows are reported. Known synthetic truth
passes at both sizes: 83/120/166/220-Hz tones within 2%, a harmonic 120→180-Hz
chirp within 100 cents with >95% admission, voiced/noise/voiced transitions,
silence and seeded white noise, identity, half gain (6.0206 dB) and octave up.
These contracts were fixed from known signals, not golden-candidate rankings.
The public [librosa source](https://librosa.org/doc/main/_modules/librosa/core/pitch.html)
also makes clear that pYIN derives candidates from YIN; it is not independent
of the historical estimator just because it adds probabilistic voicing.

On **real speech**, baseline pooled matched coverage is only 29.24% / 32.96%
for 1024 / 2048 respectively, with matched F0 in only 9 / 11 of 22 phrases.
At 1024, even four PC reference phrases have no admitted pYIN-voiced frames.
Increasing the window admits more reference frames but does not resolve low
candidate overlap. The scalar F0 error averages exclude null phrases and
compare different admitted subsets. They cannot support an all-phrase F0
winner. All 22 reference identity controls nevertheless pass: an identity test
checks arithmetic/alignment, not an estimator's accuracy on real speech.

No claims of perceptual equivalence, exact speech F0 or held-out generalization
are made. Golden traces contain progress/event timestamps, not phone/F0 labels.
Reliable speech F0 ground truth still needs original feature/period captures or
an independently labeled reference set. Do not relax voicing until a preferred
candidate wins; that would fit the measuring instrument to the result.

## Validation and preserved boundary

The final clean Debug build passes **20/20** portable tests. New assertions
cover target/carry conservation across dropped intervals and continuation,
writer sample counts, energy-independent clocks, and rejected-input state.
Six additional voice-free Python metric contracts pass, including null F0,
independent AC admission, reference-frame weighting and cache duration safety.
They are run separately in CI without proprietary inputs.

The rebuilt x86 probes were rerun on the locally installed original DLL,
SHA256 `f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7`,
after checking that hash. Results: 1024/1024 writer, 400/400 reciprocal,
1000/1000 sequential step, 2665/2665 source timeline and 683/683 isolated
step matches. These validate primitives, not the missing full runtime loop.

All six settings were rendered on the same 22 PC phrases and compared against
saved M34 WAV SHA256s: **132/132 byte-identical**. No phone/sentence-specific
rules were added. Production remains on the stable M31-compatible renderer;
stateful/shared execution is off and physical length/energy strengths stay 0.

Only scalar metric reports, hashes, code and findings are published. Original
MSI/DLL/resources, voice data, reference/rendered WAVs, feature NPZs and raw
disassembly are not added by this checkpoint. See M35_CONTINUATION.md for
recovery from GitHub and the next executable research boundary.
