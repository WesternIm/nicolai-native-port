# M22 PC parity report

Golden target: 22 WAVs from the original Windows `ELAN TTS Russian (Nicolai 16Khz)` engine.

## Production M22

M22 deliberately preserves M21 acoustics while adding reverse-engineered
neighbour diagnostics.

| metric | M22 |
|---|---:|
| phrases rendered | 22 / 22 |
| host tests | 20 / 20 |
| mean active-duration ratio | 0.996600 |
| active-duration MAE | 5.373% |
| active-duration RMSE | 7.406% |
| mean active alignment correlation | 0.192474 |
| total-WAV duration MAE | 3.058% |
| mean absolute trailing-silence error | 16.37 ms |
| WAVs bit-identical to M21 | 22 / 22 |

## Rejected experiments

### Coarse recovered phone-class proxy

| strength | active MAE | mean correlation |
|---:|---:|---:|
| 0.00 (M21/M22 default) | 5.373% | 0.19247 |
| 0.10 | 5.411% | 0.18807 |
| 0.15 | 5.396% | 0.17951 |

The class coefficients are real `wordstr.par` values, but the DLL applies them
under more specific feature predicates.  Global application is therefore not
accepted.

### SEG split asymmetric retiming

| strength | active MAE | mean correlation |
|---:|---:|---:|
| 0.25 | 5.418% | 0.17610 |
| 0.50 | 5.732% | 0.18367 |
| 0.75 | 5.714% | 0.18961 |
| 1.00 | 5.844% | 0.17420 |

The database split point is real, but using it as a direct independent
left/right duration regulator is not equivalent to the PC implementation.

### Global phone-scale retune

Changing the production `1.08` phone scale to `1.076` produced active MAE
5.644% and mean correlation 0.18367, so the M21 value is retained.

## Conclusion

M22 is a reverse-engineering/diagnostic milestone rather than a cosmetic audio
change.  The reference test prevents regressions while the next missing legacy
state (`0x400`/`0x800` feature construction) is recovered.
