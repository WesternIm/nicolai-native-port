# M36 A/B comparison launcher

The repository now has a one-click Windows entry point:

```bat
run_ab_compare.bat
```

The batch file delegates to `tools/run_ab_compare.ps1`. The PowerShell runner is
responsible for building the renderer, generating the exact 22-phrase corpus
from the PC reference manifest, rendering baseline/candidate WAVs and producing
ready-to-inspect parity reports.

## Inputs

The runner needs:

- an extracted PC `reference_pack_*` directory containing `manifest.json` and
  the 22 golden WAVs;
- a Nicolai voice directory containing `nicolai16.dat`, `exc_rus.txt` and
  `abb_rus.txt`.

Paths can be supplied explicitly:

```bat
run_ab_compare.bat -ReferencePack C:\path\to\reference_pack_20260924_222948 -VoiceDir C:\path\to\nicolai -CandidateProfile m34-shared
```

or through `NICOLAI_REFERENCE_PACK` / `NICOLAI_VOICE_DIR`. Without explicit
paths the runner checks a few local reference-pack locations and searches the
Program Files roots for the installed Nicolai voice assets.

## Candidate profiles

Current profiles:

- `m34-unit`: `NICOLAI_STATEFUL_TDS=1`, shared-phone duration off;
- `m34-shared`: `NICOLAI_STATEFUL_TDS=1`, shared-phone duration on;
- `m36`: reserved for the route-level M36 transition executor.

The `m36` profile deliberately refuses to run until
`NICOLAI_M36_TRANSITION_EXECUTOR` is actually wired into
`nicolai_batch_render`. This is a safety contract: an unknown environment
variable would otherwise be ignored and produce a fake stable-vs-stable
comparison.

Once the M36 renderer integration lands, the launcher infrastructure does not
need to change; the candidate profile only needs the synthesis path to honor the
reserved environment switch.

## Output bundle

Every run creates a timestamped directory under `metrics-work`, for example:

```text
metrics-work/ab-20260926-190000/
  baseline/                       stable WAVs
  candidate/                      candidate WAVs
  logs/                           renderer and metric stdout/stderr
  feature-cache/                  local M35 diagnostic cache
  corpus.tsv                      generated from the exact PC manifest
  baseline-parity.json/csv        historical M31 metric definitions
  candidate-parity.json/csv
  comparison.json/csv             phrase-by-phrase wins/regressions
  diagnostics-v2-2048.json        M35 shape/energy/pitch diagnostics
  diagnostics-v2-1024.json        pitch-window sensitivity run
  summary.json
  summary.txt
```

The historical headline table includes waveform correlation, active/total
duration MAE, legacy F0 contour MAE, MFCC-DTW and RMS-ratio MAE. The separate
M35 reports preserve the newer normalized spectral-shape, aligned energy and
pYIN/autocorrelation diagnostics rather than collapsing them into one score.

The runner opens the result directory in Explorer on success. Pass `-NoOpen` to
suppress that behavior.

## Why this exists before M36 audio integration

The recovered M36 source-selection/cross/window code is now large enough that
continuing static reconstruction without corpus feedback is inefficient. The
A/B runner freezes the measurement procedure before the new route-level renderer
is connected, so future corrections are compared through the same reproducible
pipeline rather than ad-hoc manual commands.
