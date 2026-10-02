# Nicolai Native Port — M36 acoustic checkpoint

For a ready-to-test Windows frontend, see [original PC voice and port in one
EXE](docs/WINDOWS_TEST_TALKER.md). The CI artifact `Nicolai-Test-win32` contains
the standalone test UI without proprietary voice data. Stable is the default;
experimental modes are clearly labeled, and original SAPI requires a working
installed Nicolai voice.

The opt-in [M50 noun е/ё filters](docs/M50_NOUN_YO.md) extend M49 with recovered
initialized noun-form membership, not per-word overrides. Six pronunciation
fixes are confirmed against fresh original traces, with no observed regression
on the previous aligned corpus. Full context/intonation remain unfinished;
M50 needs a second private data export and keeps older profiles unchanged.

The opt-in [M49 ending е/ё selector](docs/M49_ENDING_YO.md) extends M48 with
data-driven pronunciation choices, verified through the actual audio route.
Six new original phrases show 6 stress fixes and 9 е/ё fixes without observed
regressions; previous modes retain identical PCM. A small private policy export
from the user's matching original is required, never bundled or executed.
The opt-in [M48 ending-stress extension](docs/M48_ENDING_STRESS.md) adds bounded
ending vowel ordinals to M47, refusing unresolved е/ё and competing analyses.
The opt-in [M47 general lexical stress extension](docs/M47_LEXICAL_CANDIDATES.md)
adds the original exact-entry lane and supported stem-stressed adjective/short
forms, without per-word overrides. Captured original stress mismatches fall
from 5 to 2 on the previous aligned set (74 marked words), and from 21 to 7
on 52 marked words in new morphology controls. No pauses/F0/acoustic-parity
claim is made; stable and all previous profiles remain unchanged.

The optional [M46 original linguistic capture](docs/M46_ORIGINAL_LINGUISTIC_CAPTURE.md)
uses the same x86 EXE as its hidden debugger worker. `--original-trace` selects
original SAPI and requests four bounded, private intermediate snapshots.
M46b attaches the render's actual original server after SAPI output binding,
before text submission. Real captures now pass on 30 phrases with unchanged
original PCM. Zero-word service calls are preserved and counted separately.
Shared processes remain excluded. Normal profiles and audio defaults are
unchanged: this establishes an oracle, not a new acoustic improvement.

The diagnostic-only [M45 original phrase-boundary route](docs/M45_ORIGINAL_SPAN_ROUTE.md)
locates the raw separator producer in the original DLL and verifies 164
synthetic intermediate cases with an isolated x86 probe. It corrects the old
skip-flag interpretation to a morphology candidate count. No synthesis profile,
GUI package, pitch strength or timing default is changed by this checkpoint.

The opt-in [M44 original-lexicon checkpoint](docs/M44_LEXICON_STRESS.md) reads
63,294 original stem records plus suffix/stress tables from local EDAT inputs.
`m44-lexicon` inherits M43 and adds a conservative stem-stress noun/main-verb
subset: general forms such as `проверяем` and `делаем` no longer need individual
patches. Exact exceptions still win; ambiguous/unsupported analyses fall back.
Three of 22 saved-original comparisons improve in spectral shape, but pauses,
full morphology and phrase intonation remain unfinished. Stable is unchanged.

The opt-in [M43 word-rhythm/stress checkpoint](docs/M43_WORD_RHYTHM_STRESS.md)
redistributes each word's existing pre-join spoken budget and adds a limited
dictionary-backed fixed-stem `-ика` inflection fallback. `m43-word-rhythm`
selects 0.25 timing strength and corrects `акустику` to the lemma's stressed
`у`. Timing/click metrics remain mixed; a perceived emphasis on `с` in
`акустика` is not claimed fixed. Stable and older modes remain unchanged.

