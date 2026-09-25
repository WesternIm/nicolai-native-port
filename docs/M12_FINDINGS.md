# M12 findings — Nicolai ANA is direct G.711 A-law audio

M12 corrects the active audio path assumed in M11 and removes a large piece of
unnecessary reverse engineering from the Nicolai port.

No legacy x86 code is executed by the portable implementation.

## 1. M11's cmp16 branch exists, but Nicolai does not use it

The SpeechCube engine constructor copies the acoustic descriptor's coding field
(`nbr16aci.dsc + 0x30`) into the runtime engine object and only constructs the
special cmp16 decoder when that value is `0x50`.

Nicolai's descriptor stores:

```
nbr16aci.dsc + 0x1c = 16000   # sample rate
nbr16aci.dsc + 0x30 = 2       # signal coding
```

The cmp16-only descriptor path at the later acoustic-descriptor field used by
that branch is empty for Nicolai.  Therefore the cmp16/Huffman work recovered
in M11 describes a real SpeechCube codec family, but it is **not Nicolai's
active acoustic coding path**.

The M11 portable code remains in the tree as generic SpeechCube research and
for possible future voices.

## 2. Legacy type IDs map type 2 directly to A-law

A legacy formatting/diagnostic branch around VA `0x10017AC8` reads the 16-bit
signal-coding value and maps it to the original French labels.  The control
flow is:

```
value == 1 -> 0x106c5cd4 -> "loi mu"
value == 2 -> 0x106c5ccc -> "loi A"
value == 3 -> 0x106c5cc0 -> "lineaire"
```

The same binary also reports from `psola.c` that TDS accepts only linear,
A-law, or mu-law signal coding (plus the separate cmp16 special path in the
surrounding code).

Because Nicolai's coding field is `2`, its ANA signal is **G.711 A-law**.

## 3. ANA is one encoded byte per audio sample

The `.ana` loader in legacy `dicophon.c` simply opens the resource in binary
mode, allocates exactly its file size, and reads it into static memory.  It does
not run an entropy decompressor for Nicolai.

M10 already recovered exact diphone slices through:

```
phone pair -> AXM -> SEG -> ANA [start,end)
```

M12 now expands each ANA byte with the standard G.711 A-law rule into one
signed PCM16 sample.

The first boundary unit `# -> p` begins with many `0x55` / `0xD5` bytes.  Under
A-law these decode to -8 / +8, i.e. near-zero silence.  This is exactly what is
expected for the leading silence portion of a boundary-to-stop diphone.

## 4. Real Nicolai waveform statistics

Portable C++ results from the supplied database at 16,000 Hz:

| Diphone | Encoded bytes / PCM samples | Duration | PCM min | PCM max | RMS |
|---|---:|---:|---:|---:|---:|
| `# -> p` | 1,759 | 109.938 ms | -8 | 24 | 8.036 |
| `# -> m` | 2,994 | 187.125 ms | -3,392 | 7,040 | 1,499.844 |
| `m -> a0` | 2,314 | 144.625 ms | -7,808 | 12,544 | 3,078.087 |
| `a0 -> a0` | 1,900 | 118.750 ms | -8,448 | 17,920 | 3,783.378 |
| `p -> a0` | 2,865 | 179.062 ms | -10,496 | 16,896 | 3,056.226 |
| `o0 -> a0` | 2,566 | 160.375 ms | -8,064 | 15,104 | 4,123.901 |

The boundary unit is effectively silence while voiced/vowel units carry normal
speech-scale energy.  This independently matches the recovered coding enum.

## 5. First native audio artifact

`nicolai_m12_probe` decodes the real `a0 -> a0` diphone and writes it as:

```
PCM16 mono, 16000 Hz, 1900 samples, 118.75 ms
```

This is the first **actual Nicolai waveform** recovered from `nicolai16.dat` by
our portable code without calling any x86 SpeechCube DLL.

It is a raw diphone, not synthesized text.  Proper speech still requires the
legacy unit-selection / prosody / PSOLA joining logic.

## 6. Portable implementation added

M12 adds:

- `include/nicolai/g711.hpp`
- `src/g711.cpp`
- `tests/g711_test.cpp`
- `tools/nicolai_m12_probe.cpp`

The recovered coding IDs are represented as:

```
1    -> MuLaw
2    -> ALaw
3    -> Linear
0x50 -> Cmp16 (special SpeechCube branch)
```

The A-law implementation is architecture-independent C++17 and therefore
suitable for the future ARM64/Android build.

## Next milestone

M13 no longer needs to reverse-engineer a Nicolai entropy codec.  The next
problem is the actual PSOLA synthesis path:

1. interpret SEG metadata / pitch-mark data for one diphone;
2. identify which portions of each decoded PCM unit are selected for joining;
3. reproduce the window / overlap-add behavior;
4. concatenate a small known phone sequence without clicks or timing errors.

The target for M13 is the first portable **multi-diphone waveform**, not merely
one raw database unit.
