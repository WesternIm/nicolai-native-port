# M7 correction carried into M8

M7 correctly abandoned the M6 synthetic write-patch model, but it still made one
important semantic mistake: it treated the second dword in a serialized
`{0xA4F22A56, value}` pair as an absolute EDAT file offset.

Disassembly of the legacy resolver in `mtsyc32.dll` shows that `value` is a
**logical offset inside a static-data duration/range**. The caller supplies the
duration (0 = initialization, 1 = execution), and the resolver returns:

```
runtime_base[duration] + logical_offset
```

The range descriptor selects the range by tag and validates the offset against a
per-duration maximum. The 0x18-byte EDAT header contains tag/file-offset fields
used to describe the two segments; those header fields are not ordinary
serialized runtime references and are excluded by the M8 scanner.

This correction is material: the same serialized pair can map to different
addresses depending on the duration supplied by the caller. M8 therefore makes
`StaticDuration` explicit in the active API.