The opt-in [M42 join-pitch checkpoint](docs/M42_JOIN_PERIOD_CONTINUITY.md)
corrects internal run contour coordinates and partially reconciles guarded
shared-phone pitch endpoints. It inherits M41 and is selectable as
`m42-join-pitch`. Large PCM-step counts fall from 7 to 5 on 22 phrases, but
several metrics and additional cases worsen; this is a listening trial, not
a complete intonation/click fix or a stable promotion. Stable and M41 remain
byte-identical to their previous baselines.

The opt-in [M41 preservation checkpoint](docs/M41_SOUND_PRESERVATION.md)
protects threatened short features at unknown/unvoiced external joins and
uses fractional source sampling in uncovered PSOLA regions. Stable stays
unchanged. Mean MFCC-DTW improves slightly versus M40 (52.082 -> 51.881),
but corpus severe-step counts do not improve; listening validation is still
required. The test EXE exposes `m41-preserve` alongside the older profiles.

The corrected opt-in local M36 renderer reduces MFCC-DTW from 75.3109 to
71.2582 against the same 22 original WAVs (19 phrase improvements). Caller
step-slot rotation, dropped-source markers and cross source coordinates were
corrected; exact paths and counted fallback writes stay transactional.

A separate `m36-chain` A/B now exercises descriptor-owned nonzero crossings,
but scores 71.5357 and does not beat corrected local. Neither experiment beats
stable spectral shape or timing, so production remains unchanged and the
stable 22 WAVs are SHA256-identical to the earlier checkpoint.

See [acoustic evidence and reproduction](docs/M36_CHAIN_ACOUSTICS.md),
[scalar metrics](docs/metrics/m36-chain-acoustic-20260927.json), and
[corrected caller bookkeeping](docs/M36_CALLER_BOOKKEEPING.md).
No proprietary inputs or WAVs are committed.

The [next route-capture checkpoint](docs/M36_ROUTE_CAPTURE.md) adds original
caller instrumentation, compiled-portable route auditing and bounded startup
diagnostics. Its contracts pass, but live capture is blocked at SAPI rate
initialization; this is not a new acoustic improvement or PCM-parity result.

---

# Nicolai Native Port — M35

M35 audits the measurement and execution clocks without changing synthesis.
The M34 duration target itself is already too long: shared-phone target plus
flush has 10.6806% total-duration MAE; actual output has 10.5941%. The writer
differs from its requested duration by only 2.47 ms per phrase on average.
This localizes the dominant drift upstream of writing, without claiming the
PC feature builder or missing rollback is now reproduced.

A separately versioned diagnostic measures pYIN F0/voicing, independent
autocorrelation, gain-normalized spectral shape and aligned raw energy.
Known-tone, harmonic-chirp, noise/voicing, gain and octave controls pass,
but real-speech F0 coverage remains low: baseline matches only 29.2–33.0%
of reference pYIN-voiced frames across the two tested window lengths.
These conditional F0 scores are **not a promotion gate or speech ground truth**.

All 20 portable tests and 6 voice-free metric contracts pass. All six
22-WAV settings are byte-identical to M34 (132/132); the original-DLL
primitive probes also pass again. Production defaults and physical [l]/[e]
strengths remain unchanged. No phrase-specific rules or global timing fit.

See [M35 findings](docs/M35_FINDINGS.md),
[metric and clock report](docs/M35_PC_PARITY_REPORT.md),
and [reproduction / M36 continuation](docs/M35_CONTINUATION.md).
Only code, documentation, scalar metrics and hashes are tracked; voice data,
DLLs, WAVs and feature caches remain local.

---

# Nicolai Native Port — M34

M34 adds a stateful experimental execution adapter and proves the PC Q15
window writer (1024 original-DLL matches), reciprocal interpolation (400) and
sequential Q11 carry (1000). Shared-phone duration/energy and the three-point
F0 contour remain live. Tracing corrects 0x1010d330 to descriptor packaging;
coefficient production, source selection/rewind and exact window generation
are explicitly distinguished from the proven primitives.

