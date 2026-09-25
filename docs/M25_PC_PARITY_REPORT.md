# M25 PC parity report

Golden target: `reference_pack_20260924_222948`, original Windows
**ELAN TTS Russian (Nicolai 16Khz)**.

## M24 baseline

- 22/22 rendered
- mean active waveform correlation: **0.19507**
- mean active-duration ratio: **0.96985**
- active-duration MAE: **6.41%**
- normalized 12-bin F0-trajectory relative MAE: **14.20%**
- total-duration MAE: **4.02%**

## M25 production

M25 defaults:

- phone duration scale: **1.11** (M24: 1.08)
- M24 three-point declination retained at strength **1.0**
- recovered sparse-anchor correction strength: **0.05**
- reconstructed general source anchors: phrase start **+4%**, phrase end
  **-18%**, stressed-vowel lift **+12%**, multiword-start lift **+4%**
- no phrase/word-specific exceptions

Measured on the same 22 PC WAVs:

- 22/22 rendered
- mean active waveform correlation: **0.19433**
- mean active-duration ratio: **0.99247**
- active-duration MAE: **5.69%**
- normalized 12-bin F0-trajectory relative MAE: **13.49%**
- total-duration MAE: **3.60%**
- trailing-silence absolute error: **18.12 ms**

Relative to M24, M25 improves the F0-trajectory error by about 5%, active
length error by about 11%, and total-length error by about 10%. Raw waveform
correlation changes only slightly (-0.00074 absolute), which remains dominated
by phase/join mismatch in the not-yet-identical TD-PSOLA path.

A pure sparse-anchor replacement was explicitly tested. It could raise raw
correlation in some settings, but consistently worsened the normalized F0
trajectory; it is not shipped as the production default. M25 therefore uses a
small sparse correction while reverse engineering continues toward the exact
upstream PC anchor-authoring rule.
