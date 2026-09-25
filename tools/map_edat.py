#!/usr/bin/env python3
"""Map the ELAN/Acapela static-data container used by Nicolai 16 kHz.

This is a structural/interoperability probe: it does not execute any installer or
legacy binary. It emits offsets, tagged references, mode records and embedded
resource-path evidence as JSON.
"""
from __future__ import annotations

import argparse
import json
import struct
from collections import Counter
from pathlib import Path

MAGIC = 0xA4F22A56
HEADER = 0x18
MODE_SIZE = 0xB50


def u32(b: bytes, o: int) -> int:
    return struct.unpack_from('<I', b, o)[0]


def region(layout: dict, size: int, o: int) -> str:
    if 0 <= o < HEADER:
        return 'header'
    if layout['segment0_offset'] <= o < layout['segment0_end']:
        return 'segment0'
    if layout['segment1_offset'] <= o < layout['segment1_end']:
        return 'segment1'
    if layout['footer_offset'] <= o < size:
        return 'footer'
    return 'outside'


def ascii_z(b: bytes, o: int, n: int = 128) -> str:
    s = b[o:o+n].split(b'\0', 1)[0]
    return ''.join(chr(c) for c in s if 0x20 <= c <= 0x7e)


def utf16_ascii_z(b: bytes, o: int, nbytes: int = 0x200) -> str:
    out = []
    end = min(len(b), o + nbytes)
    for i in range(o, end - 1, 2):
        c = b[i] | (b[i+1] << 8)
        if c == 0:
            break
        out.append(chr(c) if 0x20 <= c <= 0x7e else '?')
    return ''.join(out)


def find_paths(b: bytes) -> list[dict]:
    results = []
    seen = set()
    low = b.lower()
    start = 0
    while True:
        p = low.find(b'nicolai', start)
        if p < 0:
            break
        a = p
        while a and 0x20 <= b[a-1] <= 0x7e:
            a -= 1
        z = p
        while z < len(b) and 0x20 <= b[z] <= 0x7e and z-a < 1024:
            z += 1
        raw = b[a:z]
        if (b'\\' in raw or b'/' in raw) and a not in seen:
            seen.add(a)
            results.append({'offset': a, 'offset_hex': hex(a), 'value': raw.decode('ascii', 'replace')})
        start = max(p + 1, z)
    return results


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('dat', type=Path)
    ap.add_argument('-o', '--output', type=Path)
    ns = ap.parse_args()
    b = ns.dat.read_bytes()
    if len(b) < HEADER:
        raise SystemExit('file too small')

    h = struct.unpack_from('<6I', b, 0)
    layout = {
        'segment0_size': h[0],
        'segment1_size': h[1],
        'tag0': h[2],
        'segment0_offset': h[3],
        'tag1': h[4],
        'segment1_offset': h[5],
    }
    layout['segment0_end'] = layout['segment0_offset'] + layout['segment0_size']
    layout['segment1_end'] = layout['segment1_offset'] + layout['segment1_size']
    layout['footer_offset'] = layout['segment1_end']
    layout['footer_size'] = len(b) - layout['footer_offset']
    layout['valid'] = (
        layout['tag0'] == MAGIC and layout['tag1'] == MAGIC and
        layout['segment0_offset'] == HEADER and
        layout['segment0_end'] == layout['segment1_offset'] and
        layout['segment1_end'] <= len(b)
    )

    refs = []
    edges = Counter()
    invalid = 0
    for o in range(0, len(b) - 7, 4):
        if u32(b, o) != MAGIC:
            continue
        t = u32(b, o + 4)
        sr, tr = region(layout, len(b), o), region(layout, len(b), t)
        edges[f'{sr}->{tr}'] += 1
        invalid += not (0 <= t < len(b))
        refs.append({'source': o, 'source_hex': hex(o), 'target': t, 'target_hex': hex(t),
                     'source_region': sr, 'target_region': tr})

    modes = []
    s0, s1 = layout['segment1_offset'], layout['segment1_end']
    for s in range(s0, max(s0, s1 - MODE_SIZE + 1)):
        if s + MODE_SIZE > len(b):
            break
        if u32(b, s + 0xAF0) != 16000 or ascii_z(b, s + 0xB00, 64) != 'nicolai16':
            continue
        modes.append({
            'offset': s, 'offset_hex': hex(s),
            'vendor': utf16_ascii_z(b, s + 0x010),
            'engine_name': utf16_ascii_z(b, s + 0x21C),
            'description': utf16_ascii_z(b, s + 0x438),
            'style': utf16_ascii_z(b, s + 0x646, 0x80),
            'voice_name': utf16_ascii_z(b, s + 0x6C6),
            'pitch_name': utf16_ascii_z(b, s + 0x8D2),
            'sample_rate': u32(b, s + 0xAF0),
            'database_name': ascii_z(b, s + 0xB00, 64),
        })

    footer_preview = b[layout['footer_offset']:min(len(b), layout['footer_offset']+128)]
    report = {
        'file': str(ns.dat),
        'size': len(b),
        'size_hex': hex(len(b)),
        'layout': {k: (hex(v) if isinstance(v, int) and k not in {'valid'} else v) for k, v in layout.items()},
        'tagged_ref_count': len(refs),
        'invalid_tagged_targets': int(invalid),
        'edge_counts': dict(sorted(edges.items())),
        'mode_records': modes,
        'resource_paths': find_paths(b),
        'footer_ascii_preview': footer_preview.decode('ascii', 'replace').rstrip(),
    }
    text = json.dumps(report, ensure_ascii=False, indent=2)
    if ns.output:
        ns.output.write_text(text + '\n', encoding='utf-8')
    else:
        print(text)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