Six settings were run on all 22 PC golden WAVs; all 20 host tests pass.
The stable baseline is SHA256-identical to M33, 22/22. Experiments stay
disabled: their raw duration errors are larger. A calibration audit also
shows that historical F0 ranking is sensitive to its 150-Hz ceiling,
whole-WAV RMS includes silence, and MFCC responds to level changes.
Historical metrics are retained, not presented as perceptual ground truth.

See [M34 findings](docs/M34_FINDINGS.md),
[parity and metric audit](docs/M34_PC_PARITY_REPORT.md),
and [reproduction / M35 continuation](docs/M34_CONTINUATION.md).
Sources, tests, reproducibility tools and metric artifacts are tracked;
proprietary DLLs/voice data and all WAVs remain local.

---

# Nicolai Native Port — M33

M33 recovers the original PC source-node timeline and Q11 synthesis-step
arithmetic, checked directly against the local original 32-bit DLL:
**2665/2665 diphone timelines** and **683/683 step records** match. The source
timeline has N+2 nodes, 10-ms unvoiced slots, signed-period node voicing flags,
and a split+1 phone boundary; M15's source-coordinate estimates were different.

An opt-in timeline adapter preserves M32's phone-local duration/energy and
three-point F0. Seven execution/physical-prosody settings were evaluated on
the same 22 PC golden WAVs. The adapter improves MFCC-DTW on all 22 phrases,
but worsens aggregate F0 parity, so it is **not enabled in production**.
Phase-search removal also fails the multi-metric gate. Stable audio remains
byte-identical to M32, 22/22 SHA256 checks; all 20 host tests pass.

See [findings](docs/M33_FINDINGS.md), [parity report](docs/M33_PC_PARITY_REPORT.md),
and [reproduction / M34 continuation](docs/M33_CONTINUATION.md). Exact primitives
are distinguished from the still-incomplete stateful Windows half-phone join.
Source, tests, tools, and metric JSON/CSV are tracked; DLLs, voice data, and WAVs
remain local.

---

# Nicolai Native Port — M32

M32 fixes the execution-side granularity that kept M31's recovered
`physical.int` duration (`[l%d]`) and energy (`[e%d]`) values disabled. The
portable renderer now consumes those values on the left/right phone support of
each diphone instead of averaging them over the complete unit, and the
piecewise-duration path retains M24+'s start/middle/end F0 contour.

The zero-effect path dispatches to the established renderer exactly; 20/20 host
tests pass, including byte-identical fallback, live three-point F0 under
asymmetric duration, and side-local energy checks. The original local voice
inputs were restored for evaluation and 39 unique coarse, micro, and fine-grid
settings were measured against all 22 PC golden WAVs. No non-zero duration or
energy setting produced a robust multi-metric win, so production deliberately
keeps both strengths at **0.0** and remains byte-identical to M31.

Reproducibility is now first-class: `tools/measure_parity.py` computes waveform,
active/total duration, F0 contour, MFCC-DTW, and RMS/energy parity in one report;
`tools/run_m32_parity.ps1` and `tools/sweep_m32_physical.ps1` reproduce the
candidate runs without committing golden WAVs or proprietary voice data.
`tools/compare_parity.py` audits candidate changes phrase by phrase. Committed
JSON/CSV summaries live in `docs/metrics/`.

See `docs/M32_FINDINGS.md`, `docs/M32_PC_PARITY_REPORT.md`, and
`docs/M32_CONTINUATION.md`.

---

# Nicolai Native Port — M31

M31 moves one layer beyond M30 pitch authoring and reconstructs the Windows engine's sibling **`[l%d]` duration** and **`[e%d]` energy** writers from the same `physical.int` authoring pass. Static tracing of the supplied `mtsyc32.dll` shows that source feature byte `+0x1d` is sign-extended into runtime duration state and consumed as an exact signed-percent correction (`base * (1 + l/100)`), while `+0x1c` carries the companion energy target.

The two 5-byte length profiles (`physical.int` bytes 3..7 and 8..12), the 5-byte energy profile (13..17), their `> / << / <<<` selector branches, terminal additions, and the multiplicative energy decline are now represented by tested portable helpers. M31 also exposes the lower-level dynamic authoring-span splitter recovered at `0x10216c70`: an effective-item count, midpoint-nearest empty-code candidate selection, the `>=5` space separator pass, and the `>6` underscore separator pass that writes the harder `(/)` boundary.

