# M36 continuation — original phone-feature builder

Branch: `m36-original-phone-features`.
Base: merged M35 on `main` at `a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

## Current checkpoint

The first M36 slice implements only the previously traced positive-domain
source-support and duration-Q11 rule from original `0x101a2780`. It adds a
voice-free C++ contract and an independent Python audit for captured scalar
records. Production rendering remains unchanged.

## Next work, in order

1. Create a guarded Win32 local capture harness for the known original DLL
   image (`mtsyc32.dll` identifiers are documented in M34/M35 findings).
2. Capture complete input/output scalar records around `0x101a2780`, beginning
   with no-feature, two-anchor, three-anchor, missing-pitch-anchor and terminal
   next-left cases. Do not commit DLL bytes, disassembly, voice data or raw
   proprietary runtime memory.
3. Validate captured duration records with:

```powershell
python tools/audit_phone_features_m36.py C:\path\to\m36-phone-records.json `
  --output .\metrics-work\m36\phone-duration-audit.json
```

4. From the validated records, recover exact feature record units, pitch-anchor
   repair, pitch lane `+0x1f54`, voicing lane `+0xfb4` and terminal next-left
   interval ownership. Add each rule behind synthetic/original oracle tests
   before connecting it to `StatefulTdsM34`.
5. Only after coefficient-lane parity, proceed to the `0x10107c20` state oracle
   for drop/rewind and saved positions `+0x54/+0x58/+0x5c`, then source
   transitions and exact windows.

## Validation expected at each checkpoint

- host C++ tests pass on Windows and Ubuntu;
- M35 voice-free metric contracts remain green;
- M36 phone-feature voice-free contracts pass;
- original-DLL probes are rerun locally when applicable;
- stable production WAV SHA identity is retained until an explicit promotion
  milestone;
- no phrase-specific or corpus-wide correction is accepted as evidence for an
  unresolved original rule.
