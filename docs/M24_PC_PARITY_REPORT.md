# M24 PC parity report

Golden target: `reference_pack_20260924_222948` from the original Windows
**ELAN TTS Russian (Nicolai 16Khz)**.

## M23 baseline

- 22/22 rendered
- mean active waveform correlation: **0.19938**
- mean active-duration ratio: **0.99510**
- active-duration MAE: **5.52%**
- normalized 12-bin F0-trajectory relative MAE: **18.49%**
- MFCC-DTW mean distance: **2.2171**

## M24 production

- 22/22 rendered
- mean active waveform correlation: **0.19507**
- mean active-duration ratio: **0.96985**
- active-duration MAE: **6.41%**
- normalized 12-bin F0-trajectory relative MAE: **14.20%**
- MFCC-DTW mean distance: **2.2108**

The F0 trajectory error falls by about **23% relative**, and the time-warped
MFCC distance improves slightly. Raw waveform correlation decreases ~2.2%
because local F0 changes alter sample phase while the current duration/join
model is still imperfect. For M24 the pitch-contour gain is kept because it is
structurally supported by the recovered PC three-point record and is materially
closer in the intended prosodic dimension.

The next parity target is to recover the upstream generator of the three signed
percent offsets, replacing the corpus-wide declination anchors with the real
Russian rule output.