Controlled projection of the exact `[l]`/`[e]` values onto the current whole-diphone renderer was tested against all 22 PC golden WAVs. Non-zero duration strengths quickly regress waveform correlation/active-duration MAE, while energy blending gives no robust multi-metric gain. Production M31 therefore keeps both new strengths at **0.0**, preserving M30 audio **byte-for-byte 22/22** while materially increasing the amount of original PC prosody logic implemented and unit-tested. All **20/20 host tests** pass and **22/22** golden phrases render.

See `docs/M31_REVERSE_FINDINGS.md` and `docs/M31_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M30

M30 reconstructs the Windows engine's **authoring-span state** around `mtsyc32.dll:0x10214f40`.  The physical pitch writer is not per-word: it scans to a literal `(/)` boundary, uses the physical class from the **span-ending** item for the whole span, counts marker-bearing words across that span for the `physical[18]`/`physical[19]` choice, and interpolates pitch with span-wide vowel ordinals/counts.

The same reverse pass proves automatic runtime insertion of `<<` when a span has `>` but no `<`, identifies an upstream fallback-marker normalizer, and finds the legacy dynamic `(/)` splitter for long authoring runs.  M30 does not guess the still-incomplete long-span split policy; the supplied 22-phrase golden corpus does not require it.

After correcting the scope of the already recovered ordinary-`<` writer, a fresh corpus sweep now gives a real acoustic win. Production enables `physical_single_marker_pitch_strength=0.002`: mean active waveform correlation improves **0.195398 -> 0.196516**, optional 12-coefficient MFCC-DTW mean distance improves **128.804 -> 128.590**, and active-duration MAE improves slightly **5.6830% -> 5.6790%**. Normalized F0-trajectory MAE changes only **13.4680% -> 13.4799%**, so the selected point is explicitly a multi-metric acoustic tradeoff rather than a fake all-green scorecard.  No phrase-specific rules are used.

All **20/20 host tests** pass and **22/22** golden phrases render.  See `docs/M30_REVERSE_FINDINGS.md` and `docs/M30_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M29

M29 closes the ordinary orthographic stress-marker (`<`) writer around `mtsyc32.dll:0x10214f40`. Static CFG/dataflow proves that the apparent k1/k3 selector is dead in the supplied Nicolai binary: the live path always uses **k1 = 22/29/36**. The interpolation inputs are also corrected from M28's diagnostic marker ordinals to the PC writer's real state: **current vowel ordinal / total vowel count**, using the embedded CP866 vowel set.

The exact `wordstr[33]` bias is recovered as `trunc(-0.5*wordstr[33])` (Nicolai: 20 -> -10), followed by the existing byte-18/19 base selection, `record[20]` target and legacy `_ftol` interpolation. Portable M29 implements this as `legacy_physical_single_marker_triplet_pc()` and adds independent diagnostics through `NICOLAI_PHYSICAL_SINGLE_MARKER_PITCH_STRENGTH`.

Golden-corpus tests show a real tradeoff: a tiny non-zero blend can raise raw waveform correlation while slightly worsening F0 trajectory, or improve F0 while lowering correlation. M29 therefore ships the new blend at **0.0** rather than selecting a corpus-specific micro-optimum. A zero-strength isolation bug found during testing was also fixed, so the 22 production WAVs are **byte-identical to M28**. All **20/20 host tests** pass and **22/22** golden phrases render.

