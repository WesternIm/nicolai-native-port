# M32 PC parity report

Golden target: original Windows **ELAN TTS Russian (Nicolai 16Khz)** reference
pack `reference_pack_20260924_222948`, 22 phrases.

## Stable M31 baseline

| metric | M31 |
|---|---:|
| rendered | 22/22 |
| host tests | 20/20 |
| mean active waveform correlation | 0.1965157232 |
| mean active-duration ratio | 0.9925498970 |
| active-duration MAE | 5.679022644% |
| total-duration MAE | 3.598674499% |
| normalized 12-bin F0 trajectory MAE | 13.479913522% |
| mean 12-coefficient MFCC-DTW distance | 128.589932660 |
| RMS-ratio MAE | 10.2377% |

## M32 validation completed in this handoff

- MSVC host tests: **20/20**.
- Parity implementation self-check: **22/22** reference WAVs measured against
  themselves; correlation 1.0, all duration/F0/RMS errors 0, MFCC-DTW 0.
- No golden WAVs or proprietary voice resources are committed.
- Production physical length/energy strengths remain **0.0 / 0.0**.

## Corpus run status

A real portable M32 22-WAV corpus was not synthesized in this handoff because
the local input set lacks `nicolai16.dat` (10,961,460 bytes), `exc_rus.txt`
(2,960,349 bytes), and `abb_rus.txt` (3,636 bytes). Those files were present in
the earlier reverse-engineering session but were correctly omitted from the M31
source archive.

No M32 improvement is claimed without that run. Restore the legally supplied
MSI or those three extracted files, execute `tools/sweep_m32_physical.ps1`, and
enable a non-zero strength only if the resulting committed scorecard shows a
robust multi-metric win over the M31 table above.
