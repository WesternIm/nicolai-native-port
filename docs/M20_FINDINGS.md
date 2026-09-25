# M20 findings

M20 is the second PC-reference conformance pass.

## Proven by the supplied PC corpus

The M19 long-phrase stretch was primarily caused by internal `#` word-boundary units being left at their raw database duration. Retiming those separately from utterance endpoints collapses the corpus-wide active-duration error from about 20% MAE to about 5.4% MAE.

The punctuation distinction is material: the comma in `привет, Николай!` needs a substantially longer boundary than an ordinary whitespace boundary, while the ASCII hyphens introduced by the legacy abbreviation `USB -> У-Эс-Бэ` must *not* be treated as punctuation dashes.

## Reverse-engineering support

The original `mtsyc32.dll` loads `rusvox\data\wordstr.par` into the Russian prosody context (pointer at the recovered context field `+0x3d04`). Its duration path multiplies phone timing by multiple `wordstr.par` coefficients, confirming that raw acoustic boundary duration is not the complete PC timing model. Separate recovered code also uses the `wordstr.par` tail values 120/240/400 while constructing pause/prosody items.

M20 does not pretend those old feature flags are fully named yet. It keeps the recovered data and uses a small explicit PC-corpus timing policy so the approximation is measurable and replaceable as more of the original logic is mapped.

## Current calibrated policy

- internal phone-duration correction: 1.08x
- ordinary inter-word `#`: 0.45x
- comma boundary: 1.40x
- weak punctuation default: 1.20x
- strong internal punctuation default: 1.60x
- utterance start/end boundary: 1.00x

Only the values exercised by the 22-phrase reference pack should be considered reference-validated.

## Additional static mapping from `mtsyc32.dll`

The duration/prosody routine around `0x10226cf0` applies `wordstr.par`
coefficients conditionally before rounding/storing per-item timing.  The
following accesses are now statically confirmed in the original PC DLL:

| wordstr offset | index | value | observed use |
|---:|---:|---:|---|
| `+0x08` | 2 | 0.80 | branch-selected base coefficient |
| `+0x10` | 4 | 0.60 | branch-selected base coefficient |
| `+0x1c` | 7 | 0.90 | first point of a 3-point contour/interpolation |
| `+0x2c` | 11 | 0.50 | conditional multiplier |
| `+0x30` | 12 | 0.85 | middle point of the 3-point contour |
| `+0x34` | 13 | 0.97 | conditional multiplier |
| `+0x38` | 14 | 0.80 | conditional multiplier near boundary/feature flags |
| `+0x3c` | 15 | 0.99 | conditional multiplier |
| `+0x40` | 16 | 0.30 | conditional multiplier |
| `+0x44` | 17 | 1.00 | final point of the 3-point contour |
| `+0x48` | 18 | 1.30 | conditional multiplier |
| `+0x54` | 21 | 0.60 | conditional multiplier |
| `+0x58` | 22 | 0.90 | conditional multiplier |
| `+0x5c` | 23 | 1.20 | conditional multiplier |
| `+0x60` | 24 | 2.00 | threshold/feature comparison path |
| `+0x64` | 25 | 0.80 | conditional multiplier |
| `+0x68` | 26 | 0.25 | rare conditional multiplier |
| `+0x6c` | 27 | 0.80 | conditional multiplier |
| `+0x70` | 28 | 0.95 | mutable prosody parameter used elsewhere |
| `+0x74` | 29 | 1.00 | mutable prosody parameter used elsewhere |
| `+0x78..0x88` | 30..34 | 120,240,400,20,150 | integer-scale timing/prosody limits |

This is why M20 deliberately does not "fix" the remaining single-word errors
with phrase-specific constants.  The original engine has a real feature-driven
prosody stage; those feature flags are the next reverse-engineering target.

## Pitch-only calibration check

A global TD-PSOLA pitch retune was tested against the same golden corpus.  A
0.90 pitch scale did **not** improve waveform parity (mean search correlation
fell slightly from about 0.1900 to 0.1893 and timing also moved away from the
reference), while 0.95 was worse.  Therefore M20 keeps pitch scale at 1.0 and
does not hide the remaining PSOLA mismatch behind an unsupported global knob.