See `docs/M29_REVERSE_FINDINGS.md` and `docs/M29_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M28

M28 resolves the original Windows **orthographic pitch-marker selector topology** around `mtsyc32.dll:0x10214f40`.  The supplied MSI's `exc_rus.txt` proves that the second `<` follows the stressed vowel and can become literal `<>` for a word ending on its stressed vowel.  Static tracing then proves that `[t%d]` is inserted exactly at the marker index and parsed onto the preceding Russian letter's 0x20-byte feature record.

The physical-table selector is now mapped as k0=`21/28/35`, k1=`22/29/36`, k2=`23/30/37`, k3=`24/31/38`, k4=`25/32/39`, k6=`27/34/41`; k5 (`26/33/40`) is not referenced by the t-writer.  The start/base choice is also recovered: marker-bearing word count `<= wordstr[24]` (2.0) uses byte 18, otherwise byte 19.  Exact `<>` semantics are implemented for first non-final (k0), later non-final (k2 + PC interpolation), and final-global (k4 + byte 20) markers.

A controlled non-terminal `<>` blend was tested against all 22 PC golden WAVs.  Small strengths trade waveform correlation against F0 error rather than improving both, so production M28 keeps that new blend at zero and preserves M27's validated terminal strength.  Consequently the 22 production WAVs are **byte-identical to M27**, while the reverse model and diagnostic path are materially more exact.  All **20/20 host tests** pass and **22/22** golden phrases render.

See `docs/M28_REVERSE_FINDINGS.md` and `docs/M28_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M27

M27 ports the proven part of the original Windows **physical-prosody classifier** and corrects an important M26 table interpretation. `physical.int` bytes 21..41 are not seven uniformly spaced pitch nodes: they are three lanes of seven **context additives**, combined with start/end values in bytes 18..20 by the exact interpolation helpers at `mtsyc32.dll:0x10215d30/0x10215d90`.

The sentence classifier at `0x10215de0` is now represented explicitly: WH questions -> class 3, non-WH questions -> legacy 7/8 family (portable fallback 8 until the old POS byte is recovered), exclamation/other explicit closure -> 4/5, declarative closure -> 1 unless the proprietary `<<<` marker selects 2, and direct internal comma/colon branches -> 12/10. The recovered WH vocabulary comes directly from the three CP866 tables embedded in `mtsyc32.dll`.

Most importantly, M27 isolates one **fully recovered pitch authoring node** from the still-incomplete interior table: the final t-triplet is `record[20] + record[25/32/39]`. Only that exact terminal node is enabled in production, at a conservative 1.6% blend; the interior physical-table strength remains zero.

Against the same 22 PC golden WAVs, mean active waveform correlation improves **0.19433 -> 0.19540**, F0-trajectory MAE **13.4901% -> 13.4680%**, active-duration MAE **5.6908% -> 5.6830%**, and total-duration MAE **3.6030% -> 3.5969%**. All 22 phrases render and 20/20 host tests pass. No phrase-specific rules are used.

See `docs/M27_REVERSE_FINDINGS.md` and `docs/M27_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M26

M26 moves the original Russian **physical prosody table** from reverse-engineering notes into the portable core. The supplied Nicolai MSI was unpacked and used alongside `mtsyc32.dll` and the embedded `nicolai16.dat`: `rusvox\data\physical.int` is now parsed directly from the voice database as the exact **14 x 42 signed-byte** image loaded by the Windows engine.

Reverse engineering also closes the upstream chain that M25 was missing: `0x1019e530` writes `[t%d]` annotations into the three source pitch fields at `+0x10/+0x14/+0x18`; `0x10214f40` authors those annotations from `physical.int`; `0x10215de0` assigns physical classes 0..13 from sentence type, interrogative lexemes and punctuation. The class table then flows through the already recovered M25 sparse-anchor transport to runtime `+0x190/+0x194/+0x198`.

M26 deliberately keeps the production audio defaults **bit-identical to M25**. A conservative direct blend of the recovered table was measured against the same 22-PC-WAV golden corpus and regressed F0/correlation before all morphology-dependent class/branch selection is ported, so it is shipped as an opt-in diagnostic (`NICOLAI_PHYSICAL_PITCH_STRENGTH`) rather than silently making the voice worse. This is a reverse/parity milestone, not a fake metric bump.

New in M26:

- exact `physical.int` extraction from `nicolai16.dat`;
- validated 14 records x 42 signed bytes;
- portable `LegacyPhysicalProsodyProfile`;
- recovered physical-class writer path and sentence/question class evidence;
- diagnostic physical-pitch lattice and `nicolai_m26_probe`;
- 20/20 host tests pass;
- 22/22 golden phrases render; production default remains M25-equivalent until the remaining class/branch guards are reproduced exactly.

See `docs/M26_REVERSE_FINDINGS.md` and `docs/M26_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M25

