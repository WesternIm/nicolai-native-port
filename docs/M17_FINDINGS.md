# M17 findings — stress-aware Russian frontend

M17 extends the independent M16 UTF-8 frontend with lexical stress, vowel
allophone selection, and a conservative obstruent assimilation pass. It does
not claim bit-identical behavior with the proprietary SpeechCube Russian NLP.

## Legacy `exc_rus.txt` stress dictionary

The installer supplied alongside the user's Nicolai package contains the
legacy CP1251 `exc_rus.txt` dictionary. M17 adds a parser for its orthographic
stress notation. Example notation:

```text
абажур  : <абажу<р>
абрикос : <абрико<с>
абрис   : <а<брис>
```

The second `<` follows the stressed vowel. The supplied file parses to:

```text
95,112 input lines
94,596 indexed single-word stress entries
```

The original dictionary is **not included** in this source archive. The code
loads it at runtime when supplied by the user.

Stress priority:

1. combining acute in UTF-8 input (`мо́локо`);
2. `ё`;
3. optional `exc_rus.txt` dictionary;
4. tiny built-in test/fallback lexicon;
5. deterministic last-vowel heuristic.

## Nicolai vowel variants

M10 exposed these native vowel symbols:

```text
a0 a1 a3 a4 a5 A0 A1
u0 u1 u4 U0 U4
i0 i1 i4
y0 y1 y4
o0 o1 O0
e0 e4 E0 E4
```

Their AXM graph topology provides useful constraints. In particular, for the
hard `a` family:

- `a1` links naturally into the following pretonic/full material;
- `a3` links into another reduced-pretonic class;
- `a5` links into post-tonic reduced material;
- `a4` is the word-final reduced form (and has a direct `a4 -> #` path).

M17 therefore uses the following **topology-compatible approximation**:

```text
stressed               -> *0
first pretonic          -> *1 family
remote pretonic a/o     -> a3
post-tonic non-final a/o-> a5
word-final reduced a/o  -> a4
other reduced vowels    -> *4 family where available
```

This mapping is intentionally documented as an independent approximation, not
a recovered historical name/meaning for every suffix.

Examples with the legacy dictionary:

```text
молоко   -> # m a3 l a1 k o0 #
привет   -> # p r' i1 v' E0 t #
сказка   -> # s k a0 s k a4 #
вокзал   -> # v a1 g z a0 l #
подписка -> # p a1 t p' i0 s k a4 #
```

Explicit stress overrides the dictionary:

```text
мо́локо -> # m o0 l a5 k a4 #
```

## Consonant assimilation

M17 applies a conservative regressive obstruent pass using only phone pairs
that exist in Nicolai's inventory:

```text
b/p, d/t, g/k, z/s, zh/sh, v/f
+ palatalized pairs where present
```

It implements final devoicing and adjacent-cluster voicing/devoicing. `в/v`
is excluded as a voicing trigger, matching the common Russian exception.

Examples:

```text
сказка   z+k -> s+k
вокзал   k+z -> g+z
подписка d+p' -> t+p'
```

## Android/JNI

The existing `synthesizePcm16(dbPath,text)` remains available. M17 adds:

```kotlin
synthesizePcm16WithStressDictionary(dbPath, excRusPath, text)
```

so an Android build can package/copy the user's legally obtained `exc_rus.txt`
and use the same stress-aware frontend.

## Verification

Host build:

```text
17 / 17 tests passed
```

Real Nicolai synthesis succeeded for all M17 probe strings. WAV validation
shows zero clipped PCM16 samples. See:

- `m17_probe_output.txt`
- `m17_wav_validation.txt`
- `m17_ctest_output.txt`

## Still not restored

- original SpeechCube Russian NLP rule tables bit-for-bit;
- full morphological stress for words absent from the external dictionary;
- complete coarticulation/assimilation rules;
- number/date/abbreviation expansion (`abb_rus.txt` is a later milestone);
- sentence-level prosody and punctuation timing;
- physical Android ARM64 device validation.
