# M37: initial-u coverage and acoustic triage (2026-09-28)

This checkpoint fixes one shared-frontend coverage bug, makes stable-port
joins measurable, and prevents mislabeled A/B saves. It **does not** change the
stable renderer's acoustic choices or claim better rhythm/intonation.

## Reproduced coverage failure

An initial unstressed у could select the interior remote phone `u4`, so a
phrase containing «улице» failed at `missing_diphone_#_u4`. The local voice
catalog contains `# -> u0` and `# -> u1`, but not `# -> u4`. The frontend now
selects `u1` at this boundary. Stressed initial у and disabled reduction still
select `u0`; interior selection is unchanged. This is a phonetic-position rule,
not a replacement for a particular word or phrase. The user's longer local
phrase now renders 165,362 samples. Its text and WAV remain private; neither
belongs in Git. The dictionary's fallback accent for unknown inflected words
is a separate potential mismatch and is not solved here.

All 22 stable WAVs of the existing UTF-8 A/B corpus were byte-identical to
their pre-change baseline. The added synthetic `street` case exercises initial
у in stable/M36 local/M36 chain and compares each CLI WAV against the batch
renderer. Fresh builds and tests are required after changing the result struct.

## Join telemetry and method

Stable `NicolaiTalker.exe --render` emits one `join=` record per diphone
overlap: shared phone index/label, center sample in final PCM, overlap length,
left/right trim, and normalized waveform correlation. The batch renderer
emits equivalent `J` records. M36 stateful paths do not report these OLA
joins. Telemetry is collected after the existing join decision; no selection,
timing, or PCM formula uses it.

`tools/audit_talker_joins.py` reads local original and stable WAVs plus the
stable render log, then reports 10 ms RMS intervals below
`max(60 PCM units, 2% of that WAV's maximum frame RMS)` for at least 30 ms,
bounded by its active samples. It lists each portable interval's nearest join.
This is a *localization aid*, not an alignment or a causality test. Thresholds
are the same rule but adapt to each WAV's own level. Keep the input WAVs and
logs outside Git. Example:

```powershell
python tools/audit_talker_joins.py `
  --original C:\path\to\original.wav `
  --portable C:\path\to\stable.wav `
  --join-log C:\path\to\stable-render.log `
  --output C:\path\to\new-audit.json
```

Three user-owned *short* original/port pairs were available locally. Fresh
stable renders were byte-identical to their old port WAVs; SHA-256 identities
and the scalar gap results are in
[`m37-gap-audit-20260928.json`](metrics/m37-gap-audit-20260928.json).

| Pair | Original/port length, ms | Quiet intervals | Quiet total, ms | Stable joins |
| --- | ---: | ---: | ---: | ---: |
| Startup | 4261.4 / 4205.4 | 9 / 13 | 1240 / 1370 | 46 |
| Short repetition | 1600.4 / 1432.2 | 2 / 4 | 220 / 270 | 13 |
| Short exclamation | 1028.8 / 1117.8 | 1 / 2 | 40 / 150 | 6 |

Many low-energy intervals sit near `к`, `т`, or word boundaries. Those phones
can contain legitimate closures; with 6–46 joins per short phrase, proximity
alone does not identify the source of unwanted breaks. The recordings are not
time/phone aligned. The original SAPI path has also been intermittent: some
user GUI jobs produced WAVs, while fresh independent jobs stalled at SetRate.

## Next acoustic boundary

Obtain repeatable original/port recordings for identical text with explicit
stress, align phone boundaries, and inspect only perceptually objectionable
gaps against reference closures. Capture original pitch/voicing and energy
features at the relevant boundary; current real-speech pYIN coverage is too
sparse to justify a blanket F0 correction. Then trial a renderer change behind
an experimental switch with duration, click/energy, and listening checks.
Do not apply a global duration multiplier or generic silence deletion, and do
not merge an unproven acoustic change into stable. Keep voice data and private
reference recordings local.
