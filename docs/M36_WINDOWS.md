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

Lengths outside that domain are allocated and resampled by separate fallback
branches; those fallbacks are not yet part of `legacy_window_m36_direct()`.

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
[ floor(0.5 * (1 + cos(pi*n/L)) * 32767), n=0..L-1 ]
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
start = L + floor(zero_slots / 2) - d + 1
```

Expressed against the conceptual concatenation:

```text
W_L || W_A
```

the cached window for `T` is exactly:

```text
(W_L || W_A)[start : start + T]
```

This explains why the original direct table can return a pointer immediately:
the intermediate windows share storage with neighboring anchors rather than
owning separately generated cosine arrays.

## Portable boundary

`include/nicolai/legacy_window_m36.hpp` and
`src/legacy_window_m36.cpp` implement the recovered direct cached domain
`[20,400)`. `tests/legacy_window_m36_test.cpp` locks:

- exact anchor sequence;
- first-anchor descending cosine topology;
- anchor padding at 24;
- packed pointer slicing for 21 and 23;
- 94/115 ownership for requested length 100;
- rejection of the still-unrecovered fallback domain.

The helper is linked into `nicolai_port` but is not used by `StatefulTdsM34` or
production synthesis yet.

## Remaining exact-window work

Before an audio experiment, recover and contract:

1. `0x10109be0` resampling for requested lengths below 20;
2. the symmetric large-length fallback for requested lengths >=400;
3. x87/Q15 last-bit behavior with an original Win32 oracle;
4. only then replace analytic M14 windows in an opt-in stateful experiment and
   rerun the 22-phrase timing/shape/energy audit.
