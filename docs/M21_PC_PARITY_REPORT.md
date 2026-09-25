# M21 PC parity report

Golden reference: original Windows `ELAN TTS Russian (Nicolai 16Khz)` corpus,
22 phrases captured by NicolaiOracle M19 v2.

| Metric | M20 | M21 |
|---|---:|---:|
| Rendered phrases | 22/22 | 22/22 |
| Mean active-duration ratio | 0.99447 | 0.99660 |
| Active-duration MAE | 5.437% | 5.373% |
| Active-duration RMSE | 7.402% | 7.406% |
| Mean active best-lag correlation | 0.18997 | 0.19247 |
| Mean total-duration ratio | 0.75606 | 0.99777 |
| Total-duration MAE | 24.394% | 3.058% |
| Total-duration RMSE | 25.995% | 3.941% |
| Mean absolute trailing-silence error | 306.4 ms | 16.4 ms |

M21 is intentionally not declared waveform-identical. The largest remaining
errors are in context-dependent phone/syllable duration and the legacy PSOLA
feature logic, not output framing.
