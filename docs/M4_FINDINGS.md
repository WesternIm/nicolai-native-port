# M4 findings — cmp16sbi manifest and quantizer readers

M4 moves from locating compressed acoustic ranges to reconstructing the legacy
`cmp16sbi` decoder initialization formats.  The new C++ code is portable and
executes no legacy x86/Windows code.

## Important correction to the M3 `rsrc.dsc` interpretation

M3 correctly associated the tagged path

```
c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\rsrc.dsc
```

with a Segment-0 static-data target, but the 288-byte region beginning at
`0x2DE64` must **not** be treated as a serialized Huffman table.  Reverse
engineering of the loader shows that `rsrc.dsc` is a descriptor/manifest which
is opened through SpeechCube's static/virtual-file layer and then parsed by the
routine at legacy VA `0x1010B2C0`.

The strange repeated bytes near `0x2DE64` therefore belong to the EDAT static
memory representation.  M4 does not assign codec semantics to them.

## Exact `rsrc.dsc` token structure

The legacy parser uses `fopen(..., "rt")`, then `fscanf` with `%s` nine times.
The parsed object reserves 0x200 bytes for each pathname.

The path slots and their use-sites are:

| Runtime slot | Role | Parser VA |
|---:|---|---:|
| `0x000` | QMLT | `0x1010AD90` |
| `0x200` | QNF | `0x1010C220` |
| `0x400` | QRms | `0x1010B4E0` |
| `0x600` | HuffmanMLT | `0x1010B6B0` |
| `0x800` | HuffmanRMS | `0x1010B9D0` |
| `0xA00` | QVec A | `0x1010AFE0` |
| `0xC00` | QVec B | `0x1010B140` |
| `0xE00` | sequence | `0x1010C020` |
| `0x1000` | noise | `0x1010BEB0` |

The textual read order is:

```
QMLT QNF QRms HuffmanMLT HuffmanRMS QVecA QVecB noise sequence
```

Ten `%hd` scalars follow.  Their read order maps to runtime offsets:

```
read[0] -> 0x1204
read[1] -> 0x1200
read[2] -> 0x1202
read[3] -> 0x1206
read[4] -> 0x120E
read[5] -> 0x1210
read[6] -> 0x1214
read[7] -> 0x121C
read[8] -> 0x1208
read[9] -> 0x120A
```

The legacy parser then copies `0x1204 -> 0x120C` and `0x1200 -> 0x1212`.
The semantic names of those ten scalar fields are intentionally left unknown
until their decoder use-sites are reconstructed.

`parse_cmp16_manifest()` implements the recovered text format in portable C++.

## QMLT format recovered

The QMLT parser at `0x1010AD90` uses `%hd` and `%lf`.

Recovered grammar:

```
int16 row_count
repeat row_count times:
    int16 centroid_count      // 0..32
    double centroid[centroid_count]
    double threshold[centroid_count - 1]
```

The decoder converts each centroid to fixed-point using:

```
trunc(centroid * 8192.0)
```

and stores the low 16 bits.  `8192 == 2^13`, therefore this is a Q13
representation.  The threshold values are read/consumed by this initializer but
are not copied into the recovered runtime centroid array at this use-site.

M4 implements this as `parse_qmlt()` and retains both the source doubles and the
Q13 values for analysis.

## QRms format recovered

The QRms parser at `0x1010B4E0` is a single codebook:

```
int16 count              // 0..32
double value[count]
```

For each input `x`, the legacy x87/MSVCRT sequence computes:

```
trunc(4.0 * pow(10.0, x * 0.05))
```

`0.05 == 1/20`, so equivalently:

```
trunc(4 * 10^(x/20))
```

The resulting low 16 bits are stored.  The relevant constants in the DLL are
confirmed as 10.0, 0.05 and 4.0; `_CIpow` and `_ftol` are imported from
MSVCRT.

M4 implements this as `parse_qrms()`.

## Decoder resource map

Reverse engineering of the outer loader establishes all nine parser roles:

```
rsrc.dsc
  +-- sequence    -> 0x1010C020
  +-- QMLT        -> 0x1010AD90
  +-- QRms        -> 0x1010B4E0
  +-- HuffmanRMS  -> 0x1010B9D0
  +-- HuffmanMLT  -> 0x1010B6B0
  +-- noise       -> 0x1010BEB0
  +-- QVecA       -> 0x1010AFE0
  +-- QVecB       -> 0x1010B140
  +-- QNF         -> 0x1010C220
```

This replaces the earlier broad hypothesis that `rsrc.dsc` itself might be a
codebook.

## What is still blocking the first real acoustic decode

The main remaining boundary is now sharply defined: the EDAT voice database
contains a **static-memory / virtual-file representation** of these resources.
The original loader can supply them to the text readers as ordinary resource
files, but the native reimplementation does not yet materialize those nine
resource payloads from EDAT.

Therefore M4 does **not** claim that unit 0 has been decoded to coefficients or
PCM.  It does establish the resource graph and implements two real codec-table
readers independently of x86.

## M5 target

Reconstruct the EDAT static-resource materializer sufficiently to obtain one or
more of the real `cmp16sbi` resources (preferably QMLT/QRms/Huffman tables), run
those resources through the M4 parsers, then connect the resulting real tables
to the portable Huffman decoder.
