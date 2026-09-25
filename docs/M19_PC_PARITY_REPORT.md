# M19 PC reference parity report

Reference: `ELAN TTS Russian (Nicolai 16Khz)` captured through 32-bit SAPI5.

## Summary

- Corpus: 22/22 portable phrases rendered successfully.
- Mean active-duration ratio (portable / PC): 1.108.
- Median residual peak gain needed after M19 gain calibration: 1.018.
- Mean active-waveform search correlation: 0.175.
- The old `missing_diphone_#_e4` failure on `это проверка синтеза речи.` is fixed in M19.
- SAPI terminal silence is reported separately and is not baked into the portable acoustic core.

## Per phrase

| ID | Text | Active duration ratio | Peak ratio | Search corr |
|---:|---|---:|---:|---:|
| 001 | мама | 0.874 | 0.949 | 0.217 |
| 002 | папа | 0.730 | 0.988 | 0.304 |
| 003 | привет | 0.867 | 0.953 | 0.194 |
| 004 | молоко | 0.852 | 1.002 | 0.208 |
| 005 | сказка | 1.100 | 1.000 | 0.252 |
| 006 | вокзал | 1.037 | 0.998 | 0.182 |
| 007 | подписка | 0.956 | 0.907 | 0.249 |
| 008 | яма | 0.940 | 0.874 | 0.168 |
| 009 | мир | 0.945 | 0.996 | 0.328 |
| 010 | люблю | 0.979 | 1.019 | 0.311 |
| 011 | щёлк | 1.068 | 1.090 | 0.162 |
| 012 | все будет хорошо | 1.277 | 0.983 | 0.116 |
| 013 | что ты делаешь? | 1.484 | 0.999 | 0.201 |
| 014 | я пришёл домой. | 1.306 | 0.973 | 0.100 |
| 015 | привет, Николай! | 0.824 | 1.016 | 0.104 |
| 016 | это проверка синтеза речи. | 1.321 | 0.983 | 0.137 |
| 017 | сегодня хорошая погода. | 1.182 | 0.907 | 0.133 |
| 018 | сто двадцать три рубля | 1.379 | 0.816 | 0.102 |
| 019 | 123 рублей | 1.286 | 0.831 | 0.101 |
| 020 | USB 123 | 1.591 | 0.688 | 0.110 |
| 021 | 2026 год | 1.287 | 0.732 | 0.065 |
| 022 | мама папа | 1.088 | 0.988 | 0.117 |
