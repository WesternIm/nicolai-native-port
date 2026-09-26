# M36 exact-window recovery

This note records the static reconstruction boundary for the original window
cache used by `0x10109be0` and built by callback `0x1010a020`. It remains
separate from synthesis promotion.

## Cache bounds and lookup

Initialization near `0x10057026` writes:

```text
context +0x2bc = 20
context +0x2be = 400
```

and builds the resource through callback `0x1010a020`. The returned packed
resource is materialized to `context +0x2c4`.

`0x10109be0` uses a direct pointer lookup only for:

```text
20 <= requested_length < 400
```

Positive lengths outside that interval use allocation/resampling branches. M36
now reconstructs the guarded runtime domain `1..400`; lengths above 400 remain
outside the portable contract until their full packed-buffer bounds are proven.

## Anchor lengths

`0x1010a020` starts at 20 and repeatedly performs the positive-domain
operation:

```text
next = _ftol(current * 1.23)
```

until the maximum is reached, then inserts 400 explicitly. The resulting anchor
sequence is:

```text
20, 24, 29, 35, 43, 52, 63, 77, 94,
115, 141, 173, 212, 260, 319, 392, 400
```

The relevant constants in the original image decode to:

```text
0x1037b490  double 1.23
0x1037b488  double pi
0x1037b480  float  32767.0
0x1023a2f0  double 0.5
0x1023a310  double 1.0
```

The conversion helper imported at `0x1023a208` is `_ftol`.

## Anchor-window construction

For anchor length `A`, let `L` be the preceding anchor length. The first anchor
uses `L=20`, supplied through the callback range argument.

The packed anchor window has exactly `A` signed WORD samples:

```text
left_pad  = floor((A - L) / 2)
right_pad = A - left_pad - L

[32767 repeated left_pad]
+
[ trunc(0.5 * (1 + cos(pi*n/L)) * 32767), n=0..L-1 ]
+
[0 repeated right_pad]
```

The original stores `pi/L` to a 32-bit float before the cosine loop. The
portable reconstruction mirrors that precision boundary before calling the host
cosine implementation. Exact last-bit identity with x87 `fcos` still requires
a direct-original oracle; the cache topology and integer/padding policy are
statically proven.

## Non-anchor lengths are packed-pointer slices

This is the important difference from the old portable `half_hann(length)`
approximation.

`0x1010a020` does **not** independently compute every length. Between adjacent
anchors `L` and `A`, all intermediate table entries are byte offsets into one
packed WORD area containing the anchor windows.

For requested `T = L + d`, where `L < T < A`:

```text
zero_slots = A - L - 1
start = end_of(W_L) + floor(zero_slots / 2) - d + 1
```

The cached window is `T` WORDs beginning at that packed offset. It may therefore
span an anchor-buffer boundary; reproducing every request by computing a fresh
Hann window loses original pointer/cache behavior.

## Small-length fallback (`1..19`)

At `0x10109c64`, factor starts at 2 and doubles while:

```text
2 + factor * requested_length <= 20
```

The table offset slot is then:

```text
slot = factor * requested_length
```

(where slot zero corresponds to cached length 20). The output copies exactly
`requested_length` WORDs from that packed source pointer using stride `factor`.

Example:

```text
requested 10 -> factor 2 -> slot 20 -> nominal cached length 40
```

and the result is every second WORD from that packed pointer.

## Maximum-length fallback (`400`)

The direct cache comparison is `<400`, so 400 enters the large-length branch.
The original starts factor at 2 and doubles it while:

```text
requested_length / factor + 2 >= 400
```

For 400, factor remains 2 and selects:

```text
slot = 400 / 2 = 200
nominal cached length = 20 + 200 = 220
```

The branch writes the first source WORD, then for factor 2 inserts the signed
integer midpoint of the current and next packed WORD, advances the packed source
index by one, and repeats. The portable helper follows the original signed IDIV
truncation and WORD storage.

The same branch contains behavior for lengths greater than 400, but those cases
can walk far across the shared packed area. M36 deliberately does not claim
that upper domain until its caller/runtime bounds are independently captured.

## Portable boundary

`include/nicolai/legacy_window_m36.hpp` and
`src/legacy_window_m36.cpp` now expose:

- `legacy_window_m36_direct()` for `[20,400)`;
- `legacy_window_m36_lookup()` for the guarded `1..400` domain.

`tests/legacy_window_m36_test.cpp` locks:

- exact anchor sequence;
- first-anchor descending cosine topology;
- anchor padding at 24;
- packed pointer slicing for 21 and 23;
- 94/115 ownership for requested length 100;
- `<20` power-of-two decimation at lengths 10 and 1;
- the exact factor-2 / cached-220 path for requested length 400.

The helper is linked into `nicolai_port` but is not used by `StatefulTdsM34` or
production synthesis yet.

## Remaining exact-window work

Before an audio promotion:

1. verify Q15 last-bit identity against original x87 `fcos` on Win32;
2. capture whether any real runtime caller requests a window above 400;
3. finish the remaining `0x101083b0 / 0x101086c0 / 0x10108cf0` transition
   state so exact windows are applied to the correct source path;
4. then enable the recovered table only in an opt-in stateful experiment and
   rerun the same 22-phrase timing/shape/energy audit.
