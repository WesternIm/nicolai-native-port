# M10 findings — deterministic PHO/AXM/SEG/ANA diphone graph

M10 replaces the M3 heuristic unit-boundary scan with a deterministic chain recovered from Nicolai's own access matrix. No x86 code is executed by the portable probe.

## 1. Important naming correction

The initialized resources are now identified unambiguously by their legacy file objects:

- `nbr16aci.seg` data: `0x4540BC .. 0x4691E0` = **86,308 bytes**
- `nbr16aci.ana` data: `0x4691E0 .. 0xA42474` = **6,132,372 bytes**

Earlier milestones called the first range an “ANA index” and the second the generic “compressed voice pool”. Functionally the old boundary inference was useful, but M10 establishes that the variable-length directory is the **SEG payload** and the cmp16 acoustic bitstream is the **ANA payload**.

The compatibility function `parse_ana_unit_index()` is retained, but new code should use `DiphoneCatalog`.

## 2. Nicolai phone inventory recovered

`nbr16aci.dsc.pho` resolves to file offset `0x42BCE4`.

Its initialized representation begins with:

```text
u32 reserved = 0
u32 phone_count = 66
char packed_symbol_stream[]
```

The packed stream deterministically tokenizes into 66 symbols:

```text
# p p' b b' m m' f f' v v' t t' d d' n n'
s s' z z' sh zh c sc ch l l' r r' j k k' g g' x x'
C CH SC X a0 a1 a3 a4 a5 A0 A1 u0 u1 u4 U0 U4
i0 i1 i4 y0 y1 y4 o0 o1 O0 e0 e4 E0 E4
```

This gives the row/column labels for the AXM access matrix.

## 3. AXM format solved

`nbr16aci.axm` data starts at `0x42DCEC`.

The file is exactly:

```text
66 * 66 * u32 matrix offsets = 17,424 bytes = 0x4410
2665 * 0x18-byte records      = 63,960 bytes
--------------------------------------------------
total                           81,384 bytes = 0x13DE8
```

That total lands exactly on the next initialized resource (`nbr.rgl` at `0x441AD4`).

All **2665** non-zero matrix entries are unique. Each points to one 24-byte record with the verified layout:

```text
+0x00  u32  0
+0x04  u32  0
+0x08  u8   1
+0x09  u8   2
+0x0A  u8   left_phone_index
+0x0B  u8   right_phone_index
+0x0C  u32  seg_offset
+0x10  u32  seg_size
+0x14  u32  0
```

The embedded `(left,right)` indexes match the AXM matrix cell for **all 2665 records**.

The original x86 AXM loader independently supports this geometry: around `0x10092A4F` it allocates records of size `0x18` while constructing the access table.

## 4. AXM partitions SEG exactly

For every occupied phone pair, AXM supplies an exact `seg_offset + seg_size` slice.

Sorting the 2665 slices by `seg_offset` gives a contiguous partition:

```text
first SEG byte: 0
last SEG byte:  86,308
holes:          0
overlaps:       0
```

Each SEG slice begins with:

```text
i32 compressed_ana_start
i32 signed_ana_end
int16 metadata[]
```

The absolute value of `signed_ana_end` is the end offset in `nbr16aci.ana`. The sign is preserved in the portable structure because its semantic meaning has not yet been proven.

The remainder of each variable-length SEG record is an even-sized vector of `int16` metadata values.

## 5. SEG partitions the compressed ANA payload

The 2665 SEG records form one exact compressed boundary chain:

```text
unit[n + 1].compressed_start == abs(unit[n].signed_end)
```

Coverage on the supplied database:

```text
ANA payload bytes:   6,132,372
indexed by SEG:      6,132,370
trailing bytes:              2
```

There are no gaps between indexed compressed ranges.

This gives a deterministic portable lookup:

```text
(left phone, right phone)
        -> AXM matrix cell
        -> 0x18 AXM record
        -> exact SEG record
        -> exact compressed ANA byte slice
```

## 6. Example: first diphone

The first occupied matrix cell is `# -> p`:

```text
AXM relative record: 0x4410
SEG:                  [0, 16)
ANA:                  [0, 1759)
compressed bytes:     1759
SEG metadata:         [-2, 10, 6, -10]
signed end:           -1759
```

The first 24 bytes of its real compressed ANA slice are:

```text
55 55 55 55 55 55 55 55 55 55 55 55
55 55 55 55 55 55 55 55 55 D5 D5 D5
```

No x86 routine is involved in selecting or extracting those bytes.

## 7. Portable code added

New module:

```text
include/nicolai/diphone_catalog.hpp
src/diphone_catalog.cpp
tests/diphone_catalog_test.cpp
tools/nicolai_m10_probe.cpp
```

Core APIs:

```cpp
parse_nicolai_phone_inventory(...)
parse_nicolai_diphone_catalog(...)
find_diphone(...)
extract_diphone_compressed_bytes(...)
```

The M10 build passes **8/8 host tests**.

## 8. Next target (M11)

The selection/indexing side of the acoustic database is now deterministic. M11 should focus on the codec itself:

1. recover/materialize the real cmp16 resource tables (`QMLT`, `QRms`, Huffman RMS/MLT, QVec, QNF, sequence, noise);
2. bind those tables to one selected `DiphoneUnit`;
3. decode the `# -> p` ANA slice into the first portable numeric acoustic frame/sample representation.

The important change is that M11 no longer needs to guess which bytes belong to a unit: M10 supplies the exact unit payload and its SEG metadata.