M25 recovers the original PC engine's **sparse pitch-anchor transport and interpolation architecture**. The old engine stores optional three-point signed pitch percentages in 0x20-byte phonetic feature records, copies them into runtime `+0x190/+0x194/+0x198`, searches forward for the next explicit anchor and linearly bridges sparse anchors before converting them around Nicolai's 83-Hz pitch base.

The exact upstream linguistic rule that authors every source pitch triplet is still being recovered. A pure guessed sparse replacement regressed the golden corpus, so production M25 keeps M24's global contour as the backbone and mixes the recovered sparse architecture conservatively (5%) with **no phrase-specific hacks**. Against the same 22 PC-oracle WAVs, active-duration MAE improves 6.41% -> 5.69%, normalized F0-trajectory MAE 14.20% -> 13.49%, and total-duration MAE 4.02% -> 3.60%; mean raw waveform correlation is essentially flat at 0.19433 vs 0.19507.

See `docs/M25_REVERSE_FINDINGS.md` and `docs/M25_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M24

M24 restores the **three-point F0 contour representation** used by the original
Windows Russian prosody engine. `mtsyc32.dll` stores three pitch offsets for
each phonetic item at `+0x190/+0x194/+0x198` and converts each offset around
the 83-Hz `pitch.par` base. Portable TD-PSOLA can now follow a changing F0
inside a diphone instead of forcing one pitch ratio over the whole unit.

## M24 changes

- proves the PC per-item pitch triplet at `+0x190/+0x194/+0x198`;
- proves the upstream triplet is copied into the prosody record before the
  already recovered `base * (1 + 0.01*offset)` conversion;
- adds start/middle/end pitch interpolation to portable TD-PSOLA;
- maps the contour coherently through mixed SEG voiced runs;
- adds a corpus-wide falling-contour reconstruction while the exact upstream
  Russian triplet authoring rules are still being recovered;
- normalized 12-bin F0-trajectory relative MAE improves **18.49% -> 14.20%**;
- MFCC-DTW mean distance improves **2.2171 -> 2.2108**;
- 22/22 golden phrases render and 20/20 host tests pass.

Raw sample correlation is slightly lower than M23 because changing local F0
changes phase while join/duration parity is incomplete. The M23 acoustic pitch
path remains available by setting `NICOLAI_PITCH_DECLINATION_STRENGTH=0`.

See `docs/M24_FINDINGS.md` and `docs/M24_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M23

M23 restores the first data-backed part of the original Windows **pitch model**.
The PC Russian engine uses an 83-Hz runtime base (`pitch.par` / `+0x81264`) and
processes contour deviations around that centre instead of simply preserving
the recorded F0 of each database diphone.

## M23 changes

- proves the `pitch.par` parser at `mtsyc32.dll:0x10106880`;
- proves the built-in Nicolai/Russian pitch base is **83**;
- recovers `0x101a2250`: `base + (pitch-base) * wordstr[28]`, with `wordstr[28]=0.95`;
- recovers the independent pitch/base clamp to **0.90..1.00** at `0x101a2310`;
- proves the main PC prosody path stores percentage pitch offsets around the base (`0x102272c0`);
- adds per-diphone absolute-F0 centering to portable TD-PSOLA rather than a single global pitch multiplier;
- retunes the word-position timing contour from 0.25 to 0.075 after restoring pitch regulation;
- improves mean active waveform correlation **0.19247 -> 0.19938**;
- improves median portable/reference F0 ratio **1.0959 -> 1.0183**;
- reduces mean absolute F0-ratio error **10.16% -> 7.90%**;
- 22/22 golden phrases render and 20/20 host tests pass.

