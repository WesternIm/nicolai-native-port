# M36 — original phone-feature recovery, first slice

Base: merged M35 on `main`, merge commit
`a9d24018a8b84a9982b2ab8ab4dddb381dbe5002`.

This checkpoint starts recovery of the original `0x101a2780` phone-feature /
coefficient builder. It deliberately does **not** change production synthesis,
stateful synthesis policy, pitch/voicing policy, windows, source selection or
Android behavior.

## Proven slice implemented

Prior tracing established that the original builder spans source support over
previous-right plus next-left:

```text
(previous.last - previous.split) + (next.split - next.first)
```

For the positive feature-duration path, the duration coefficient is:

```text
duration_q11 = (total_feature_duration << 11) / combined_source_support
```

with integer truncation and a positive zero-result fallback to `1`.

M36 now exposes only that supported domain through
`legacy_phone_duration_m36()` in `legacy_phone_features.{hpp,cpp}`. Invalid
geometry, non-positive feature duration and integer results outside the portable
`int` domain are rejected rather than assigning unproven x86 wrap/fault
semantics.

A new portable `legacy_phone_features_test` covers source ownership, unity and
fractional Q11 scaling, the positive zero-result fallback and guarded invalid
cases. The implementation is linked into `nicolai_port` but is not called by
`Engine`, `StatefulTdsM34` or any renderer.

## Offline original-record audit

`tools/audit_phone_features_m36.py` accepts a local JSON record list (or an
object containing `records[]`). Each record contains:

```json
{
  "id": "case-id",
  "previous_split": 80,
  "previous_last": 120,
  "next_first": 200,
  "next_split": 260,
  "feature_duration_sum": 100,
  "observed_support": 100,
  "observed_duration_q11": 2048
}
```

`observed_support` is optional. The audit independently reconstructs the proven
support and duration rule and reports matches, mismatches and invalid captures.
It intentionally does not infer pitch, voicing or terminal ownership. The
voice-free contract `tools/test_m36_phone_features.py` runs in CI alongside the
M35 metric contracts.

Example local use:

```powershell
python tools/audit_phone_features_m36.py C:\path\to\m36-phone-records.json `
  --output .\metrics-work\m36\phone-duration-audit.json
```

Original DLLs, disassembly, voice data, captured raw runtime memory and derived
proprietary feature caches remain local/untracked.

## Still missing before substitution into synthesis

The following parts of `0x101a2780` are **not recovered by this slice**:

- exact supported 2/3-point feature record layout and units;
- missing-pitch-anchor repair and interpolation ownership;
- voicing lane construction at descriptor `+0xfb4`;
- exact pitch lane producer at descriptor `+0x1f54`;
- terminal next-left interval adjustment;
- packaging interactions before `0x1010d330`;
- runtime drop/rewind and buffered selection in `0x10107c20`.

Do not replace existing portable phone hints with M36 data until original
runtime captures validate those rules. In particular, do not use corpus-level
duration retuning as a substitute for the original feature builder.

## Next exact step

Build a guarded Win32 local capture harness around the supported original DLL
image and capture the input/output phone records surrounding `0x101a2780`.
The first useful corpus should include no-feature, two-anchor, three-anchor,
missing-pitch-anchor and terminal next-left cases. Feed the scalar duration
fields into `audit_phone_features_m36.py`; only after those records agree should
M36 grow the pitch/voicing and terminal rules.
