# M14 findings — SEG pitch marks and portable TD-PSOLA

M14 is the first milestone that performs pitch-synchronous grain resynthesis on
real Nicolai PCM. It does not execute any x86 SpeechCube code.

## 1. Clean voiced SEG records expose a deterministic period sequence

Several fully voiced Nicolai diphones share the same verified shape:

```text
[2, N, split, N, p0, p1, ... p(N-1)]
```

Examples from the supplied `nicolai16.dat`:

```text
m->a0   [2,14,5,14, 150,152,152,153,152,152,154,154,154,153,154,156,157,159]
a0->m   [2, 9,5, 9, 174,180,184,190,193,194,201,208,217]
a0->a0  [2,10,5,10, 172,173,175,176,174,175,172,171,170,171]
m->m    [2,16,8,16, 173,173,172,173,172,172,173,172,173,173,171,172,171,171,169,170]
```

For these records the number after the four-word header exactly matches the
number of following positive period values. Mixed/unvoiced records use signed
control tokens and are deliberately not treated as clean voiced schedules.

The period chain fits inside the decoded A-law waveform with small edge margins.
M14 centers the chain in the source waveform and obtains explicit source pitch
marks by cumulative summation of the SEG periods.

## 2. Legacy Hanning math recovered from `mtsyc32.dll`

The legacy Hanning builder around VA `0x1010A020` uses x87 operations whose
constants decode to:

```text
pi      = 3.141592653589793
half    = 0.5
q15max  = 32767.0
```

The inner window expression is therefore:

```text
0.5 * (1 + cos(pi*x/P)) * 32767
```

for a descending half-window. `legacy_half_hann_q15()` implements that exact
mathematical primitive. The portable TD-PSOLA grain uses the corresponding
symmetric Hann shape.

For `P=160` the M14 probe obtains:

```text
first / midpoint / last = 32767 / 16384 / 0
```

## 3. Portable TD-PSOLA stage

For every clean voiced diphone M14 now:

1. decodes the exact ANA A-law slice to PCM16;
2. parses the clean voiced SEG period schedule;
3. creates pitch marks from the recorded period sequence;
4. extracts approximately two-period Hann-windowed grains centered on those
   marks;
5. places synthesis marks at pitch-controlled spacing;
6. overlap-adds and normalizes the grains;
7. independently supports duration scaling by mapping synthesis time back to
   source time.

This gives independent pitch and duration controls without resampling the
waveform, which is the core behavior expected from time-domain PSOLA.

## 4. Verified real Nicolai results

For the neutral `# -> m -> a0 -> m -> a0 -> #` prototype:

```text
m->a0: 14 periods, 15 source marks, 15 synthesis marks
       mean source period 153.714 samples
       mean target period 153.214 samples

a0->m: 9 periods, 10 source marks, 10 synthesis marks
       mean source period 193.444 samples
       mean target period 190.778 samples
```

With `pitch_scale = 1.20`:

```text
m->a0 mean target period: 127.552 samples
      ~16000/127.552 = 125.44 Hz
      versus ~104.09 Hz source-period mean

a0->m mean target period: 157.583 samples
```

The ~20% shorter synthesis period is the expected result of a +20% pitch
control while unit target duration remains nominal.

With `duration_scale = 1.25`, voiced units receive more synthesis grains while
mean target pitch period remains essentially unchanged.

## 5. Current boundary

M14 does **not** claim bit-exact reconstruction of the entire 1997 Tempo-PSOLA
engine. Specifically:

- the signed control grammar in mixed/unvoiced SEG records is not decoded yet;
- clean voiced periods are proven structurally, but the exact legacy meaning of
  every header word is not fully named;
- M13's phase-aligned boundary OLA is still used when concatenating neighboring
  diphone units;
- legacy prosody/linguistic duration targets are not connected yet.

The important change is that voiced waveform generation itself is now genuinely
pitch-synchronous and driven by Nicolai's own SEG period data rather than a
single boundary-period hint.