The one remaining M23 calibration term (`5/19` SEG-deviation retention before
the exact 0.95 PC transform) stands in for the still-unrecovered upstream
contour-event generator.  It is corpus-wide and has no phrase-specific cases.

See `docs/M23_FINDINGS.md` and `docs/M23_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M22

M22 is the fourth PC-reference conformance milestone.  It resolves the legacy
previous/next phoneme walkers and the CP866 symbol rules in `mtsyc32.dll`, adds
portable diagnostics for those rules, and keeps production audio bit-identical
to M21 because the tested ungated approximations all measured worse against the
Windows golden corpus.

## M22 changes

- identifies `0x102381d0` / `0x102382d0` as previous/next phoneme-symbol walkers;
- corrects `0x88/0x89/0x90/0x93` to CP866 И/Й/Р/У rather than abstract feature IDs;
- maps the relevant portable Nicolai phone labels to those recovered symbols;
- adds exact neighbour-pattern diagnostics to `nicolai_batch_render`;
- documents the recovered consonant/sonorant and voicing-pair class tables;
- documents the default feature-switch state and the still-unresolved `0x400` / `0x800` record bits;
- rejects coarse class-wide retiming, direct SEG split retiming, and a global phone-scale retune after golden-corpus measurement;
- 22/22 production WAVs are byte-for-byte identical to M21; 20/20 host tests pass.

See `docs/M22_FINDINGS.md` and `docs/M22_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M21

M21 is the third PC-reference conformance pass.

## M21 changes

- consumes the recovered `wordstr.par` 0.90 -> 0.85 -> 1.00 utterance-position contour;
- applies the contour conservatively (strength 0.25) and normalized across words;
- adds the PC-like terminal quiet flush inferred from the golden corpus and the recovered 150 ms `wordstr.par` timing quantum;
- total-WAV duration MAE improves from **24.39% to 3.06%**;
- mean absolute trailing-silence error improves from **306.4 ms to 16.4 ms**;
- active-duration MAE improves slightly from **5.437% to 5.373%**;
- mean active alignment correlation improves from **0.1900 to 0.1925**;
- 22/22 golden phrases render; 20/20 host tests pass.

See `docs/M21_FINDINGS.md` and `docs/M21_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M20

M20 is the second conformance pass against the original Windows **ELAN TTS Russian (Nicolai 16Khz)** reference pack.

## M20 changes

- PC-reference corpus is treated as the golden target; the portable core is tuned toward it.
- Internal phone timing and word-boundary timing are now separate policy layers.
- Ordinary inter-word boundaries are no longer left at full raw database duration.
- Frontend preserves comma / punctuation / hyphen boundary type without changing acoustic phone IDs.
- `USB -> У-Эс-Бэ` hyphens are distinguished from punctuation dashes.
- 22/22 golden-reference phrases render; 20/20 host tests pass.
- Mean active-duration ratio improves from **1.108 (M19)** to **0.994 (M20)**.
- Mean absolute active-duration error improves from **20.2%** to **5.4%**.

See `docs/M20_FINDINGS.md` and `docs/M20_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M19

M19 is the first portable-core release calibrated against a real reference pack from the installed original 32-bit SAPI5 **ELAN TTS Russian (Nicolai 16Khz)**.

## M19 changes

- PC-reference output gain stage (`2.0x`, saturating PCM16) kept separate from G.711/PSOLA math.
- Builtin stress correction for `э́то`, fixing the PC-corpus regression `missing_diphone_#_e4`.
- Batch reference-pack comparator (`tools/compare_reference_pack.py`) with active-speech timing, level and alignment metrics.
- 22/22 phrases from the supplied PC reference corpus now render in the portable core.
- 20/20 CTest tests pass.

The remaining large parity gap is predominantly **timing/prosody/PSOLA**, not simple output level. See `docs/M19_PC_PARITY_REPORT.md`.

---

# Nicolai Native Port — M18 PC-compatibility milestone

