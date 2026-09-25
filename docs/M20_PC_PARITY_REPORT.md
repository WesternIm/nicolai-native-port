# M20 — PC-reference timing conformance pass

M20 uses `reference_pack_20260924_222948.rar` — the supplied 22-phrase original Windows Nicolai capture — as the golden target. The port is changed to match the PC engine; the reference is never changed to match the port.

## Main finding

M19 treated every internal `#` word boundary like a raw acoustic boundary unit. Those units are large in the database (often roughly 180–240 ms each), so multi-word utterances accumulated excessive duration. The original `mtsyc32.dll` has a separate `wordstr.par` prosody layer and does not simply preserve every raw boundary at 1.0x.

## M20 changes

- preserve punctuation type in the Russian frontend while keeping the acoustic phone chain unchanged;
- add an explicit legacy timing policy separate from `duration.par` and the acoustic decoder;
- calibrate internal phone timing to 1.08x relative to the M18 duration-table approximation;
- retime ordinary inter-word boundaries to 0.45x instead of raw 1.0x;
- give comma boundaries their own 1.40x policy; this fixes the `привет, Николай!` reference duration without globally stretching all word gaps;
- distinguish ASCII hyphens from punctuation dashes so `USB -> У-Эс-Бэ` does not acquire false long pauses;
- retain utterance start/end boundary handling separately;
- keep the 2.0x PC reference gain stage separate from timing/PSOLA.

## Corpus result (22/22)

- Mean active-duration ratio: **1.108 -> 0.994** (ideal 1.000)
- Mean absolute duration error: **20.2% -> 5.4%**
- Duration RMSE: **25.1% -> 7.4%**
- Median absolute duration error: **16.2% -> 4.3%**
- Mean active waveform search correlation: **0.175 -> 0.190**
- Tests: **20/20 passing**.

## Per phrase

| ID | Text | M19 ratio | M20 ratio | M20 corr |
|---:|---|---:|---:|---:|
| 001 | мама | 0.874 | 0.928 | 0.206 |
| 002 | папа | 0.730 | 0.792 | 0.324 |
| 003 | привет | 0.867 | 0.928 | 0.225 |
| 004 | молоко | 0.852 | 0.927 | 0.270 |
| 005 | сказка | 1.100 | 1.165 | 0.233 |
| 006 | вокзал | 1.037 | 1.082 | 0.168 |
| 007 | подписка | 0.956 | 1.007 | 0.262 |
| 008 | яма | 0.940 | 1.034 | 0.218 |
| 009 | мир | 0.945 | 0.980 | 0.339 |
| 010 | люблю | 0.979 | 1.049 | 0.315 |
| 011 | щёлк | 1.068 | 1.081 | 0.156 |
| 012 | все будет хорошо | 1.277 | 0.990 | 0.112 |
| 013 | что ты делаешь? | 1.484 | 1.065 | 0.157 |
| 014 | я пришёл домой. | 1.306 | 0.977 | 0.148 |
| 015 | привет, Николай! | 0.824 | 0.992 | 0.095 |
| 016 | это проверка синтеза речи. | 1.321 | 1.035 | 0.130 |
| 017 | сегодня хорошая погода. | 1.182 | 0.999 | 0.145 |
| 018 | сто двадцать три рубля | 1.379 | 1.019 | 0.152 |
| 019 | 123 рублей | 1.286 | 0.964 | 0.107 |
| 020 | USB 123 | 1.591 | 1.000 | 0.115 |
| 021 | 2026 год | 1.287 | 0.942 | 0.138 |
| 022 | мама папа | 1.088 | 0.922 | 0.166 |

## Remaining gap

Timing is no longer the dominant gross failure for multi-word text. The next parity target is the original context-dependent `wordstr.par` duration/prosody logic and pitch/PSOLA scheduling inside `mtsyc32.dll`. Single-word outliers such as `папа` and `сказка` show that `(left_ms + right_ms)/2` plus one global phone factor is still only an approximation. Waveform correlation therefore remains far from bit-exact PC parity.
