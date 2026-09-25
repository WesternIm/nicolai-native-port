# M3 findings — ANA unit directory and portable Huffman builder

M3 converts the opaque ANA/acoustic region found in M2 into a reproducible
variable-length directory of compressed acoustic ranges.  No legacy Windows
code is executed.

## Recovered ANA boundary chain

`analysis_data` is 86,284 bytes.  It is a chain of variable-sized records.  The
first two little-endian signed 32-bit integers of every record are:

```
compressed_start, signed_compressed_end
```

The robust recovered invariant is:

```
record[n+1].compressed_start == abs(record[n].signed_compressed_end)
```

The next record is therefore found by searching the bounded, 16-bit-aligned
forward window for that next start value.  This avoids imposing a guessed fixed
record format on metadata which is demonstrably variable-length.

On the supplied Nicolai database the parser recovers:

- 2,665 records
- compressed pool: 6,132,372 bytes
- indexed coverage: 6,132,370 bytes
- trailing bytes: 2
- negative signed ends: 978
- positive signed ends: 1,687
- compressed range sizes: 1,076..6,382 bytes, mean 2,301.08 bytes

The sign of `signed_compressed_end` is preserved but **M3 does not assign a
meaning to it yet**.

The ranges are strongly consistent with acoustic/diphone indexing, but M3 does
not claim that each range is an independently framed compressed stream.  They
may be boundaries into a shared codec bit/sample stream.

## First recovered ranges

```
#0  ANA+0x0000  voice[0,1759)      record=16 bytes
#1  ANA+0x0010  voice[1759,3678)   record=16 bytes
#2  ANA+0x0020  voice[3678,6697)   record=30 bytes
#3  ANA+0x003E  voice[6697,9825)   record=34 bytes
#4  ANA+0x0060  voice[9825,12819)  record=34 bytes
#5  ANA+0x0082  voice[12819,16596) record=40 bytes
```

The terminal record begins at ANA+`0x15102` and indexes
`voice[6130629,6132370)`.  The complete ANA table ends ten bytes later.

## Portable Huffman construction

M2 contained an architecture-neutral traversal equivalent to the small legacy
x86 tree walker.  M3 adds `build_huffman_tree()`, which constructs the same
logical binary tree from explicit bit codes without x86 pointer layouts.

The builder is deliberately separated from serialized Acapela codebook parsing:
M3 has not yet recovered the `cmp16sbi` codebook serialization.

## `rsrc.dsc` lead

Segment 1 contains a tagged path:

```
c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\rsrc.dsc
```

Its tagged target is Segment-0 offset `0x2DE64`.  The object is bounded by the
next tagged target at `0x2DF84`, i.e. 288 bytes.  It has a conspicuous structure:
15 zero bytes followed by 13 repetitions at 21-byte spacing of the 5-byte
sequence `AD AE 3C AB EC` plus zero padding.

This is recorded as a codec-resource lead only.  Its semantics (quantizer,
codebook descriptor, constants, or something else) are not yet proven.

## M4 target

Recover the `cmp16sbi` initialization/framing around the resource descriptor and
serialized Huffman/quantizer tables, then decode one indexed acoustic range into
pre-PSOLA numeric data.  PCM is **not** claimed in M3.
