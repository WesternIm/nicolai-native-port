# M19 — first PC-reference parity pass

M19 is the first portable-core milestone calibrated against a WAV pack produced by the
installed original 32-bit SAPI5 `ELAN TTS Russian (Nicolai 16Khz)`.

Changes:
- add a distinct final PC-reference gain stage (2.0x, saturating PCM16) rather than
  corrupting G.711 or PSOLA math;
- add builtin stress for `это` (stress on the first vowel), fixing the regression
  `missing_diphone_#_e4` seen in the 22-phrase PC reference corpus;
- add `compare_reference_pack.py` for batch active-speech timing/level/alignment reports.

Important: endpoint silence from the SAPI reference is measured separately and is not
baked into the acoustic core yet. The reference contains a roughly fixed terminal pad,
but it has not yet been proven whether that belongs to SpeechCube prosody, SAPI stream
framing, or the server output layer.
