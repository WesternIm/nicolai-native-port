# M16 findings — first independent Russian text frontend

M16 is the first milestone where the public `Engine::synthesize()` accepts
ordinary UTF-8 Russian text and drives the already-portable Nicolai acoustic
renderer without a manually supplied phone sequence.

## What is implemented

Portable C++17 frontend:

- UTF-8 decode and Russian lowercasing;
- word/punctuation boundaries;
- mapping of Russian consonants into Nicolai's 66-phone inventory;
- palatalization for `и`, `е`, `ё`, `ю`, `я`, and `ь` where applicable;
- `ъ` as an iotation boundary;
- word-initial / post-vowel / post-sign `j` insertion for `е/ё/ю/я`;
- neutral legacy vowel variants `a0/o0/u0/y0/e0/i0`;
- multi-word `#` boundary insertion;
- direct integration into `Engine::synthesize()`.

Examples verified against the supplied Nicolai database:

```text
мама      -> # m a0 m a0 #
папа      -> # p a0 p a0 #
яма       -> # j a0 m a0 #
мир       -> # m' i0 r #
привет    -> # p r' i0 v' e0 t #
люблю     -> # l' u0 b l' u0 #
щёлк      -> # sc o0 l k #
молоко    -> # m o0 l o0 k o0 #
мама папа -> # m a0 m a0 # p a0 p a0 #
```

All of the above resolve to existing Nicolai diphones and synthesize through
M15's hybrid SEG / TD-PSOLA renderer.

## Deliberately NOT claimed yet

This is an independent minimal G2P frontend, not a bit-identical reconstruction
of the proprietary SpeechCube Russian linguistic frontend.

M16 does **not** yet restore:

- lexical stress;
- unstressed vowel reduction/allophones;
- full consonant assimilation/devoicing rules;
- number/date/abbreviation expansion;
- original exception dictionary semantics;
- phrase prosody and intonation contours.

The combining acute accent U+0301 is currently accepted but ignored as a future
stress hint. Digits currently return a normalization error instead of being
silently mispronounced.

## Architectural consequence

The Android JNI entry point already calls `Engine::synthesize()`. Therefore M16
moves the existing JNI path from a stub to the same portable text-to-PCM
pipeline on platforms where the project is compiled. ARM64 execution still
requires an actual Android NDK build/device verification; M16 itself was host
validated.
