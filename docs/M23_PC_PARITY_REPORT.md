# M23 PC parity report

Golden target: the 22 WAV files captured from original Windows
**ELAN TTS Russian (Nicolai 16Khz)** in `reference_pack_20260924_222948`.

All values below use the same comparator as M19–M22.  F0 diagnostics are a
separate librosa/YIN measurement and are not used as a pass/fail oracle.

| Metric | M22 production baseline | M23 | Direction |
|---|---:|---:|---:|
| rendered phrases | 22/22 | 22/22 | same |
| host tests | 20/20 | 20/20 | same |
| mean active duration ratio | 0.99660 | 0.99510 | ~same |
| active-duration MAE | 5.373% | 5.516% | +0.143 pp |
| active waveform alignment correlation | 0.19247 | **0.19938** | **+3.59% relative** |
| total-WAV duration MAE | 3.058% | 3.289% | +0.231 pp |
| trailing-silence MAE | 16.37 ms | 16.85 ms | ~same |
| median peak gain needed | 1.0337x | **1.0123x** | closer to 1.0 |
| median portable/reference F0 ratio | 1.0959 | **1.0183** | closer to 1.0 |
| mean absolute F0-ratio error | 10.16% | **7.90%** | **-22.2% relative** |
| median phrase F0 (reference / portable) | 84.26 / 93.28 Hz | **84.26 / 87.38 Hz** | closer |

M23 intentionally accepts a very small timing regression because it restores a
major missing subsystem: the original engine is centred around an 83-Hz runtime
pitch and expresses contour movement relative to that base.  The change improves
both the independent F0 diagnostic and the waveform-alignment score.

The timing regression is smaller than the pitch improvement and is expected to
be revisited when the upstream PC contour-event generator is recovered rather
than compensated with another global duration multiplier.
