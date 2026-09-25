# M2: legacy Huffman decoder notes

`mtsyc32.dll` contains source-path/debug strings tying the acoustic decoder to the
Tempo/PSOLA implementation and a `cmp16sbi` codec:

- `ref\\cvox\\psolav1\\tds\\cmp16sbi\\init.c`
- `ref\\cvox\\psolav1\\tds\\cmp16sbi\\huffman_dec.c`
- decoder labels/diagnostics include `Huffman RMS`, `HuffmanMLT`, `NOISE`.

A small tree-walk primitive was identified in the x86 code around image address
`0x1010C930` (for the examined build). Its relevant 32-bit node behavior is:

- `WORD [node+2] > 0` means leaf;
- leaf value is obtained through the pointer at `[node+4]` and returned as a WORD;
- internal node consumes one logical bit represented as a 16-bit word;
- zero follows the pointer at `[node+0x0C]`;
- non-zero follows the pointer at `[node+0x08]`;
- running past the supplied logical-bit count returns `0xFFFF`.

`include/nicolai/huffman.hpp` and `src/huffman.cpp` implement the same traversal as
portable logical nodes rather than trying to preserve 32-bit pointer layout.

This is **not yet the full Nicolai decompressor**. M3 still needs the tree-builder,
bitstream framing, quantizer/state reconstruction and the sample reconstruction
path that surrounds this primitive. The purpose of M2 is to replace the first
confirmed x86 decoder primitive with architecture-independent code.
