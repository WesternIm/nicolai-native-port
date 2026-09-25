# M29 PC parity report

Golden target: original Windows **ELAN TTS Russian (Nicolai 16Khz)** reference
pack `reference_pack_20260924_222948`, 22 phrases.

Production M29 deliberately leaves
`physical_single_marker_pitch_strength=0.0`.  The new ordinary-`<` authoring
path is exact enough to test, but the measured non-zero blends trade raw
waveform alignment against F0-trajectory error instead of improving both.

## Production metrics

- rendered: **22/22**;
- host tests: **20/20**;
- mean active-duration ratio: **0.9925898**;
- active-duration MAE: **5.6830%**;
- mean active waveform correlation: **0.1953980**;
- normalized 12-bin F0 trajectory MAE: **13.4680%**;
- total-duration MAE: **3.5969%**.

The production 22-WAV corpus is **byte-identical to M28**.  M29 is therefore a
reverse-model/conformance milestone rather than a manufactured acoustic bump.

## Diagnostic candidates

Two representative non-zero points demonstrate the current tradeoff:

- single=0.001, terminal=0.015 -> correlation ~0.19631, F0 MAE ~13.498%;
- single=0.003, terminal=0.018 -> correlation ~0.19494, F0 MAE ~13.442%.

Neither is a robust Pareto improvement over production.  Production remains at
zero until the remaining authoring-span/context state is reproduced.
