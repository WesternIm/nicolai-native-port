# M18 findings — PC-derived Russian compatibility

## Legacy dictionaries

The supplied `exc_rus.txt` is not merely a stress dictionary. M18 indexes it as
full pronunciation substitutions and preserves legacy stress markup. The file
contains 95,073 usable replacement entries in this build, including 324
multi-word expressions. A separate stress index contains 94,596 single-word
entries.

The supplied `abb_rus.txt` contains 212 usable abbreviation expansions. M18
applies abbreviation rules before generic decimal-integer normalization so
legacy special cases take precedence.

## duration.par

The EDAT static-file record for `rusvox\\data\\duration.par` resolves to the
initialization segment and contains exactly 64 `{phone[4], uint32_ms}` entries.
The x86 parser at `mtsyc32.dll:0x10106690` independently confirms the source
format. M18 consumes this table directly from `nicolai16.dat`.

The duration renderer treats an internal diphone as half of the left phone plus
half of the right phone, so its duration target is `(left_ms + right_ms) / 2`.
Boundary diphones retain source duration until the original boundary/pause policy
is mapped exactly.

## wordstr.par

The static resource resolves to 35 serialized floats. The first 19 fields are
consistent with values that the old parser scaled by `0.01` after textual load;
the final serialized values include `120, 240, 400, 20, 150`. The full object is
exposed by `parse_legacy_russian_wordstr()`, but M18 deliberately does not assign
unproven names to these fields.

## Reference harness

`tools/windows_reference.ps1` captures the original SAPI Nicolai voice to
16-kHz/16-bit/mono WAV. `tools/compare_wav.py` reports duration, RMS, peak,
alignment lag, normalized correlation and exact sample equality. This provides
a deterministic parity loop once the original Windows voice is run.

## Current validation

- 19/19 host tests pass.
- The M18 real-voice probe synthesizes `молоко`, `привет`, `все будет хорошо`,
  `USB 123`, and `мама папа`.
- Generated WAVs are 16-kHz mono PCM16 with zero clipped samples in the probe
  corpus.
- `nicolai_render` successfully synthesizes arbitrary command-line text through
  the M18 pipeline.

## Remaining parity work

Exact Windows parity is not claimed without original-engine reference output.
The largest remaining behavioral surfaces are phrase punctuation/intonation,
legacy `digit1..4.ini` number grammar, remaining rusvox morphology/rules, and
final pitch/duration behavior controlled by the still-unmapped `wordstr.par`
fields.
