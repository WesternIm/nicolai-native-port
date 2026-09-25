# M30 PC parity report

Golden target: original Windows **ELAN TTS Russian (Nicolai 16Khz)** reference
pack `reference_pack_20260924_222948`, 22 phrases.

M30 corrects physical pitch authoring from the previous per-word approximation
to the Windows writer's `"(/)"`-bounded authoring-span state.  After that
correction the exact ordinary `<` pitch layer is enabled conservatively at
`physical_single_marker_pitch_strength=0.002`.

## M29 -> M30

| metric | M29 | M30 |
|---|---:|---:|
| rendered | 22/22 | **22/22** |
| host tests | 20/20 | **20/20** |
| mean active waveform correlation | 0.1953980 | **0.1965157** |
| active-duration MAE | 5.6830% | **5.6790%** |
| mean active-duration ratio | 0.9925898 | **0.9925499** |
| normalized 12-bin F0 trajectory MAE | **13.4680%** | 13.4799% |
| total-duration MAE | **3.5969%** | 3.5987% |
| mean 12-coefficient MFCC-DTW distance | 128.8036 | **128.5899** |

The raw waveform-correlation gain is about **0.57% relative**, and the optional
MFCC-DTW speech-spectrum metric improves about **0.17% relative**.  F0
trajectory changes by only +0.012 percentage point and total-duration MAE by
+0.0018 percentage point.  The selected point is therefore an acoustic-parity
tradeoff, not a claim that every scalar metric improves simultaneously.

The sweep around the selected value shows a clear local correlation optimum:
0.001 -> 0.1963734, 0.002 -> 0.1965157, 0.003 -> 0.1963752.  Values above
0.004 lose most of the gain.  No phrase-specific tuning is present.

## What actually changes

Because the exact marker path only applies where the reconstructed annotation
state exposes an ordinary `<` event, only a subset of the 22 WAVs changes.
This is expected and preferable to a global pitch multiplier.  The remainder
stay byte-identical to the M29 path.

## Remaining dominant gaps

- exact long-text authoring-code / `(/)` splitter state;
- complete `<<` / `<<<` semantics across all l/e/t writer passes;
- replacement of the reconstructed global M24/M25 F0 backbone by native
  physical-prosody output;
- TD-PSOLA pitch-mark/join/phase parity, which remains the main limiter of raw
  sample correlation;
- residual phone-specific duration mismatches (`папа`, `сказка`, etc.).
