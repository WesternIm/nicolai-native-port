# M15 findings — complete structural SEG grammar and mixed-unit rendering

## 1. Signed SEG grammar is structurally solved

Every one of the 2,665 Nicolai diphones validates against the same parser:

```text
word 0 : mode (+2 or -2)
word 1 : total slot count N
word 2 : diphone split index
then repeated runs until N slots are covered:

  signed_run_count
  if signed_run_count > 0:
      exactly signed_run_count pitch values follow
  if signed_run_count < 0:
      no pitch values follow

sum(abs(signed_run_count)) == N
```

Examples:

```text
m->a0
[2,14,5, 14, 150,152,152,153,152,152,154,154,154,153,154,156,157,159]

#->m
[-2,17,10, -9, +8, -104,105,149,171,172,198,200,220]

m->#
[2,20,5,
 +2, 238,254,
 -7,
 +6, -157,158,166,166,278,302,
 -5]
```

This parser consumes every metadata word with no trailing data and exactly
covers each record's declared slot count.

## 2. Database-wide validation

On the supplied `nicolai16.dat`:

```text
catalog units:       2665
SEG parsed:          2665
SEG laid out:        2665
all voiced:          1307
all unvoiced:         238
mixed:               1120
signed pitch resets:  426

run count histogram:
1 run  -> 1545 units
2 runs -> 1101 units
3 runs ->   17 units
4 runs ->    2 units
```

No record fails the structural grammar.

## 3. Negative pitch entries

A negative pitch value occurs only as the **first pitch value of a positive
(voiced) run**. There are 426 such occurrences.

For every occurrence with at least two pitch entries:

```text
abs(first_negative) - second_positive = 0 or +/-1 sample
```

Examples:

```text
-104, 105
 -80,  81
-157, 158
-143, 143
```

This strongly marks a voiced transition/reset boundary. M15 preserves this
fact explicitly as `signed_period_reset` and uses the magnitude as the period.
The exact historical Tempo name/meaning of the sign bit is not yet claimed.

## 4. Complete source-span layout

For voiced runs, the source duration contribution is the sum of absolute pitch
period magnitudes. For mixed records the remaining PCM samples are distributed
proportionally across the declared unvoiced slots. This gives a contiguous
partition of every decoded A-law unit:

```text
run[0].begin == 0
run[i].end   == run[i+1].begin
last.end     == PCM sample count
```

This succeeds for all 2,665 units.

For fully voiced records the residual edge slack is kept symmetric, preserving
the strong M14 edge-margin observation.

## 5. M15 hybrid renderer

M14 rejected signed/mixed SEG records and passed their entire decoded PCM to the
M13 boundary join. M15 instead renders each SEG run independently:

```text
voiced run
  -> one source pitch mark/grain per recorded period
  -> TD-PSOLA pitch/duration processing

unvoiced run
  -> duration-only short-time Hann OLA
  -> pitch_scale is not applied

run boundary
  -> raised-cosine OLA
```

The completed unit then enters the existing diphone-boundary OLA stage.
Therefore mixed records such as `#->m`, `p->a0`, and `a0->#` no longer use a
whole-unit raw fallback.

## 6. Pitch-mark correction relative to M14

M14 provisionally interpreted `N` recorded periods as `N+1` pitch marks. The
complete grammar establishes a stronger structural relationship: a `+N` run
contains exactly `N` pitch entries, one per voiced slot.

M15 therefore uses **N source grains/marks for N pitch entries**. This avoids an
extra synthetic boundary mark and also works naturally for voiced sub-runs in
mixed records.

## 7. Real probe results

The `# -> m -> a0 -> m -> a0 -> #` chain now processes every diphone through a
parsed SEG schedule. In the neutral render:

```text
#->m  runs [-9,+8] : unvoiced + voiced TD-PSOLA
a0-># runs [+6,-17]: voiced TD-PSOLA + unvoiced
```

The separate `# -> p -> a0 -> #` probe exercises fully unvoiced, unvoiced to
voiced, and voiced to unvoiced transitions in one chain.

Generated WAV validation shows no PCM16 clipping in the probe outputs.

## 8. Remaining limitations

M15 removes the major M14 fallback, but it is not claimed to be bit-exact Tempo:

- the exact semantic name of the negative first pitch entry remains unknown;
- unvoiced duration scaling uses short-time Hann OLA; exact legacy unvoiced regulator behavior is still unknown;
- the original Tempo duration/prosody regulator is not connected;
- Russian text -> phoneme/stress/prosody frontend is not connected;
- Android ARM64 execution still requires an actual NDK/device validation run.
