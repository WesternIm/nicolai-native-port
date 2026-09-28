# M39: source-backed acoustic parameter screen (2026-09-28)

The first user A/B listen did not hear a meaningful difference between the
ordinary port and M38. This checkpoint tests whether two existing, general
prosody controls or a simple spectral imbalance offer a better direction.
It does **not** change the stable renderer or ship another EXE.

## Inputs and limits

The same local 22-phrase original Nicolai SAPI reference pack and the same
voice-data hashes as M35 were used. The current M38-branch stable batch output
is byte-identical to the earlier stable pack. Original WAVs, text, binaries,
new candidate WAVs and full per-phrase reports remain in ignored local paths.
Only scalar conclusions and reproducible analysis code are committed.

The historical `measure_parity.py` F0 contour is a 12-bin YIN diagnostic; it
does not have ground-truth original voicing labels. The odd/even columns below
are a fixed phrase-ID split, not an independently collected test set. MFCC-DTW
and duration are also diagnostics, not listening scores. These limits make a
negative screen useful, but cannot prove an acoustic improvement.

## Controlled pitch trials

Each row changed only the listed timing-policy pitch values via the batch
renderer environment. Unlisted defaults, input text and voice data were the
same. Lower is better for all three metrics. The two split columns report the
mean per-phrase F0 error on odd and even IDs separately.

| Trial | Changed value(s) | F0 all / odd / even, % | F0 wins/losses vs stable | MFCC-DTW | Duration MAE, % |
| --- | --- | ---: | ---: | ---: | ---: |
| Stable | defaults | 13.48 / 14.87 / 11.93 | — | 52.01 | 3.60 |
| Earlier lift | declination mid 0.94 → 1.00 | 14.99 / 16.40 / 13.77 | 7 / 15 | 51.93 | 3.40 |
| Lower tail | declination end 0.78 → 0.70 | 13.89 / 14.30 / 13.50 | 8 / 14 | 52.42 | 3.63 |
| Lift and tail | both changes | 14.45 / 16.43 / 12.46 | 9 / 13 | 52.42 | 3.44 |
| Stress +5% | stressed-vowel boost 1.00 → 1.05 | 14.31 / 15.80 / 12.89 | 10 / 12 | 52.26 | 3.53 |
| Stress +10% | stressed-vowel boost 1.00 → 1.10 | 15.39 / 16.48 / 14.37 | 5 / 17 | 53.02 | 3.62 |

The aggregate median F0 trajectory initially suggested that the port needed
an early lift and lower ending. Testing whole phrases disproved this simple
three-point fit: every candidate increases the mean F0 error and loses more
phrases than it wins. The tiny MFCC/duration gains in two rows do not justify
promoting a pitch regression, especially with uncertain YIN voicing. These
settings remain rejected trials, not new acoustic presets.
The committed scalar summary is
[`m39-acoustic-screen-20260928.json`](metrics/m39-acoustic-screen-20260928.json).

## Spectral balance screen

`tools/audit_spectral_balance_m39.py` compares paired 16-kHz WAVs using
energy-gated 2048-sample frames. It reports gain-invariant median power ratios
for 3–7 kHz versus 0.3–3 kHz and spectral flatness, per phrase. Synthetic
tests cover high-band sensitivity, level invariance where the signal exceeds
the quantization floor, wrong-rate and short-input rejection.

Across 22 phrases, the port's high/low ratio is 1.12 dB higher on average,
0.56 dB higher at the median, and higher in only 12/22 phrases. Port spectral
flatness is higher in 16/22, with a mean delta of 0.0068. There are large
phrase-level swings in both directions. These utterance-level statistics are
not phone-aligned and do not measure perceived breathiness; importantly, they
do **not** support a blanket high-frequency boost or generic added noise.

## Original feature-capture boundary

The guarded x86 `run_m36_runtime_capture.ps1` was retried against the installed
DLL whose SHA-256 still matches M35's pinned image. SAPI warm-up timed out at
`set-rate` before synthesis. Diagnostic trials that skipped SetRate then timed
out at `set-volume`, and one skipping both timed out at `set-stream`. The
temporary switches were removed. This local evidence points to original-engine
initialization on the first operational COM call, not a rate-specific fault;
it does not establish why the server fails or supply original feature records.
The bounded launcher stopped only its owned trigger processes. No installed
ELAN files, registry entries or user WAVs were modified.

## Decision and next target

Keep the stable profile unchanged and M38 opt-in. Do not turn up declination,
stress, EQ or noise globally. For a meaningful intonation/"breath" correction,
obtain original phone-feature pitch/voicing/energy records or a reliable
independently labeled original F0 track, then test a phone-local intervention
on held-out phrases and by listening. In parallel, isolate the SAPI server's
first-call startup failure without changing the installed voice.

## Reproduce locally

Use the ignored local voice data and original reference pack. For each trial,
render 22 WAVs with `nicolai_batch_render` and exactly one or two of
`NICOLAI_PITCH_DECLINATION_MID=1.00`,
`NICOLAI_PITCH_DECLINATION_END=0.70`, or
`NICOLAI_PITCH_STRESSED_VOWEL_BOOST=1.05/1.10`; never combine the stress
trials with the declination trials. Run `tools/measure_parity.py` for each
candidate against the same reference pack and retain its private JSON locally.
The spectral screen is directly reproducible with:

```powershell
python tools/audit_spectral_balance_m39.py `
  C:\path\to\reference_pack C:\path\to\stable_port_wavs `
  --output metrics-work\new-spectral-audit.json
python tools/test_spectral_balance_m39.py
```

The audit refuses to overwrite an existing report and writes only scalar
values plus input hashes. Do not commit the reference/candidate WAVs or their
source text.
