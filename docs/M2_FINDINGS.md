# Nicolai native port — M2 findings

M2 follows EDAT links from the real late `nbr16aci.ana` descriptor rather than
hard-coding the final payload offsets.

For the examined `nicolai16.dat`:

| item | range / offset | bytes |
|---|---:|---:|
| ANA path | `0x453F9C` | — |
| ANA descriptor | `0x453F84` | — |
| analysis object | `0x4540A4` | — |
| inline analysis data | `0x4540BC..0x4691C8` | 86,284 |
| compressed acoustic payload | `0x4691C8..0xA4245C` | 6,132,372 |
| next object | `0xA4245C` | — |

The object at `0xA4245C` contains the ASCII marker `Fenetres_de_Hanning`, which is
consistent with a synthesis/windowing object immediately following the opaque
acoustic payload.

## Checksums / statistical evidence

Inline analysis block:

- SHA-256: `5160cafd5bbe03eba59d127b20ccc46c17735a74031ad15a7d9a4ccc1cd97998`
- entropy: ~5.391184 bits/byte

Opaque compressed acoustic payload:

- SHA-256: `4fc9b7815fc0cf8986d2e533743fce7db1807109893eddac91babb6b0e527ab6`
- entropy: ~7.241720 bits/byte

The acoustic range contains no normal EDAT tagged-object boundary before its end;
it behaves like an opaque compressed stream rather than a normal object graph.

## Codec evidence from the legacy engine

The corresponding x86 `mtsyc32.dll` includes debug/source-path strings for:

- `psolav1/tds/cmp16sbi/init.c`
- `psolav1/tds/cmp16sbi/huffman_dec.c`
- PSOLA/diphone code (`syc_tds.c`, `syc_rgl.c`, `diphone.c`)

This makes a custom Huffman-based 16-bit acoustic compression path the strongest
current interpretation of the 6.13 MB payload.

## What M2 does

- Locates the ANA descriptor structurally.
- Follows its EDAT links to the analysis object, SEG object and acoustic payload.
- Identifies payload end from the next EDAT target rather than a fixed constant.
- Can optionally extract `analysis.bin` and `compressed_voice.bin` locally.
- Implements a portable version of the confirmed Huffman tree-walk primitive.
- Does **not** pretend that the complete decoder is finished; PCM synthesis is
  still disabled until the framing/tree construction and reconstruction stages
  are understood.

## M3 target

Reconstruct the `cmp16sbi` stream framing and Huffman-tree initialization well
enough to decode the first acoustic block into sample-domain values. After that,
the next target is correlating decoded samples with `.seg/.ana` unit boundaries
and feeding them into the Tempo/PSOLA overlap-add path.
