# M8 findings — legacy static address resolver and resource registry

## 1. Legacy resolver semantics recovered

Relevant `mtsyc32.dll` routines identified during disassembly:

- forward static resolver around `0x10008980`
- reverse resolver around `0x10008A10`
- range-descriptor setup around `0x10008A60` / `0x10008CC0`
- root translation loop around `0x10009352`
- static allocator path around `0x1000A8C0`

The forward routine takes a tag, a logical offset, and a duration. It walks
0x60-byte range descriptors. For a matching tag and in-range logical offset it
returns the selected runtime base plus the logical offset.

For the Nicolai EDAT, a portable memory-mapped interpretation is:

```
Initialization base = file offset 0x00000018
Execution base      = file offset 0x00A43EB4
```

The corresponding sizes are `0xA43E9C` and `0x30180`.

Two root references validate the model:

```
Segment0 root: logical 0xA4245C, initialization
  0x18 + 0xA4245C = 0xA42474 -> "Fenetres_de_Hanning"

Segment1 root: logical 0x2DE64, execution
  0xA43EB4 + 0x2DE64 = 0xA71D18
  -> c:\cvswf\ref\data\tempo-psola\russian\nicolai\16aci\modeinfo.dsc
```

This is the central M8 correction: serialized pointers are **not absolute file
offsets**.

## 2. Serialized reference census

Excluding the 0x18-byte EDAT header, Nicolai contains 3190 serialized tagged
references. With the single recovered range descriptor:

- 3190 fit the initialization duration
- 706 also fit the execution duration
- 706 therefore fit both ranges by bounds alone
- 0 fit neither

This demonstrates why source location is insufficient to infer duration. The
caller/routine supplies that semantic choice.

## 3. `rsrc.dsc` leads to a real resource registry

The path-bearing metadata record for:

```
c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\rsrc.dsc
```

contains a secondary serialized reference with execution logical offset `0x128`.
Resolving it yields file offset `0xA43FDC`.

At that location M8 parses a three-pointer registry header:

1. auxiliary pointer
2. base-directory pointer
3. linked-list head pointer

The base directory is:

```
c:\cvswf\elan4406\dsc\gendesc\desc\16stbi\
```

The list nodes are three serialized pointer slots:

```
name -> path -> next
```

The real Nicolai database yields exactly **201 entries** and terminates cleanly.
Examples:

```
Syc.VoxDsc       -> ...\16stbi\voix.dsc
Syc.CanauxDsc    -> ...\16stbi\canaux.dsc
Syc.Russkij.DataDir -> c:\cvswf\ref\cvox\rusvox\data
Syc.Russkij.Abrev.Abreviations -> ...\abb_rus.txt
```

This is the first milestone where the portable code walks a meaningful Acapela
resource graph rather than merely cataloguing offsets.

## 4. What M8 does not claim

M8 does not yet execute cmp16 decompression or PSOLA synthesis. It also does not
assume that a serialized reference's duration can be inferred solely from its
source segment. Duration remains explicit in the portable API until individual
runtime structures establish it.

## 5. Next target

M9 should follow the registry's `Syc.VoxDsc` and `Syc.CanauxDsc` resources and
recover their serialized/runtime descriptor graphs. The goal is to reach the
voice/channel-specific cmp16 resources (QMLT, QRms, HuffmanMLT/RMS, QNF, QVec,
sequence, noise) using the corrected resolver rather than heuristic address
interpretation.
