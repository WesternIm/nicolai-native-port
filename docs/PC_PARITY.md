# PC parity workflow

M18 is a **PC-derived compatibility milestone**, not a claim of bit-exact parity
with the Windows SpeechCube executable.

The portable path now consumes several original Russian resources/data layouts:

- `exc_rus.txt` pronunciation/stress replacements;
- `abb_rus.txt` abbreviation expansions;
- the embedded `duration.par` table (64 phone-duration entries);
- the embedded 35-float `wordstr.par` profile (parsed and exposed for further
  parity work);
- the original Nicolai AXM/SEG/ANA acoustic database.

The current container does not have Wine/32-bit Windows Speech API available, so
an original Windows Nicolai process cannot be executed here. Exact parity must
therefore be measured on a Windows machine that has the original Nicolai SAPI
voice installed.

## Produce a Windows reference

Open Windows PowerShell in this project's directory:

```powershell
powershell -ExecutionPolicy Bypass -File tools/windows_reference.ps1 `
  -Text "Привет, мама!" `
  -OutWav .\reference_privet_mama.wav
```

The script selects a SAPI voice whose description contains `Nicolai` and writes
16 kHz / 16-bit / mono PCM (SAPI audio format type 18).

## Produce the portable WAV

Use `nicolai_m18_probe` for the built-in corpus, or call `Engine::synthesize()`
from a small host/Android harness with the same exact text and dictionaries.

## Compare

```bash
python tools/compare_wav.py reference_privet_mama.wav portable_privet_mama.wav
```

The comparison reports duration, RMS/peak, best alignment lag, normalized
correlation, and exact sample equality. Remaining mismatches should be used to
reverse the specific legacy prosody/normalization rule instead of tuning by ear.

## Known non-parity items after M18

1. The exact semantic mapping of all 35 `wordstr.par` fields is not yet proven.
2. Sentence-level pause and intonation rules (`Dut_HLP_punct`, `Lng_HLP_punct`,
   `Lng_HLP_pause`) are not yet replicated bit-for-bit.
3. Number grammar is currently a deterministic Russian cardinal implementation
   after `abb_rus.txt`; the original `digit1..4.ini` machinery remains to be
   reconstructed for full equivalence.
4. Some morphology/phonological choices still use the independent M17 rules
   where the original rusvox rule path has not yet been mapped.
5. Physical ARM64/Android execution still needs an Android NDK/device parity run.

These are now concrete parity targets, not unknown acoustic-format work.
