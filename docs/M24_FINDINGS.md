# M24 findings — three-point F0 contour

## PC structure recovered

The Windows `mtsyc32.dll` Russian prosody record carries **three signed pitch
values per phonetic item**. In the per-word record, the triplet is stored at:

- `+0x190`
- `+0x194`
- `+0x198`

`0x10213150` copies the three upstream values into that record. Later,
`0x10226cf0` iterates exactly three DWORDs and converts every percent offset to
absolute F0 using the already recovered runtime base:

```
F0 = pitch_base * (1 + 0.01 * percent_offset)
```

For Nicolai, `pitch_base` defaults to **83 Hz** (`pitch.par`, runtime
`+0x81264`). This proves that M23's one-pitch-value-per-diphone renderer was
structurally too coarse even when its median F0 was close.

## Portable change

M24 extends TD-PSOLA with an optional start/middle/end pitch contour. The
synthesis-mark hop is now allowed to vary continuously over a voiced run. The
hybrid SEG renderer maps the unit-level contour into each voiced SEG run, so
mixed voiced/unvoiced units no longer restart the contour blindly at every run.

The exact upstream Russian rule engine that authored the PC triplets has not
been fully recovered yet. M24 therefore adds a **corpus-wide declination
reconstruction**, not phrase-specific exceptions. The selected production
anchors are:

```
start = 1.04
mid   = 0.94
end   = 0.78
```

These multipliers are applied on top of M23's recovered 83-Hz pitch-centering
stage. Setting `NICOLAI_PITCH_DECLINATION_STRENGTH=0` returns the acoustic pitch
path to M23 behavior while retaining the new infrastructure.

## Why a falling contour

Across the 22 original PC WAVs, normalized F0 trajectories show a strong
utterance-level decline. Using 12 equal active-time bins, the median PC curve is
approximately:

```
85.8, 86.6, 91.2, 91.0, 84.3, 83.2,
82.3, 81.4, 72.5, 69.4, 67.1, 63.3 Hz
```

M23 is much flatter and often rises through the middle. M24's production curve
tracks the PC fall substantially more closely.

## Important limitation

The three-point representation and percent-to-Hz conversion are recovered PC
mechanics. The selected declination anchor values are a portable reconstruction
fit to the complete 22-phrase golden corpus. They are not claimed to be the
original hidden upstream PC rule constants.
