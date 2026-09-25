# M26 reverse findings — Russian physical prosody (`physical.int`)

M26 uses the supplied **Acapela/ELAN Nicolai 5.1 MSI** as an additional reverse-engineering source. The MSI contains the original 32-bit Russian synthesis stack (`mtsyc32.dll`, `ettsengine*.dll`, `libvoyellise.dll`, `babtts.dll`) and `nicolai16.dat`. The proprietary files are used only as input to the reverse/parity work and are not redistributed in this source archive.

## 1. Exact `physical.int` resource

The static resource registry inside `nicolai16.dat` contains:

```
...\rusvox\data\physical.int
```

The serialized target is `0x50688`. With the recovered EDAT segment-0 mapping, its payload begins at file offset `0x506a0`. The next static object begins at target `0x508d4`, proving an exact payload size of:

```
0x508d4 - 0x50688 = 0x24c = 588 bytes
588 = 14 * 42
```

This matches the Windows loader/parser: it allocates **14 records of 42 bytes**. M26 therefore parses the resource as 14 `std::array<int8_t,42>` records and validates canonical bytes from the supplied Nicolai image.

Record 0 is all zero. Record 1 begins:

```
-20,49,0,-30,0,20,30,10,-8,0,17,26,8,70,80,90,100,30,
10,20,-20,
15,15,10,15,-10,-15,-20,
15,13,10,13,-10,-15,-20,
15,10,10,10,-10,-15,-20
```

The final 21 bytes form three parallel seven-value groups. `mtsyc32.dll` reads them while authoring the three pitch-percentage fields.

## 2. Upstream pitch-triplet author is now identified

M25 established that the 0x20-byte source feature record contains:

- `+0x10` first signed pitch percentage;
- `+0x14` second signed pitch percentage;
- `+0x18` third signed pitch percentage;
- `+0x1c` pitch anchor/current value;
- `+0x1d` another prosody scalar.

M26 closes the writer chain:

### `0x1019e530`

Parses bracket annotations and writes the source feature record:

```
[tX] -> +0x10, then +0x14, then +0x18
[eX] -> +0x1c
[lX] -> +0x1d
```

### `0x10214f40`

Builds those `[t%d]`, `[e%d]` and `[l%d]` annotations. It indexes `state+0x81258`, which the loader initializes from **physical.int**, using the physical class stored in `state+0x130`.

The normal pitch path accesses the record around bytes 18/19 plus the parallel groups starting at 21, 28 and 35. A recovered terminal/special path uses base byte 20 plus bytes 25, 32 and 39. Calls to `0x10215d30` perform integer linear interpolation between selected signed table values.

### `0x10213150`

Under the explicit-pitch feature bit, copies the authored source values to runtime `+0x190/+0x194/+0x198`, after which the M25 sparse-anchor/interpolation pipeline consumes them.

So the recovered PC chain is now:

```
sentence/morphology/punctuation
        -> 0x10215de0 physical class
        -> physical.int (14 x 42)
        -> 0x10214f40 [t/e/l] authoring
        -> 0x1019e530 source feature fields
        -> 0x10213150 runtime pitch triplet
        -> sparse-anchor interpolation
        -> 83-Hz percentage -> F0
        -> wordstr[28] centering
        -> PSOLA
```

## 3. Physical class classifier (`0x10215de0`)

`state+0x130` is the per-position physical-prosody class array. The classifier writes values **0..13**.

Confirmed sentence-level branches include:

- ordinary declarative closure -> class **1** or **2**;
- WH/interrogative lexical list -> class **3**;
- ellipsis (`...`) -> class **6**;
- other question/morphological branches -> classes **7** / **8**.

Internal punctuation/morphology paths write classes **9..13**. Literal tests recovered in this routine include `:`, `)`, `,`, `(`, `_`, `,_`, and space.

The WH lists embedded in `mtsyc32.dll` contain Russian forms equivalent to groups such as `Кто/кого/кому/...`, `Что/чего/чему/...`, `Сколько`, `Какой`, `Который`, `Каков`, `Как`, `Зачем`, `Почему`, `Где`, `Когда`, `Неужели`.

The exact morphology-dependent choice for all 4/5/7/8/9..13 cases is not yet fully ported. M26 therefore does **not** pretend that one guessed sentence class can replace the entire PC classifier.

## 4. MSI cross-check

The extracted MSI contains, among others:

```
mtsyc32.dll       7,950,336 bytes
nicolai16.dat     10,961,460 bytes
ettsengine.dll       217,088 bytes
ettsengines5.dll     245,861 bytes
libvoyellise.dll      86,016 bytes
babtts.dll           503,808 bytes
exc_rus.txt        2,960,349 bytes
abb_rus.txt            3,636 bytes
```

`mtsyc32.dll` contains literal/debug strings for `pitch.par`, `physical.int`, `wordstr.par`, `rusvox\syc_rus.c` and the generic prosody source module. This independently confirms that the three resources belong to the original Russian prosody layer rather than being coincidental bytes in the voice database.

## 5. Portable implementation

M26 adds:

- `LegacyPhysicalProsodyRecord`;
- `LegacyPhysicalProsodyProfile`;
- `parse_legacy_russian_physical(...)`;
- a normalized diagnostic view of the three seven-value groups;
- `nicolai_m26_probe`, which dumps the exact signed table from a legally supplied `nicolai16.dat`;
- an opt-in `physical_pitch_strength` experiment in the host parity renderer.

The normalized seven-node helper is a **diagnostic/conformance view**, not a claim that every lexical branch in `0x10214f40` maps phone position directly to a uniform 0..6 index. The exact table and the Windows interpolation arithmetic are recovered; the remaining work is reproducing the original branch/feature selection.

## 6. Why production strength remains zero

After recovering that `state+0x130` is a **per-word/per-position** physical-class lattice, M26 was tested with a word-wise table path rather than the earlier whole-utterance approximation. A conservative diagnostic run used strength `0.025`, internal class `0`, and terminal class `1` across the same 22-phrase PC oracle. It slightly improved the F0 contour and container length, but regressed the more important alignment/timing metrics:

- F0-trajectory MAE: `13.4901% -> 13.4540%` (slightly better);
- active correlation: `0.19433 -> 0.19022` (worse);
- active-duration MAE: `5.6908% -> 5.8924%` (worse);
- total-duration MAE: `3.6030% -> 3.4794%` (better).

A small grid over terminal classes 1..6 also failed to produce one globally correct class: different punctuation/lexical cases preferred different rows, exactly as the recovered `0x10215de0` classifier predicts. The table and interpolation path are therefore useful evidence, but hard-coding one row would be an overfit rather than a faithful port. M26 keeps `physical_pitch_strength=0` by default and preserves M25 production audio byte-for-byte.

## Next

M27 should port the remaining **class/branch selection** around `0x10215de0` and `0x10214f40`: in particular, the declarative 1/2 distinction, exclamation 4/5, question 7/8, internal 9..13 punctuation classes, and the runtime condition selecting physical base bytes 18/19/20. Once those selectors are faithful, the physical table can replace more of the reconstructed M24/M25 contour instead of merely being an opt-in probe.
