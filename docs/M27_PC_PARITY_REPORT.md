# M27 PC parity report

Golden target: `reference_pack_20260924_222948`, original Windows
**ELAN TTS Russian (Nicolai 16Khz)**.

## Production M27

M27 enables only the fully recovered terminal `physical.int` t-node at a 1.6%
blend.  The incomplete interior physical table remains disabled.

Same 22 golden phrases:

- rendered: **22/22**;
- host tests: **20/20**;
- mean active-duration ratio: **0.99258981**;
- active-duration MAE: **5.6830%**;
- normalized 12-bin F0 trajectory relative MAE: **13.4680%**;
- mean active waveform correlation: **0.19539801**;
- total-duration MAE: **3.5969%**;
- trailing-silence MAE: **18.12 ms**.

## Versus M26 / M25 production baseline

M26 intentionally preserved M25 audio.  Its baseline was:

- active-duration ratio: `0.99247065`;
- active-duration MAE: `5.6908%`;
- F0 trajectory MAE: `13.4901%`;
- active waveform correlation: `0.19432888`;
- total-duration MAE: `3.6030%`.

M27 therefore makes a small but simultaneous improvement in raw waveform
correlation, F0 contour, active timing, and total timing without enabling the
known-incomplete interior physical selector.

The raw-correlation gain is about **0.55% relative** (`0.19433 -> 0.19540`).
The absolute F0-contour error falls from **13.4901% to 13.4680%**.

The gain is intentionally modest: M27 is using only one exact terminal node.
A diagnostic full-table blend still regresses correlation because the PC
context selector for the seven additive alternatives is not yet completely
ported.
