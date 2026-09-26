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
run_ab_compare.bat -ReferencePack C:\path\to\reference_pack_20260924_222948 -VoiceDir C:\path\to\nicolai -CandidateProfile m36
```

or through `NICOLAI_REFERENCE_PACK` / `NICOLAI_VOICE_DIR`. Without explicit
paths the runner checks a few local reference-pack locations and searches the
Program Files roots for the installed Nicolai voice assets.

## Candidate profiles

Current profiles:

- `m34-unit`: `NICOLAI_STATEFUL_TDS=1`, shared-phone duration off;
- `m34-shared`: `NICOLAI_STATEFUL_TDS=1`, shared-phone duration on;
- `m36`: `NICOLAI_M36_TRANSITION_EXECUTOR=1`; `nicolai_batch_render` enables
  the established stateful/shared-phone caller and the stateful adapter then
  dispatches into the runnable M36 local transition experiment.

### Current M36 scope

The first runnable `m36` profile is intentionally narrower than the complete
static M36 reconstruction. It executes, per portable diphone descriptor:

- recovered initial-transition first/repeated grain geometry;
- recovered ordinary first/repeated grain source selection;
- recovered deferred-terminal single-grain/fade path;
- exact guarded M36 window-cache lookup;
- original Q15 writer and M36 post-write state rotation;
- the existing shared-phone upstream duration policy, so the comparison is not
  confounded by switching back to whole-diphone M34 timing.

It does **not** yet call the recovered nonzero cross-descriptor executor.
Static recovery proved that executor itself, but the present portable diphone
slice does not expose all caller step-buffer ownership/source context required
at the descriptor boundary. Inventing `deferred terminal == cross buffered
step` would create a plausible-looking but false state machine. Cross therefore
remains disabled until that caller/source ownership is captured or proven.

This means `m36` is now useful for directional A/B correction of local grain
selection/windows/terminal behavior, but its corpus score is not a claim of a
complete PC renderer.

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

## Recommended first comparison

Run:

```bat
run_ab_compare.bat -CandidateProfile m36
```

Use the result as a diagnostic checkpoint, not a promotion gate. The useful
question is whether local M36 grain/window behavior moves timing and normalized
shape away from the old stateful ~10.59% / ~39.50 direction and toward the
stable ~3.60% / ~37.19 reference without a global correction. If it regresses,
inspect phrase-level wins/losses before recovering more caller state.