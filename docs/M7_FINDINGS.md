# M7 findings — EDAT address space and M6 correction

## Result

M7 replaces the M6 "flat write-patch stream" model with an address-space model
that matches the legacy x86 loader more closely.

The 8-byte form

```
56 2A F2 A4  <u32 absolute EDAT offset>
```

is treated as a serialized tagged reference.  The target is an **absolute file
offset**.  The legacy loader keeps two loaded segment ranges and translates the
offset into the corresponding in-memory segment address.

The portable implementation deliberately represents the result as:

```
(region, absolute_file_offset, relative_offset)
```

instead of inventing a native ARM64 pointer.

## Verified layout for the supplied Nicolai database

```
Header:   [0x000000, 0x000018)
Segment0: [0x000018, 0xA43EB4)
Segment1: [0xA43EB4, 0xA74034)
Footer:   [0xA74034, 0xA74234)
```

Tagged-reference scan:

- total: 3192
- unresolved/outside: 0
- source Header: 2
- source Segment0: 2476
- source Segment1: 714
- target Header: 8
- target Segment0: 3183
- target Segment1: 1

Source -> target pairs:

- Header -> Segment0: 1
- Header -> Segment1: 1
- Segment0 -> Header: 1
- Segment0 -> Segment0: 2475
- Segment1 -> Header: 7
- Segment1 -> Segment0: 707

The two segment roots decode cleanly:

```
Segment0 @ 0x18     -> 0xA4245C (Segment0 + 0xA42444)
Segment1 @ 0xA43EB4 -> 0x02DE64 (Segment0 + 0x02DE4C)
```

## x86 loader evidence

The following addresses refer to the supplied `mtsyc32.dll` image base as seen
in the disassembly used during M7.

### Header is exactly six dwords / 0x18 bytes

At `0x100090EA` the loader calls `fread` for exactly `0x18` bytes.  The six
values match the EDAT header already parsed by M1:

1. Segment0 size
2. Segment1 size
3. Segment0 tag/magic
4. Segment0 file offset
5. Segment1 tag/magic
6. Segment1 file offset

### Two segment bases are built independently

At `0x10009164` (one loader mode), the code iterates twice and forms loaded
segment bases from a file/image base plus each segment file offset.

At `0x10009352..0x10009510`, the loader operates on two segment descriptors and
performs range-based translations.  The important behavior for the portable
model is that a serialized absolute offset is associated with a loaded segment
range before becoming a runtime address.

### `0x120` is an allocation request, not proof of a serialized patch size

At `0x1000B2D6` the virtual/static file path calls `0x1000A8C0` with a requested
size of `0x120`.  The returned storage is then populated as a runtime file
object.  The caller writes fields at:

```
+0x104
+0x108
+0x10C
+0x110
+0x114
+0x118
+0x11C
```

Therefore M6's claim that an observed `0x120` span necessarily serialized into
a `0x114` runtime node was too strong.  It mixed two distinct concepts:

- static allocation / resource metadata
- runtime file-object construction

M7 removes the materializer from the active library and keeps it only under
`legacy/m6/` as historical work.

## Path-bearing Segment1 records: revised interpretation

M7 still detects 24 useful path-bearing records, but names their first reference
`primary_ref_offset` rather than `payload` or `destination`.

Two examples:

```
rsrc.dsc
  primary:   0xA43EB4 -> 0x2DE64
  secondary: 0xA43FC8 -> 0x00128
  fields:    2 / 2

modeinfo.dsc
  primary:   0xA71D10 -> 0x2DC24
  secondary: 0xA71E24 -> 0x2DF84
  fields:    2 / 2
```

Both primary and secondary references resolve through the same EDAT address
space.  Their higher-level meaning is intentionally left open until the static
allocator/range descriptors are reconstructed.

A separate geometry check finds **22 consecutive path-bearing records** where
both the Segment1 metadata location and the primary Segment0 target advance by
exactly `0x120`:

```
serialized: 0xA70570 .. 0xA71D10
primary:    0x02C484 .. 0x02DC24
first: non_verb1.flx
last:  nicolai\16aci\modeinfo.dsc
```

This agrees with the `0x120` allocation request seen at `0x1000B2D6`, but M7
treats it only as an allocation-geometry correlation, **not** proof of a
write-patch format.

## New portable code

M7 adds:

- `include/nicolai/address_space.hpp`
- `src/address_space.cpp`
- `tests/address_space_test.cpp`
- `tools/nicolai_m7_probe.cpp`

The old M6 materializer is no longer linked into `nicolai_port`.

## Next target

The next useful layer is the range/static-allocation descriptor used by
`0x1000A8C0` and the loader around `0x10009352`.

The goal for M8 is to model the legacy resolver far enough to answer:

1. which static allocation owns a given EDAT absolute target,
2. what runtime base/correction the x86 loader associates with it,
3. what the `rsrc.dsc` primary/secondary refs mean in that allocation table,
4. and then follow the resolved cmp16 descriptor toward QMLT/Huffman resources.
