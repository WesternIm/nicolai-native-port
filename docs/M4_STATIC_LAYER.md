# M4 static-resource layer notes

The Nicolai `.dat` footer identifies the build as an ELAN `edat` image for
`intel/WINDOWS/MSVC6`.  The legacy DLL contains diagnostics for loading and
saving "static data" and for resolving "static memory" for resource files.

For `16stbi/rsrc.dsc`, Segment 1 contains a tagged path association whose target
is Segment-0 offset `0x2DE64`.  Inspection of the runtime loader shows that this
association is consumed by a generic static/virtual file layer before the
`cmp16sbi` descriptor reader runs.

Consequences:

1. `0x2DE64` is not interpreted as plaintext `rsrc.dsc`.
2. The repeated `AD AE 3C AB EC` pattern is not labeled a Huffman codebook.
3. The next reverse-engineering task is the generic EDAT static-file
   materializer, not another search for the compressed voice pool.
4. Once a virtual file can be reconstructed as bytes/text, M4 already has
   portable readers for the manifest, QMLT and QRms.

Useful legacy evidence:

- descriptor parser: `0x1010B2C0`
- generic resource locator used before it: `0x1000AE80`
- generic parser/static wrapper: `0x1000B440`
- manifest is opened with `fopen(path, "rt")`
- format strings confirmed in the binary: `%s`, `%hd`, `%lf`, `%f`, `%d`

This document intentionally stops short of assigning an unproven serialization
format to the static-memory block.
