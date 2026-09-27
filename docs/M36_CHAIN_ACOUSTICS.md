# M36 acoustic correction and multi-descriptor A/B — 2026-09-27

The corrected local experiment reduces historical MFCC-DTW from **75.3109 to
71.2582**, improving 19/22 phrases versus the earlier local checkpoint.
Normalized shape improves from 54.6811 to **51.8847**. Total-duration error is
unchanged at 10.5861%. This is progress inside the experimental branch, NOT a
production win: stable remains 52.0139 raw / 37.1928 normalized / 3.5987% timing.

## What was wrong

Re-auditing `0x10107c20` corrected three caller errors:

- ordinary/deferred steps swap/write once; they do not synchronize both slots;
- no dropped interval overwrites a buffered record or advances +0x2a;
- initial/cross perform an early rotation AND the common positive tail.

The local adapter had never updated the last-positive +0x2a marker after normal
success, and replaced it with the dropped index on failure to emit. This selected
the wrong source for subsequent dropped-bridge grains. It now uses the common
bookkeeping contract. Its simplified local initial writer is also transactional:
a late failure cannot leak exact PCM before compatibility fallback.

The cross source binding was wrong too: phase 1 reads **prev[k]**, not
prev[k+1]. Phases 3/4 use **cur[i+1]** even when i is terminal; only secondary
temporary support uses cur[i] at a terminal interval. Real previous descriptor
PCM ending at lastPosition+1 can therefore satisfy a deferred-tail crossing.
Tests now cover this without an invented extended PCM slice, and reject a
terminal current repeat that would read past its real PCM.

The old phone-feature oracle result (259/259 at `0x101a2780`) does not prove
these caller/PCM claims. They have static-dataflow and synthetic-contract
evidence; no live original route/PCM oracle is claimed.

## Separate chain experiment

`resynthesize_stateful_m36_chain_experimental()` accepts explicit descriptors,
keeps a common PCM cursor, buffers positive terminal records, and feeds the
previous/current PCM bases to the corrected nonzero cross route. Ordinary
success and drops retain the corrected caller marker/slot behavior. Exact
executors stage writes; unsupported requests fall back once to counted M34
compatibility writes. A late invalid unit discards the whole chain transaction.

`NICOLAI_M36_CHAIN_EXECUTOR=1` is opt-in and separate from the existing local
`NICOLAI_M36_TRANSITION_EXECUTOR=1` profile. The batch caller currently infers
cross-versus-flush from the previous unit's terminal voicing flag. This is an
experimental policy, not captured original caller gates. It starts unbuffered
descriptors through zero-cross rather than inventing an initial dummy record.

Still outside this adapter: complete dropped-tail rollback/gate composition,
the full two-record initial route, degenerate terminal-first route handling,
original metadata/event finalization, and exact uncached windows above 400.
Those limits and explicit compatibility paths block parity/promotion claims.

The 22-phrase chain run observed 80 successful exact nonzero crossings, 207
unbuffered starts, 88 terminal flushes, and 60 compatibility requests. The
corrected local run observed 287 local starts, 167 flushes and 1 fallback.
Telemetry is `M36 <id> <starts> <exact-crosses> <flushes> <fallbacks>` in stdout.

## Measurements

All profiles use the same original 22 WAVs, 16 kHz voice database and existing
upstream timing/pitch policy. Both new bundles reproduce the old stable WAVs
byte-for-byte (22/22 SHA256 checks). No global duration fit or phrase exception.

| profile | raw MFCC-DTW | normalized shape | total duration MAE | active duration MAE | RMS MAE |
| --- | ---: | ---: | ---: | ---: | ---: |
| stable | 52.0139 | 37.1928 | 3.5987% | 5.6790% | 10.2377% |
| prior M36 local | 75.3109 | 54.6811 | 10.5861% | 16.1316% | 8.5693% |
| corrected M36 local | 71.2582 | 51.8847 | 10.5861% | 16.3028% | 9.3265% |
| M36 chain | 71.5357 | 52.6415 | 10.5861% | 16.9311% | 10.1088% |

Chain is slightly worse than corrected local on raw AND normalized shape;
do not attribute the local improvement to cross integration. Timing remains
upstream-limited. Corrected local's RMS/active-duration errors worsen versus
the prior local checkpoint, so even that improvement is not a multi-metric win.
Both experiments regress raw MFCC versus stable on all 22 phrases.

pYIN remains diagnostic-only: corrected local pooled voiced coverage is 29.8%
(2048 frame) / 22.3% (1024); chain is 27.9% / 18.9%. Conditional F0 improvement
cannot compensate for missed frames or certify original pitch parity.

Scalar evidence is in `metrics/m36-chain-acoustic-20260927.json`. WAVs,
proprietary DLL/voice bytes and feature caches remain local.

## Reproduction

From a configured Visual Studio developer environment, with parity requirements
installed in the selected Python environment:

```powershell
.\tools\run_ab_compare.ps1 -CandidateProfile m36-chain `
  -ReferencePack C:\path\to\reference_pack -VoiceDir C:\path\to\Elan `
  -BuildDir build -Python .\.venv\Scripts\python.exe -NoOpen

.\tools\run_ab_compare.ps1 -CandidateProfile m36 `
  -ReferencePack C:\path\to\reference_pack -VoiceDir C:\path\to\Elan `
  -BuildDir build -Python .\.venv\Scripts\python.exe -SkipBuild -NoOpen
```

The runner now explicitly reads UTF-8 on PowerShell 5.1, distinguishes normal
native diagnostic stderr from nonzero exits, checks every WAV, rejects an
inactive byte-identical experiment and checks nonzero exact chain telemetry.
It requires fresh WAV directories, preserving old results without allowing a
failed phrase to be counted from stale output on a repeated run.
The earlier local bundle `ab-20260927-m36-chain` used mojibake input and is
INVALID; only the `*-utf8` bundles supplied the committed measurements.

Local verification: full rebuilt Debug x64 CTest 28/28; targeted Debug x86
contracts 4/4; six metric contracts; phone-feature contracts; original DLL
phone-feature oracle 259/259. GitHub CI must be checked separately.

Next: capture original descriptor flags/flush gates and dropped-tail rollback
at `0x10107c20`, then compare the real two-record source ownership against this
bounded chain. Do not enable either experiment in production until timing and
normalized shape beat stable together.
