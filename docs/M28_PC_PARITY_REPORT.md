# M28 PC parity report

Golden target: `reference_pack_20260924_222948`, original Windows
**ELAN TTS Russian (Nicolai 16Khz)**.

## Production M28

M28 deliberately keeps the new non-terminal exact-empty-marker blend disabled
because the measured corpus grid produced no joint waveform/F0 improvement.
The already validated M27 terminal blend remains 0.016.  Production audio is
therefore intentionally **byte-identical to M27** while the marker authoring
model becomes substantially more exact.

Same 22 golden phrases:

- rendered: **22/22**;
- host tests: **20/20**;
- mean active-duration ratio: **0.99258981**;
- active-duration MAE: **5.6830%**;
- normalized 12-bin F0 trajectory relative MAE: **13.4680%**;
- mean active waveform correlation: **0.19539801**;
- total-duration MAE: **3.5969%**;
- trailing-silence MAE: **18.12 ms**.

SHA-256 comparison of the 22 production WAVs against M27 gives **22/22 exact
matches**.

## Why ship an audio-neutral milestone?

M27's k4 correction had the right numeric branch but an overly broad semantic
label: it was projected onto every phrase-final phone.  M28 proves when k4 is
actually authored, proves `[t]` placement onto the preceding stressed letter,
recovers base 18/19 selection, and maps k0/k1/k2/k3/k4/k6.  Enabling only a
partial new branch before the k1/k3 ordinary-marker guard is known can improve
one corpus metric while worsening another, so production remains conservative.

This avoids overfitting the 22 reference phrases and leaves M29 with a precise
remaining target instead of accumulating another heuristic layer.
