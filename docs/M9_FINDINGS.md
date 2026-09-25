# M9 findings — voice/channel catalogs and Nicolai acoustic descriptor

## 1. `voix.dsc` runtime object recovered

The execution static duration contains a legacy file object at file offset
`0xA498F0` / execution logical offset `0x5A3C`:

```
c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\voix.dsc
```

Its `+0x10c/+0x110` tagged data reference resolves in execution duration to
logical `0x5B5C`, file offset `0xA49A10`.

The x86 loader at `0x1001344d..0x100135b4` independently establishes the
record geometry: up to 64 entries, stride `0x884` bytes. The portable parser now
uses this structure directly.

The real Nicolai database contains 50 occupied voice records out of 64 slots.
The first serialized pointer in each occupied record resolves in initialization
duration to a voice-name string.

Nicolai is record **46**:

```
record file offset: 0xA621C8
name logical:       0x2E8
name:               Nicolai
```

Known fixed fields in the record include:

```
+0x118  exception dictionary path
+0x340  acoustic DSC path
+0x444  modeinfo DSC path
+0x684  255-entry uint16 symbol map
```

For Nicolai:

```
exception:
  c:\cvswf\ref\cvox\front\except\data\exc_rus.txt

acoustic descriptor:
  C:\cvswf\ref\data\tempo-psola\russian\nicolai\16aci\nbr16aci.dsc

mode info:
  c:\cvswf\ref\data\tempo-psola\russian\nicolai\16aci\modeinfo.dsc
```

The Nicolai symbol map at `+0x684` is the identity sequence `0..254`.

## 2. `canaux.dsc` runtime object recovered

The execution duration contains:

```
c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\canaux.dsc
```

at file offset `0xA6BC14`, logical `0x27D60`.

Its data reference resolves to file offset `0xA6BD34`. The x86 channel parser
callback at `0x10011a40` requests a `0x45F0`-byte static object, matching the
observed serialized region exactly.

Portable summary of the real database:

```
serialized size:    0x45F0
slot capacity:      64
non-zero slot mode: 49
```

The detailed semantics of the channel fields remain intentionally unnamed until
more of the parser is recovered.

## 3. Nicolai acoustic DSC recovered

The Nicolai voice record's acoustic path resolves to an initialization-duration
legacy file object at:

```
file offset: 0x429284
logical:     0x42926C
```

Its data tagged reference is logical `0x42938C`. Resolving in initialization
duration gives file offset `0x4293A4`.

The first 16 dwords of the acoustic descriptor are now exposed without assigning
unproven names. Critically, dword `+0x1C` is **16000**, matching every other
independent indication that Nicolai is the 16 kHz voice database.

The descriptor also exposes/reaches the following real assets:

```
c:\cvswf\bddson\diphones\russian\nicolai\dico-psola\nbr.rgl
c:\cvswf\bddson\diphones\russian\nicolai\dico-psola\16aci\nbr16aci.axm
c:\cvswf\bddson\diphones\russian\nicolai\dico-psola\16aci\nbr16aci.seg
c:\cvswf\bddson\diphones\russian\nicolai\dico-psola\16aci\nbr16aci.ana
```

This gives a verified portable chain:

```
resource registry
  -> voix.dsc
  -> voice record 46: Nicolai
  -> nbr16aci.dsc
  -> 16 kHz + RGL/AXM/SEG/ANA acoustic assets
```

No x86 code is executed by the M9 probe.

## 4. M9 correction to the old M2/M3 acoustic byte ranges

M8 established that tagged targets are logical offsets inside a static duration,
not raw EDAT file offsets. Applying that rule to the initialized Nicolai file
objects corrects the old M2/M3 extraction by `+0x18` at both ends.

The initialization-duration objects now resolve as:

```
nbr16aci.seg data logical 0x4540A4 -> file 0x4540BC
nbr16aci.ana data logical 0x4691C8 -> file 0x4691E0
next initialization target 0xA4245C -> file 0xA42474
```

Therefore the verified real file ranges are:

```
ANA/index data: [0x4540BC, 0x4691E0) = 86,308 bytes
cmp16 payload:  [0x4691E0, 0xA42474) = 6,132,372 bytes
```

The corrected cmp16 payload begins with sixteen `0x55` bytes. Parsing the
corrected ANA range still yields exactly **2665 units**, covering **6,132,370**
bytes and leaving the same **2-byte** payload tail. This independently validates
the corrected boundary model.

The old M2/M3 offsets are retained only as historical findings and must not be
used by new decoder code.

## 5. Next target

M10 should follow the initialized `nbr16aci.dsc` / AXM / RGL graph into the
exact `cmp16sbi` initialization descriptor and recover the real QMLT, QRms,
HuffmanMLT/RMS, QNF, QVec, sequence and noise resources with the corrected
static-address semantics. The target is to feed unit #0 from the verified
`0x4691E0` cmp16 stream into the portable decoder.
