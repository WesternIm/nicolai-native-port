# M11 findings — cmp16 entropy front-end

M11 follows the exact `# -> p` compressed ANA slice recovered by M10 into the
legacy cmp16 decoder.  It still executes **no** x86 code.

## 1. Packed ANA bytes are expanded MSB-first

The inner driver at legacy VA `0x1021B050` indexes the static table at
`0x106139F0` with `compressed_byte << 4` and copies eight consecutive
`uint16_t` entries into the decoder's bit buffer (`descriptor + 0x40`).

Direct inspection of the table gives:

- `0x00 -> 00000000`
- `0x01 -> 00000001`
- `0x02 -> 00000010`
- `0x55 -> 01010101`
- `0x80 -> 10000000`
- `0xA5 -> 10100101`
- `0xFF -> 11111111`

Therefore the transformation is exactly ordinary **MSB-first byte-to-bit
expansion**, except the legacy decoder stores each logical bit in one 16-bit
word.  M11 reimplements this in `expand_cmp16_word_bits_msb()`.

## 2. Internal block size is 32 source bytes

The runtime initializer around `0x10058B09` stores `0x20` into descriptor field
`+0x4E`.  The driver loops exactly that many source bytes, generating eight
WORD bits per byte.  One legacy entropy block is therefore:

```
32 packed bytes -> 256 uint16_t logical bits
```

For the M10 `# -> p` slice (1,759 bytes), simple geometry is 54 complete
32-byte blocks plus 31 bytes.  M11 deliberately does **not** assign semantic
meaning to the final 31-byte remainder yet.

## 3. The Huffman entry point is now connected to the real bitstream

The small routine at `0x1010C930` (`huffman_dec.c`) consumes a `uint16_t` array
of 0/1 values and advances a WORD bit position.  Zero chooses one child and a
non-zero word chooses the other.  It returns the leaf's signed 16-bit symbol or
`-1` when the supplied bit budget is exhausted.

The real decoder calls it from:

- first entropy stage: `0x102287A0` (call at `0x102287FE`)
- second entropy stage: `0x10228910` (call at `0x102289FE`)
- frame driver: `0x1021B050`

The portable `decode_cmp16_huffman()` now uses the same word-bit cursor
contract.  Synthetic codebooks are covered by tests; the remaining blocker for
**real** symbols is binding the voice's real Huffman codebooks.

## 4. Exact cmp16 resource order / runtime fields

The decompression descriptor reader at `0x1010B2C0` allocates `0x1220` bytes,
reads nine `%s` tokens into 0x200-byte slots, then reads its scalar tail.  The
constructor (`0x10058309...`) turns those paths into file objects.  Parser
dispatch (`0x10058510...`) proves the roles:

| resource | manifest slot | runtime file object | parser VA |
|---|---:|---:|---:|
| QMLT | `+0x000` | `+0x6C` | `0x1010AD90` |
| QNF | `+0x200` | `+0x88` | `0x1010C220` |
| QRms | `+0x400` | `+0x70` | `0x1010B4E0` |
| HuffmanMLT | `+0x600` | `+0x78` | `0x1010B6B0` |
| HuffmanRMS | `+0x800` | `+0x74` | `0x1010B9D0` |
| QVecA | `+0xA00` | `+0x80` | `0x1010AFE0` |
| QVecB | `+0xC00` | `+0x84` | `0x1010B140` |
| sequence | `+0xE00` | `+0x68` | `0x1010C020` |
| noise | `+0x1000` | `+0x7C` | `0x1010BEB0` |

The parsed runtime decoder object is `0x8C` bytes.  Additional proven fields:

- `+0x40`: expanded WORD-bit buffer
- `+0x44`: derived window/count value
- `+0x48`, `+0x4A`, `+0x4C`: 16-bit decode dimensions/counts
- `+0x4E`: source block bytes = **32**
- `+0x50`: compiled HuffmanMLT runtime
- `+0x54`: compiled HuffmanRMS runtime
- `+0x58`: maximum value derived from QMLT
- `+0x5C`: scratch allocation
- `+0x60`: `field_44 / 4`
- `+0x64`: 256-WORD scratch allocation

## 5. Exact first real input used by the portable front end

M10 identified the first Nicolai diphone `# -> p` as ANA bytes `[0,1759)`.
M11 feeds those exact bytes to the portable bit expansion.  The first block is:

```
55 55 55 55 55 55 55 55
55 55 55 55 55 55 55 55
55 55 55 55 55 d5 d5 d5
d5 d5 d5 d5 d5 d5 d5 d5
```

and begins as the logical bit stream:

```
01010101 01010101 ... 01010101 11010101 11010101 ...
```

This is the first point in the port where **actual Nicolai compressed bytes are
transformed by code that exactly mirrors the legacy entropy front-end**.

## Remaining boundary

M11 does not claim decoded Nicolai coefficients yet.  The next narrow problem
is to locate/materialize the actual `HuffmanRMS` and `HuffmanMLT` parsed tables
(or their serialized preloaded representation), then feed the M11 word-bit
cursor into those real trees.  Once that succeeds the first output will be
real entropy symbols rather than a synthetic self-test.
