# M21 findings — wordstr pacing + PC terminal framing

M21 is the third conformance pass against the supplied original Windows
**ELAN TTS Russian (Nicolai 16Khz)** golden reference corpus.

## 1. Recovered wordstr utterance-position contour

The original `mtsyc32.dll` routine around `0x10226cf0` iterates a word-like
outer structure and constructs a three-point contour from the exact
`wordstr.par` values:

- index 7  / +0x1c = **0.90**
- index 12 / +0x30 = **0.85**
- index 17 / +0x44 = **1.00**

The DLL computes two slopes around the half-way point of the outer-item count.
That proves the three values are not independent constants: they form a
position-dependent utterance contour.

The same legacy routine only applies the contour under feature flags that are
not fully named yet. Applying the raw contour unconditionally therefore hurts
parity. M21 keeps the recovered *shape*, normalizes it over the words in an
utterance so it does not change corpus-wide speaking rate, and uses a
conservative strength of **0.25**. This setting was selected by golden-corpus
comparison; it improves both duration MAE and mean alignment correlation while
leaving one-word phrases unchanged.

## 2. Original PC output contains a terminal quiet flush

The supplied PC WAV files contain substantially more trailing silence than M20.
Across the 22 phrases M20 ends on average **306.4 ms too early** after the last
active sample.

The recovered `wordstr.par` tail contains the integer-scale values
`120, 240, 400, 20, 150`. The old DLL has separate pause/prosody construction
paths using this tail. A **two-quantum 150 ms terminal flush (300 ms)** matches
the supplied PC corpus closely. This is implemented as an explicit conformance
hypothesis, not claimed as a fully named legacy branch yet.

After adding the 300 ms quiet flush:

- mean trailing-silence absolute error: **306.4 ms -> 16.4 ms**
- total-WAV duration MAE: **24.39% -> 3.06%**
- total-WAV duration RMSE: **26.00% -> 3.94%**
- mean total-duration ratio: **0.7561 -> 0.9978**

## 3. Active-speech parity

With the conservative position contour (strength 0.25):

- mean active-duration ratio: **0.9945 -> 0.9966**
- active-duration MAE: **5.437% -> 5.373%**
- mean best-lag waveform correlation: **0.1900 -> 0.1925**

The active-duration RMSE is essentially unchanged because the remaining large
errors are concentrated in a few single words (`папа`, `сказка`, etc.). Those
cannot be solved by an utterance-position contour: the original DLL has
additional per-phone/per-syllable feature flags selecting coefficients such as
0.8, 0.6, 0.5, 0.97, 0.99, 1.3, 1.2 and 0.25.

## Next target

M22 should map the helper outputs around `0x102381d0/0x102382d0` and the internal
feature bytes used by `0x10226cf0`. That is the path to replacing the remaining
single-word duration errors with the original feature-driven rules rather than
phrase-specific constants.
