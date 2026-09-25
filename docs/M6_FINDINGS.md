> **Superseded by M7:** the M6 write-patch/materializer interpretation was too strong. See `M7_FINDINGS.md`.

# M6 findings — exact EDAT path-node materialization

M6 corrects the most important ambiguity left by M5.

## The `0x120` record is a patch, not a runtime node

For all 24 path-bearing records, Segment 1 contains an exact serialized shape:

```text
+0x000  tagged ref: destination in Segment0    8 bytes
+0x008  path[0x104]
+0x10c  u32 field_104
+0x110  u32 field_108
+0x114  tagged ref: runtime field +0x10c       8 bytes
+0x11c  u32 runtime field +0x110
--------
0x120 bytes serialized
```

The 32-bit runtime object is only `0x114` bytes:

```text
+0x000  char path[0x104]
+0x104  u32
+0x108  u32
+0x10c  pointer
+0x110  u32
--------
0x114 bytes runtime
```

The extra 12 serialized bytes are exactly:

- 8-byte destination tag that is not part of the runtime object;
- +4 bytes because a 32-bit pointer is serialized as an 8-byte `magic + offset` reference.

This explains why interpreting bytes after `+0x114` as fields of the same node was wrong: for ordinary records that location is already the next patch record.

## Reproducible records in Nicolai

M6 finds **24/24** exact path patch records.  Examples:

```text
rsrc.dsc
  serialized:  0xA43EB4
  destination: 0x2DE64
  object ref:  0x00000128
  fields:      2 / 2 / state 1

modeinfo.dsc
  serialized:  0xA71D10
  destination: 0x2DC24
  object ref:  0x0002DF84
  fields:      2 / 2 / state 1
```

The portable materializer writes a faithful 0x114-byte node at the Segment0
destination, storing the referenced EDAT file offset at `+0x10c` as a pointer
surrogate.  It deliberately does not invent a host pointer yet.

## Correction to the M5 `116 x 21` interpretation

The repeated `AD AE 3C AB EC` / 21-byte-stride region is visible in Segment0
*before* materialization and spans across multiple patch destinations, including
the `rsrc.dsc` destination.  Therefore it cannot be identified as an
`rsrc.dsc`-specific Huffman/QMLT table.

M6 treats it only as pre-materialization backing contents.  Once the rsrc path
patch is applied, the bytes at 0x2DE64 are overwritten by the runtime file node
beginning with `c:\\cvswf\\...\\16stbi\\rsrc.dsc`.

## Why this matters

The loader model is now concrete:

```text
Segment1 patch stream
      |
      | destination tagged ref
      v
Segment0 backing allocation
      ^
      | materialized 32-bit runtime object
```

This is the first EDAT object class for which we can reproduce the loader's
serialized-to-runtime size contraction (`0x120 -> 0x114`) exactly.

## M7 target

The bytes immediately following `modeinfo.dsc` and `rsrc.dsc` begin additional
non-path patch records.  M7 should recover the generic patch grammar so the
materializer can rebuild not only path nodes but the referenced runtime objects
(`object_ref` targets) and eventually expose the real cmp16 descriptor tables.
