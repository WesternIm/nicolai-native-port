# Port status after M21

## Working without Windows/x86 runtime

- EDAT/static address model
- Nicolai voice/acoustic descriptor discovery
- 66x66 AXM diphone catalog / 2665 units
- deterministic SEG -> ANA unit mapping
- G.711 A-law -> PCM16 decoding
- full M15 SEG voiced/unvoiced grammar
- portable TD-PSOLA/Hann OLA renderer
- UTF-8 Russian frontend
- legacy `exc_rus.txt` pronunciation/stress loader
- legacy `abb_rus.txt` loader
- embedded `duration.par` 64-phone timing profile
- embedded `wordstr.par` 35-float profile extraction
- punctuation-aware word-boundary metadata
- M20 PC-reference timing policy
- M21 normalized `wordstr.par` utterance-position contour
- M21 PC-like terminal output framing
- host arbitrary-text WAV rendering
- Android JNI text -> PCM entrypoints
- Windows-reference capture + batch WAV comparison tooling

## Verified on host

20/20 tests pass. The supplied 22/22 original-PC reference phrases render. Against the current golden reference pack, M21 reaches mean active-duration ratio 0.9966, 5.37% mean absolute active-duration error, and reduces total-WAV duration MAE from M20's 24.39% to 3.06%.

## Not yet certified

- physical Android ARM64 execution on a device/NDK toolchain;
- bit-exact Windows SpeechCube waveform/prosody parity;
- complete original context-dependent `wordstr.par` rule mapping;
- original pitch/intonation contour parity;
- complete `digit1..4.ini` and rusvox morphology behavior.
