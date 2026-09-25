#!/usr/bin/env python3
"""Recover Nicolai's variable-length ANA acoustic-range directory.

This tool intentionally never executes the legacy Windows binaries.  It reads
nicolai16.dat, follows the already-recovered EDAT ANA descriptor, and rebuilds
the boundary chain that indexes the compressed acoustic pool.

Optional --extract N writes the Nth compressed range.  The project archive does
not ship any extracted voice material; extraction only operates on a database
the user already has locally.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from collections import Counter
from pathlib import Path

MAGIC = 0xA4F22A56
ANA_NAME = b"nbr16aci.ana"


def u32(b: bytes, o: int) -> int:
    return struct.unpack_from("<I", b, o)[0]


def i32(b: bytes, o: int) -> int:
    return struct.unpack_from("<i", b, o)[0]


def i16s(b: bytes) -> list[int]:
    return list(struct.unpack("<" + "h" * (len(b) // 2), b))


def ascii_start(b: bytes, p: int) -> int:
    while p > 0 and 0x20 <= b[p - 1] <= 0x7E:
        p -= 1
    return p


def descriptor6(b: bytes, o: int):
    if o < 0 or o + 0x18 > len(b):
        return None
    if u32(b, o) != 2 or u32(b, o + 4) != MAGIC:
        return None
    if u32(b, o + 0x0C) != 1 or u32(b, o + 0x10) != MAGIC:
        return None
    return u32(b, o + 8), u32(b, o + 0x14)


def tagged_targets(b: bytes) -> list[int]:
    return sorted({u32(b, o + 4) for o in range(0, len(b) - 7, 4)
                   if u32(b, o) == MAGIC and u32(b, o + 4) < len(b)})


def locate_assets(b: bytes):
    candidates = []
    start = 0
    targets = tagged_targets(b)
    while True:
        p = b.find(ANA_NAME, start)
        if p < 0:
            break
        start = p + 1
        ps = ascii_start(b, p)
        d = descriptor6(b, ps - 0x18)
        if not d:
            continue
        analysis_obj, _rgl_obj = d
        d2 = descriptor6(b, analysis_obj)
        if not d2:
            continue
        voice_start, seg_obj = d2
        nxt = next((x for x in targets if x > voice_start), None)
        if nxt is None or nxt <= voice_start:
            continue
        if nxt - voice_start < 1024 * 1024:
            continue
        candidates.append({
            "ana_path": ps,
            "ana_descriptor": ps - 0x18,
            "analysis_object": analysis_obj,
            "seg_object": seg_obj,
            "analysis_start": analysis_obj + 0x18,
            "analysis_end": voice_start,
            "voice_start": voice_start,
            "voice_end": nxt,
        })
    if not candidates:
        raise RuntimeError("Nicolai ANA/acoustic descriptor not found")
    return max(candidates, key=lambda x: x["voice_end"] - x["voice_start"])


def find_next_start(ana: bytes, p: int, expected: int, scan: int = 1024):
    lim = min(len(ana) - 4, p + scan)
    for q in range(p, lim + 1, 2):
        if i32(ana, q) == expected:
            return q
    return None


def parse_chain(ana: bytes, payload_size: int):
    recs = []
    p = 0
    expected = 0
    while p + 8 <= len(ana):
        st, signed_end = struct.unpack_from("<ii", ana, p)
        if st != expected or st < 0:
            raise RuntimeError(f"boundary chain breaks at ANA+0x{p:X}: {st} != {expected}")
        end = abs(signed_end)
        if not (st < end <= payload_size):
            raise RuntimeError(f"bad compressed range at ANA+0x{p:X}: {st}..{end}")
        q = find_next_start(ana, p + 8, end)
        terminal = q is None
        rec_end = len(ana) if terminal else q
        metadata = i16s(ana[p + 8:rec_end])
        recs.append({
            "index": len(recs),
            "ana_offset": p,
            "compressed_start": st,
            "signed_end": signed_end,
            "compressed_end": end,
            "compressed_size": end - st,
            "record_size": rec_end - p,
            "metadata": metadata,
            "terminal": terminal,
        })
        expected = end
        if terminal:
            break
        p = q
    return recs


def entropy(data: bytes) -> float:
    if not data:
        return 0.0
    n = len(data)
    return -sum((c/n) * math.log2(c/n) for c in Counter(data).values())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("database", type=Path)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--extract", type=int, metavar="UNIT")
    ap.add_argument("--output", type=Path, help="output for --extract")
    args = ap.parse_args()

    b = args.database.read_bytes()
    a = locate_assets(b)
    ana = b[a["analysis_start"]:a["analysis_end"]]
    voice = b[a["voice_start"]:a["voice_end"]]
    recs = parse_chain(ana, len(voice))
    sizes = [r["compressed_size"] for r in recs]

    report = {
        "database": args.database.name,
        "analysis": {
            "offset": a["analysis_start"], "size": len(ana),
            "sha256": hashlib.sha256(ana).hexdigest(),
        },
        "compressed_voice": {
            "offset": a["voice_start"], "size": len(voice),
            "sha256": hashlib.sha256(voice).hexdigest(),
        },
        "unit_index": {
            "count": len(recs),
            "covered_bytes": recs[-1]["compressed_end"],
            "trailing_payload_bytes": len(voice) - recs[-1]["compressed_end"],
            "negative_signed_ends": sum(r["signed_end"] < 0 for r in recs),
            "positive_signed_ends": sum(r["signed_end"] > 0 for r in recs),
            "min_compressed_size": min(sizes),
            "max_compressed_size": max(sizes),
            "mean_compressed_size": sum(sizes) / len(sizes),
            "record_size_histogram": dict(sorted(Counter(r["record_size"] for r in recs).items())),
        },
        "first_records": recs[:12],
        "last_records": recs[-6:],
        "unit0": {
            "sha256": hashlib.sha256(voice[recs[0]["compressed_start"]:recs[0]["compressed_end"]]).hexdigest(),
            "entropy_bits_per_byte": entropy(voice[recs[0]["compressed_start"]:recs[0]["compressed_end"]]),
        },
    }

    out = json.dumps(report, ensure_ascii=False, indent=2)
    if args.json:
        args.json.write_text(out + "\n", encoding="utf-8")
    print(out)

    if args.extract is not None:
        if not (0 <= args.extract < len(recs)):
            raise SystemExit("unit index out of range")
        r = recs[args.extract]
        data = voice[r["compressed_start"]:r["compressed_end"]]
        dst = args.output or Path(f"unit_{args.extract:04d}.cmp16sbi")
        dst.write_bytes(data)
        print(f"extracted {len(data)} bytes -> {dst}")


if __name__ == "__main__":
    main()
