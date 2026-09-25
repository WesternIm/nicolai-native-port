# M25 reverse findings — PC pitch-anchor transport/interpolation

M25 continues reverse engineering of the original Windows `mtsyc32.dll` and
uses `reference_pack_20260924_222948` only as the conformance target.

## Corrected data layout

`state + 0x16c` is **not** itself a pitch-triplet array. It is an outer array of
12-byte per-word descriptors. Each descriptor points to an inner array of
0x20-byte phonetic feature records.

Inside those 0x20-byte records, the source pitch fields are:

- `+0x10` — first signed pitch percentage
- `+0x14` — second signed pitch percentage
- `+0x18` — third signed pitch percentage
- `+0x1c` — signed anchor/current-pitch byte copied into the runtime word record
- `+0x1d` — an additional signed scalar/feature copied into the runtime record

At `0x10214da0..0x10214df3` (`0x10213150`), when the runtime feature flag
contains bit `0x400`, the engine copies the three 32-bit source values to the
runtime phonetic record at `+0x190/+0x194/+0x198`.

## Sparse-anchor interpolation

`0x10227630` searches forward through word/phone records for the next nonzero
pitch anchor. The central prosody pass converts signed percentages to absolute
F0 with the already recovered rule:

```
F0 = pitch_base * (1 + 0.01 * signed_percent)
```

For Nicolai `pitch_base` defaults to 83 Hz. A later stage (`0x101a2250`)
applies the exact `wordstr[28]` centering transform (0.95 for Nicolai).

When an explicit current anchor and a future anchor are separated by phonetic
items, the PC code bridges them linearly; sentinels `-1/-2` denote missing or
special pitch states. This establishes that the original contour is a sparse
anchor lattice, not one global F0 multiplier per diphone.

## Per-word call sequence

The recovered sequence around `0x10212df0` is:

```
0x101a2380(...)
0x101a22a0(state)
0x101a1dc0(state, runtime_pitch_base)
for each word:
    0x10213150(state, word_index)  # feature/pitch source -> runtime records
    0x102277c0(state)
    0x10226cf0(state)              # central duration/pitch prosody
    0x101a1dd0(word_index, state)  # post-prosody pitch/output stage
    0x101a2380(...)
```

`0x101a1dc0` simply stores the passed base pitch into the current pitch state.
`0x101a1dd0` consumes the runtime pitch records, handles `-1/-2`, calls the
0.95 centering transform, computes point-to-point deltas and emits output
commands.

## What is still unknown

The exact upstream linguistic rule that decides *which* 0x20-byte source
feature records receive explicit `+0x10/+0x14/+0x18` values is not yet fully
recovered. The basic feature-array constructor allocates/initializes these
records but does not explain all later pitch-authoring writes.

Therefore M25 implements the **recovered transport and interpolation
architecture** but keeps the M24 global declination as the dominant production
backbone. A general phrase-edge/stressed-vowel sparse lattice is mixed in at
only 5%. A pure replacement was tested and rejected because it worsened the
golden PC corpus. There are no phrase-specific exceptions.
