# M30 reverse findings — authoring spans and physical pitch state

M30 corrects the scope of the legacy Russian physical-prosody writer around
`mtsyc32.dll:0x10214f40`.  M27-M29 had already recovered most individual
physical-table operations, but still approximated their state on a per-word
basis.  Static tracing in M30 shows that the Windows engine first constructs an
**authoring span** and then runs the marker/pitch writer over that whole span.
All claims below refer to the supplied Nicolai/SpeechCube 5.1 MSI.

## 1. The t-writer operates on authoring spans

`state+0x12c` is an array of four-byte authoring codes.  Starting from the
current outer index, `0x10214f40` scans forward until the code equals literal
`"(/)"`; that position is the end of the current authoring span.

The important arrays are:

- `state+0x12c`: authoring/boundary codes;
- `state+0x13c`: annotated orthographic strings (`char **`);
- `state+0x130`: physical-class bytes.

At `0x1021568b..0x102156c2` the physical class is loaded from
`state+0x130[span_end]`.  One `physical.int` record therefore governs **all
words in that authoring span**.  This is a material correction to the old
per-word approximation.

Portable M30 models the observable sentence-level spans explicitly.  Sentence,
question, exclamation and semicolon closure end a span; comma and colon remain
inside the current span.  The short 22-phrase golden corpus does not require an
approximation of the legacy long-run splitter described below.

## 2. Marker-bearing word count is span-wide

`0x10214fcc..0x10215072` scans every word in the current authoring span and
counts a word when its annotated representation contains either `>` or `<`.
Normal stressed `exc_rus.txt` entries contain those markers, so for ordinary
Russian text this closely follows the number of stressed marker-bearing words
in the span.

That exact span-wide count selects the already recovered base byte:

```
marker-bearing words <= wordstr[24] (2.0) -> physical[18]
marker-bearing words >  wordstr[24]       -> physical[19]
```

## 3. Vowel interpolation is also span-wide

The writer scans all annotated strings in the same authoring span with the
embedded CP866 vowel set:

```
АЕЁИОУЫЭЮЯаеёиоуыэюя
```

The total vowel count and current vowel ordinal passed to the interpolation
helpers are therefore **span-wide**, not word-local.  M30 rewires the recovered
ordinary `<` writer to those span-wide counters.

## 4. Automatic `<<` authoring

If the current span contains no `<`, `0x10215088..0x102150e6` searches for a
word containing `>` and calls the legacy insertion helper at `0x10216870` with
literal `"<<"`.

This proves that `<<` is not required to appear literally in `exc_rus.txt`;
it can be synthesized by the runtime authoring pass.  That explains why the
supplied exception dictionary contains ordinary `<`/`>` stress notation but no
literal `<<` entries.

## 5. Marker normalization occurs before the t-writer

The upstream pass around `0x101a1bf0` also operates on `"(/)"`-bounded spans.
It counts `<`/`>` markers and, when a span lacks usable markers, walks backward
toward the final vowel and inserts a fallback `>` marker (or appends `>` when
no vowel can be selected).

Consequently the physical pitch writer receives normalized annotated spans;
it is not responsible for inventing every basic stress marker from scratch.

## 6. The writer authors l/e/t annotation families

The same span state is used to author three annotation families:

- `[l%d]`;
- `[e%d]`;
- `[t%d]`.

M30 remains focused on the `[t%d]` pitch path, but the shared span/class scope
is important for later duration/energy parity work.

## 7. Dynamic long-span splitting exists but is not guessed

The region around `0x102175xx..0x1021766a` can write literal `"(/)"` into the
authoring-code array.  Separate passes choose blank/space-like split points and
break sufficiently long runs (one observed threshold is around five units,
another branch handles runs greater than six).

The exact upstream units/guards are not yet named well enough to justify a
portable approximation.  The 22 golden phrases are short, so production M30
intentionally does **not** invent a heuristic splitter merely to claim feature
coverage.

## 8. Correction to the old terminal-node wording

M27-M29 described the empirically useful k4 terminal correction more strongly
than the current static evidence warrants.  Re-reading the full span writer
shows that the k4 test depends on the writer's live vowel counter/string
mutation state, and the apparent `total_vowels == current_vowel` test is not
cleanly explained by a simple "last global vowel" model.

The existing 0.016 terminal correction is retained because it improves the
PC golden corpus, but M30 documents it as a **measured legacy proxy pending a
complete state audit**, not as a fully proven semantic label.

## 9. Corrected ordinary-`<` layer now gives an acoustic win

After changing the physical row, marker-word count and vowel interpolation to
span scope, the exact ordinary-`<` layer was re-swept on the same 22 PC WAVs.
Representative points (terminal proxy kept at 0.016):

| ordinary `<` strength | active corr | F0 contour MAE | active-duration MAE |
|---:|---:|---:|---:|
| 0.000 | 0.1953980 | 13.4680% | 5.6830% |
| 0.001 | 0.1963734 | 13.4856% | 5.6790% |
| **0.002** | **0.1965157** | 13.4799% | **5.6790%** |
| 0.003 | 0.1963752 | 13.4825% | 5.6787% |
| 0.004 | 0.1958747 | 13.4977% | 5.6790% |
| 0.005 | 0.1948562 | 13.4939% | 5.6780% |

An independent 12-coefficient MFCC-DTW check also improves from a mean
128.8036 at strength 0 to 128.5899 at 0.002.  The F0-trajectory regression at
0.002 is only about 0.012 percentage point, while waveform alignment, spectral
DTW and active timing all move toward the PC reference.  M30 therefore enables
this recovered layer conservatively at **0.002**.  No phrase-specific cases are
used.

## 10. New portable diagnostic

`legacy_authoring_spans_m30()` exposes the reconstructed span state for tests
and reverse diagnostics: first/last word, span-ending physical class,
marker-bearing-word count and vowel count.  Unit tests cover:

- `что ты делаешь?` -> one span, class 3, three marker-bearing words;
- `привет, Николай!` -> one span, class 4, two marker-bearing words;
- `мама. папа.` -> two authoring spans.

## Next

M31 should move upstream of the t-writer and recover the exact **authoring-code
producer / dynamic `(/)` span splitter**, including how `<<`/`<<<` and the
`[l%d]`/`[e%d]` passes modify the shared state.  Once the span state is exact
for arbitrary long text, progressively replacing the reconstructed M24/M25 F0
backbone with native `physical.int` output becomes much safer.
