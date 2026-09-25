# M31 PC parity report

Golden target: original Windows **ELAN TTS Russian (Nicolai 16Khz)** reference pack `reference_pack_20260924_222948`, 22 phrases.

M31 implements exact `physical.int` `[l%d]` duration and `[e%d]` energy authoring arithmetic plus the lower-level dynamic authoring-span split primitive. The new execution-side projections are deliberately disabled by default because the portable renderer is still diphone-granular while the PC runtime consumes these features phone-locally.

## Production M30 -> M31

Production M31 keeps the new length/energy strengths at zero. A clean M31 render was SHA-256 compared against the M30 production corpus: **22/22 WAVs are byte-identical**.

| metric | M30 | M31 |
|---|---:|---:|
| rendered | 22/22 | **22/22** |
| host tests | 20/20 | **20/20** |
| mean active waveform correlation | 0.1965157 | **0.1965157** |
| active-duration MAE | 5.6790% | **5.6790%** |
| mean active-duration ratio | 0.9925499 | **0.9925499** |
| normalized 12-bin F0 trajectory MAE | 13.4799% | **13.4799%** |
| total-duration MAE | 3.5987% | **3.5987%** |
| mean 12-coefficient MFCC-DTW distance | 128.5899 | **128.5899** |

## Controlled `[l]` sweep

Representative experimental phone-to-diphone projection results:

| physical length strength | active corr | active-duration MAE |
|---:|---:|---:|
| 0.0000 | **0.1965157** | **5.6790%** |
| 0.0005 | 0.1965248 | 5.6790% |
| 0.0010 | 0.1961864 | 5.6839% |
| 0.0020 | 0.1953254 | 5.7021% |
| 0.0050 | 0.1952869 | 5.7063% |
| 0.0100 | 0.1914258 | 5.7422% |

The 0.0005 point is below a meaningful timing step in several WAVs and is not a robust cross-metric win; larger strengths regress.

## Controlled `[e]` sweep

| physical energy strength | active corr | active-duration MAE | RMS-ratio MAE |
|---:|---:|---:|---:|
| 0.000 | **0.1965157** | 5.6790% | **10.2377%** |
| 0.002 | 0.1965109 | 5.6790% | 10.2488% |
| 0.005 | 0.1965227 | 5.6789% | 10.2657% |
| 0.010 | 0.1965112 | 5.6789% | 10.2937% |
| 0.020 | 0.1964880 | 5.6789% | 10.3495% |

Again, no non-zero point gives a defensible multi-metric improvement. Production remains zero-strength.

## Interpretation

This is an architecture/reverse milestone rather than a fabricated audio bump. The original Windows writer is now known well enough to author the phone-local length and energy fields, but the portable execution path needs matching phone-local timing/energy consumption before those values should control production audio.
