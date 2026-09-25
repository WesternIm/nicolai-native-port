#!/usr/bin/env python3
"""Locate Nicolai's analysis block and opaque compressed acoustic payload.

The locator follows tagged EDAT descriptors; it does not execute any Acapela
binary. Optionally extracts the two byte ranges for local interoperability work.
The extracted proprietary voice data is intentionally not bundled with this repo.
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
HEADER = 0x18


def u32(b: bytes, o: int) -> int:
    return struct.unpack_from('<I', b, o)[0]


def entropy(x: bytes) -> float:
    if not x:
        return 0.0
    c = Counter(x)
    n = len(x)
    return -sum((v / n) * math.log2(v / n) for v in c.values())


def ascii_runs(x: bytes, min_len: int = 4) -> list[str]:
    out: list[str] = []
    cur = bytearray()
    for c in x:
        if 0x20 <= c <= 0x7E:
            cur.append(c)
        else:
            if len(cur) >= min_len:
                out.append(cur.decode('ascii'))
            cur.clear()
    if len(cur) >= min_len:
        out.append(cur.decode('ascii'))
    return out


def descriptor6(b: bytes, o: int):
    if o < 0 or o + 0x18 > len(b):
        return None
    vals = struct.unpack_from('<6I', b, o)
    if vals[0] != 2 or vals[1] != MAGIC or vals[3] != 1 or vals[4] != MAGIC:
        return None
    if vals[2] >= len(b) or vals[5] >= len(b):
        return None
    return {'offset': o, 'payload': vals[2], 'linked': vals[5], 'words': vals}


def tagged_targets(b: bytes) -> list[int]:
    out = set()
    for o in range(0, len(b) - 7, 4):
        if u32(b, o) == MAGIC:
            t = u32(b, o + 4)
            if 0 <= t < len(b):
                out.add(t)
    return sorted(out)


def find_asset_map(b: bytes) -> dict:
    if len(b) < HEADER:
        raise ValueError('file too small')
    s0_size, s1_size, tag0, s0, tag1, s1 = struct.unpack_from('<6I', b, 0)
    s0_end = s0 + s0_size
    s1_end = s1 + s1_size
    if tag0 != MAGIC or tag1 != MAGIC or s0 != HEADER or s0_end != s1 or s1_end > len(b):
        raise ValueError('invalid EDAT layout')

    targets = tagged_targets(b)
    candidates = []
    start = 0
    needle = b'nbr16aci.ana'
    while True:
        path = b.find(needle, start)
        if path < 0:
            break
        start = path + 1
        path_start = path
        while path_start > 0 and 0x20 <= b[path_start - 1] <= 0x7E:
            path_start -= 1
        d = descriptor6(b, path_start - 0x18)
        if not d:
            continue
        a = descriptor6(b, d['payload'])
        if not a:
            continue
        voice_start = a['payload']
        next_targets = [x for x in targets if voice_start < x <= s0_end]
        if not next_targets:
            continue
        voice_end = next_targets[0]
        size = voice_end - voice_start
        if size < 1024 * 1024:
            continue
        candidates.append((size, path_start, d, a, voice_end))

    if not candidates:
        raise ValueError('Nicolai acoustic payload not found')
    size, path, d, a, voice_end = max(candidates)
    analysis_start = a['offset'] + 0x18
    analysis_end = a['payload']
    voice_start = a['payload']
    analysis = b[analysis_start:analysis_end]
    voice = b[voice_start:voice_end]
    next_obj = b[voice_end:min(len(b), voice_end + 0x200)]

    return {
        'ana_path_offset': path,
        'ana_descriptor_offset': d['offset'],
        'analysis_object_offset': a['offset'],
        'seg_object_offset': a['linked'],
        'analysis': {
            'start': analysis_start,
            'end': analysis_end,
            'size': len(analysis),
            'entropy_bits_per_byte': entropy(analysis),
            'sha256': hashlib.sha256(analysis).hexdigest(),
            'head_hex': analysis[:32].hex(),
        },
        'compressed_voice': {
            'start': voice_start,
            'end': voice_end,
            'size': len(voice),
            'entropy_bits_per_byte': entropy(voice),
            'sha256': hashlib.sha256(voice).hexdigest(),
            'head_hex': voice[:32].hex(),
        },
        'next_object': {
            'offset': voice_end,
            'ascii_runs': ascii_runs(next_obj),
            'head_hex': next_obj[:32].hex(),
        },
        'layout': {
            'segment0_offset': s0,
            'segment0_end': s0_end,
            'segment1_offset': s1,
            'segment1_end': s1_end,
        },
    }


def add_hexes(o):
    if isinstance(o, dict):
        out = {}
        for k, v in o.items():
            out[k] = add_hexes(v)
            if k in {'start', 'end', 'offset', 'ana_path_offset', 'ana_descriptor_offset',
                     'analysis_object_offset', 'seg_object_offset', 'segment0_offset',
                     'segment0_end', 'segment1_offset', 'segment1_end'} and isinstance(v, int):
                out[k + '_hex'] = hex(v)
        return out
    if isinstance(o, list):
        return [add_hexes(x) for x in o]
    return o


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('dat', type=Path)
    ap.add_argument('-o', '--output', type=Path, help='write JSON report')
    ap.add_argument('--extract-dir', type=Path,
                    help='extract analysis.bin and compressed_voice.bin (not needed for report)')
    ns = ap.parse_args()

    b = ns.dat.read_bytes()
    report = find_asset_map(b)
    if ns.extract_dir:
        ns.extract_dir.mkdir(parents=True, exist_ok=True)
        a = report['analysis']
        v = report['compressed_voice']
        (ns.extract_dir / 'analysis.bin').write_bytes(b[a['start']:a['end']])
        (ns.extract_dir / 'compressed_voice.bin').write_bytes(b[v['start']:v['end']])

    text = json.dumps(add_hexes(report), ensure_ascii=False, indent=2)
    if ns.output:
        ns.output.write_text(text + '\n', encoding='utf-8')
    else:
        print(text)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
