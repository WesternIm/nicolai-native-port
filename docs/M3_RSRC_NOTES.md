# `rsrc.dsc` lead

Recovered from `nicolai16.dat`:

- path source/tag: `0xA43EB4`
- path text begins: `0xA43EBC`
- tagged target: `0x0002DE64`
- next unique tagged target: `0x0002DF84`
- candidate object length: 288 bytes (`0x120`)
- reference count to this exact target: 1

The 288-byte candidate object consists of a 15-byte all-zero prefix followed by
13 occurrences of the byte sequence `AD AE 3C AB EC` at exactly 21-byte
intervals (offsets 15, 36, ..., 267), with zero padding between occurrences.

Do not yet label this as a Huffman table: the association with generic
`16stbi/rsrc.dsc` is strong, but the field semantics have not been reconstructed.

---

**M4 correction:** the Segment-0 target associated with `rsrc.dsc` is now known
to sit behind SpeechCube's generic static/virtual-file layer.  Do not interpret
the 288-byte region as a Huffman/codebook payload.  See `M4_FINDINGS.md` and
`M4_STATIC_LAYER.md`.
