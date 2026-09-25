# M26 PC parity report

Golden target: `reference_pack_20260924_222948`, original Windows **ELAN TTS Russian (Nicolai 16Khz)**.

## Production default

M26 adds the recovered `physical.int` table and authoring/classification infrastructure but leaves `physical_pitch_strength=0` because an incomplete direct class mapping regressed the golden corpus. Therefore the production default is intentionally **audio-identical to M25**.

Measured on the same 22 PC WAVs:

- 22/22 rendered;
- 20/20 host tests pass;
- mean active-duration ratio: **0.99247065**;
- active-duration MAE: **5.6908%**;
- normalized 12-bin F0-trajectory relative MAE: **13.4901%**;
- mean active waveform correlation: **0.19432888**;
- total-duration MAE: **3.6030%**.

These equal M25 by design. The milestone is that the next prosody work can now consume the original 14x42 physical table instead of guessing its values.

## Rejected word-wise physical blend

The later, more faithful diagnostic maps `physical.int` **per word/position** (matching the recovered `state+0x130` class lattice) instead of applying one row to an entire utterance. With strength `0.025`, internal class `0`, and terminal class `1`, the 22-phrase result is:

- F0-trajectory MAE: **13.4540%**;
- active waveform correlation: **0.19021950**;
- active-duration MAE: **5.8924%**;
- total-duration MAE: **3.4794%**.

F0 and total container length improve slightly, but active alignment and timing regress. A terminal-class grid over rows 1..6 also shows that no single row is globally correct; the PC classifier really does choose classes from punctuation, interrogative lexemes and morphology. The experiment is therefore retained as a diagnostic, not a production default.

This is preferable to overfitting the 22 golden phrases with phrase-specific class assignments: M26 preserves the known-good M25 sound while moving the reverse-engineered PC architecture forward.
