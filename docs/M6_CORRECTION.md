# M6 correction carried by M7

M6 correctly found the path-bearing Segment1 records and the mapping of their
known prefix fields, but over-interpreted the reconstructed prefix length as the
complete x86 runtime object size.

M7 proves from `mtsyc32.dll` that the runtime descriptor is 0x120 bytes.  The
M6 0x114-byte product is retained only as a historical/known-prefix experiment.
New work must use `static_replay.hpp` and must not infer the runtime tail
(+0x114/+0x118/+0x11c) from the path-bearing record alone.