M18 moves the portable Nicolai port from an independent Russian frontend toward
**legacy PC SpeechCube compatibility**.  It still does not claim bit-exact
Windows parity because the original x86/SAPI engine cannot be executed in the
current build environment.  Instead, M18 consumes the original data and parser
layouts that were recovered from the supplied SpeechCube 5.1 database and DLL,
and ships an explicit Windows-reference parity harness.

## What changed since M17

- full `exc_rus.txt` pronunciation replacements, not stress-only lookup;
- multi-word exceptions (the supplied dictionary contains 324);
- `abb_rus.txt` CP1251 abbreviation expansion;
- deterministic cardinal integer expansion after legacy abbreviations;
- exact embedded `duration.par` extraction: 64 Nicolai phone durations;
- embedded `wordstr.par` extraction: all 35 legacy values are preserved for
  ongoing prosody parity work;
- duration-controlled hybrid TD-PSOLA now uses the legacy phone-duration table;
- Android JNI can load both legacy dictionaries;
- arbitrary-text `nicolai_render` host CLI;
- Windows SAPI reference-capture script and WAV comparison tool.

The proprietary `nicolai16.dat`, `exc_rus.txt`, and `abb_rus.txt` are **not
included** in this source archive.

## Current portable pipeline

```text
UTF-8 Russian text
  -> legacy abbreviation expansion (abb_rus.txt)
  -> legacy full exception/pronunciation expansion (exc_rus.txt)
  -> integer normalization fallback
  -> stress-aware Russian frontend
  -> Nicolai phone sequence
  -> exact embedded duration.par targets
  -> AXM diphone lookup
  -> SEG voiced/unvoiced grammar
  -> ANA G.711 A-law -> PCM16 @ 16 kHz
  -> SEG-driven TD-PSOLA / Hann OLA
  -> PCM16
```

## Confirmed embedded PC data

The original `duration.par` has been recovered as 64 exact phone durations. A
few examples:

```text
a0 = 100 ms
a1 =  65 ms
a3 =  50 ms
a4 =  55 ms
m  =  70 ms
p  =  80 ms
sh = 110 ms
sc = 150 ms
```

The original `wordstr.par` object has also been recovered as 35 floats.  M18
preserves it but does not invent names for fields whose exact legacy semantics
have not yet been proven.

## Build

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Historical M18 host result at that milestone: **19/19 tests passed**.

## Render arbitrary text

```bash
./build/nicolai_render \
  /path/to/nicolai16.dat \
  /path/to/exc_rus.txt \
  /path/to/abb_rus.txt \
  /tmp/out.wav \
  "Привет, мама!"
```

## M18 probe

```bash
./build/nicolai_m18_probe \
  /path/to/nicolai16.dat \
  /path/to/exc_rus.txt \
  /path/to/abb_rus.txt \
  /tmp/m18-out
```

On the supplied resources the probe indexed:

```text
stress entries:       94,596
exception entries:    95,073
multiword exceptions:    324
abbreviations:           212
duration entries:         64
```

## Windows PC parity

See `docs/PC_PARITY.md`.

On a Windows installation with the original Nicolai SAPI voice:

```powershell
powershell -ExecutionPolicy Bypass -File tools/windows_reference.ps1 `
  -Text "Привет, мама!" `
  -OutWav .\reference.wav
```

Then compare it with the portable output:

```bash
python tools/compare_wav.py reference.wav portable.wav
```

This is the intended path to exact PC parity: every mismatch becomes a
measurable normalization/prosody/acoustic target instead of a by-ear tweak.

## Status

The acoustic database format, A-law decoding, SEG schedule, diphone addressing,
and portable synthesis path are working. M18 additionally uses several original
PC Russian resources. Exact PC identity is **not yet certified** because the
legacy Windows engine has not been run side-by-side in this environment.
Remaining known parity targets are sentence-level punctuation/intonation,
`digit1..4.ini` number grammar, full rusvox morphology/rule parity, and physical
Android/ARM64 execution verification.
