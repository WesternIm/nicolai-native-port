# M5 findings — EDAT static-file records

M5 reconstructs the serialized static/preloaded file-node layer used by the
ELAN EDAT image.  It does not execute the legacy x86 engine.

## Static-file node signature

A reproducible record form is present in Segment 1:

```
[tagged ref -> Segment0 payload]
[path string, max runtime field width 0x104]
[u32 field_104]
[u32 field_108]
[tagged/ref-bearing serialized tail ...]
```

The tagged reference is encoded as:

```
0xA4F22A56
u32 target_file_offset
```

The original 32-bit runtime structure used native pointers.  EDAT expands those
pointer-bearing fields into tagged file-offset references, so the bytes after a
pointer field cannot be interpreted by applying runtime offsets blindly.

`scan_static_file_records()` recognizes these records generically by EDAT
layout, target region, path shape and fixed scalar fields.

## Results on Nicolai 16 kHz

The probe finds 24 path-bearing static-file records.

The generic cmp16 descriptor is recovered without hardcoding its address:

```
path:       c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\rsrc.dsc
record ref: 0xA43EB4
payload:    0x2DE64
field_104:  2
field_108:  2
field_10c tagged target: 0x128
```

Nicolai's mode descriptor is recovered by the same scanner:

```
path:       c:\cvswf\ref\data\tempo-psola\russian\nicolai\16aci\modeinfo.dsc
record ref: 0xA71D10
payload:    0x2DC24
field_104:  2
field_108:  2
field_10c tagged target: 0x2DF84
```

This establishes that the path/payload relation used in earlier milestones is
not a one-off coincidence.

## rsrc payload structure

Starting from the automatically located `rsrc.dsc` payload, M5 detects a long
regular region:

```
first marker: 0x2DE73
marker:       AD AE 3C AB EC
stride:       21 bytes
count:        116 consecutive records
```

The semantics of those 21-byte records are not yet assigned.  M5 deliberately
does not call them Huffman records or QMLT rows without evidence.

## Important model correction

The EDAT image behaves more like a serialized/preloaded memory database than a
simple archive of original text files.  Segment 1 contains path-bearing nodes
and tagged pointer relocations; Segment 0 contains the corresponding static
payloads / runtime-oriented data.

Therefore the next task is not necessarily to regenerate the historical
plaintext `rsrc.dsc`.  A better path is to reconstruct the pointer relocation /
static-memory materializer far enough to interpret the runtime structures that
already exist in the image.

## M6 target

1. Recover the tagged-pointer relocation semantics for one complete file node.
2. Determine how the payload at `0x2DE64` is represented in runtime memory.
3. Identify the 116 x 21-byte records by matching their consumers in
   `mtsyc32.dll`.
4. Once one real table is identified, expose it through a portable C++ view and
   connect it to the M4 cmp16 readers/decoder.
