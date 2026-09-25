# M27 reverse findings — physical-prosody classifier and exact terminal t-node

M27 continues the Windows Nicolai parity work against the same supplied MSI and
`reference_pack_20260924_222948` golden corpus.  The goal is not to tune words
individually; every change below comes from `mtsyc32.dll`/`physical.int` and is
applied generically.

## 1. `0x10215de0` physical-class classifier

The classifier writes one signed physical-prosody class byte per position into
`state+0x130`.  The following branches are now recovered sufficiently to port:

- exact terminal `"."` and `";"` enter the declarative branch;
- declarative class **2** is selected only when the proprietary suffix marker
  `<<<` is present; otherwise class **1** is used;
- terminal `?` searches three embedded CP866 WH-word lists.  A WH match selects
  class **3**;
- a non-WH question selects class **7** or **8** from a morphology/POS-byte
  lattice.  The current portable frontend does not expose that old POS byte, so
  M27 uses class 8 as an explicit conservative fallback rather than guessing by
  phrase;
- exact `...` selects class **6** in the PC engine;
- other explicit terminal signs use class **5** for a WH lexeme and **4**
  otherwise;
- internal `:` and `)` select class **10**;
- internal `,` and `(` select class **12**;
- `_`, `,_` and space feed context branches producing classes 9..13.

The recovered WH vocabulary is copied from the literal tables at
`0x104e8808`, `0x104e88f0`, and `0x104e89e0`: forms of Кто, Что/legacy Што,
Сколько, Какой, Который, Каков, Как, Зачем, Почему, Где, Когда, Неужели.

The portable M27 class probe therefore produces, for example:

```
что ты делаешь?    -> 0 0 3
привет, Николай!   -> 12 4
я пришёл домой.    -> 0 0 1
когда домой?       -> 0 3
ты дома?           -> 0 8   (portable fallback for the still-missing 7/8 POS bit)
```

No corpus ID, phrase string, or individual golden word is special-cased.

## 2. Important correction to the M26 interpretation of `physical.int`

M26 correctly recovered the 14 x 42 signed-byte table, but its normalized
seven-node helper was deliberately diagnostic.  M27 resolves the actual layout
used by the `[t%d]` writer more precisely.

For one 42-byte record, the pitch subsection beginning at byte 18 is:

- byte 18: one start/base variant;
- byte 19: the alternate start/base variant;
- byte 20: the end/base value;
- bytes 21..27: seven **context additives** for pitch lane 0;
- bytes 28..34: seven context additives for pitch lane 1;
- bytes 35..41: seven context additives for pitch lane 2.

The 7-value groups are therefore **not seven uniformly spaced time nodes**.
They are context alternatives chosen by the marker/word branch in
`0x10214f40`.

## 3. Exact interpolation helpers

`0x10215d30` and `0x10215d90` are now ported as exact arithmetic helpers.
For `count > 1`, `0x10215d30` computes:

```
additive + start + (end - start) * position / (count - 1)
```

and `0x10215d90` computes:

```
start + (end - start) * position / (count - 1)
```

The result passes through MSVCRT `_ftol`, i.e. conversion/truncation toward
zero.  For `count == 1` the first helper returns `additive + start`, and the
second returns `start`.

This explains the repeated calls in the PC writer much better than M26's
uniform normalized-table interpretation.

## 4. Fully recovered final t-node

One branch is already unambiguous and does not depend on the still-unported
internal marker selector: the final/special t-node is authored as

```
lane0 = record[20] + record[25]
lane1 = record[20] + record[32]
lane2 = record[20] + record[39]
```

For declarative class 1 in the supplied Nicolai `physical.int`, that evaluates
to `-30,-30,-30` pitch-percent units before the already recovered 83-Hz and
`wordstr[28]` stages.

M27 therefore separates this proven terminal node from the incomplete interior
physical table.  The interior `physical_pitch_strength` remains **0** by
default, while the exact terminal node has its own small production blend.

## 5. Production choice

A grid against all 22 PC oracle WAVs tested only the exact terminal-node blend.
The best joint result for raw active correlation and F0 trajectory was around
`0.016` (1.6%).  This is deliberately weak: the existing M24/M25 contour remains
primary until the remaining `0x10214f40` marker/context selector is reproduced.

M27 defaults:

```
physical_pitch_strength          = 0.0
physical_terminal_pitch_strength = 0.016
```

Thus M27 uses the recovered PC table only where its authoring rule is known,
instead of letting the still-wrong interior table interpolation damage the
voice.

## Next

M28 should port the marker/context selector inside `0x10214f40`.  In particular,
it must reproduce which of bytes 21..27 / 28..34 / 35..41 is selected and when
byte 18 versus 19 is used as the start/base value.  Once that is faithful, the
interior physical-prosody strength can rise above zero and replace more of the
reconstructed M24/M25 contour.
