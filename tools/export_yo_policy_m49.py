"""Export PRIVATE selector data from a pinned local original; never execute it.

Only standard Python is required. Outputs must stay with local licensed voice
inputs, not in Git or the public test package. This is not an activation tool.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib

ORIGINAL_SHA = 'f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7'


def extract(image):
    if hashlib.sha256(image).hexdigest() != ORIGINAL_SHA:
        raise ValueError('unsupported_original_sha256')
    nt = struct.unpack_from('<I', image, 0x3c)[0]
    if image[nt:nt+4] != b'PE\0\0':
        raise ValueError('invalid_pe')
    machine, sections, timestamp = struct.unpack_from('<HHI', image, nt+4)
    optional = nt+24
    optional_size = struct.unpack_from('<H', image, nt+20)[0]
    if (machine, timestamp, struct.unpack_from('<I', image, optional+28)[0],
            struct.unpack_from('<I', image, optional+56)[0]) != (0x14c, 0x412a0cb4, 0x10000000, 0x797000):
        raise ValueError('unsupported_original_pe')
    rva, size = 0x4e8530, 8*61
    chunks = []
    for index in range(sections):
        header = optional+optional_size+40*index
        virtual_size, virtual, raw_size, raw = struct.unpack_from('<IIII', image, header+8)
        if virtual <= rva and rva+size <= virtual+min(virtual_size, raw_size):
            start = raw+rva-virtual
            if start+size > len(image):
                raise ValueError('selector_file_bounds')
            chunks.append(image[start:start+size])
    if len(chunks) != 1:
        raise ValueError('selector_section_bounds')
    payload = chunks[0]
    for kind in range(8):
        for lane in range(2):
            start = 61*kind+30*lane
            length = payload[start]
            if length > 29 or any(payload[start+length+1:start+30]):
                raise ValueError('selector_list_bounds')
            members = payload[start+1:start+length+1]
            if (len(set(members)) != length or (lane and any(v >= 20 for v in members))
                    or (not lane and 0 in members)):
                raise ValueError('selector_values')
    return b'N49YOv1\0'+bytes.fromhex(ORIGINAL_SHA)+struct.pack('<HHI', size, 0, zlib.crc32(payload))+payload


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--original', type=Path)
    parser.add_argument('--output', type=Path, help='Fresh PRIVATE path beside local voice data')
    args = parser.parse_args()
    if args.self_test:
        for image in (b'', b'MZ', bytes(536), b'PE\0\0'+bytes(4096)):
            try:
                extract(image)
            except ValueError as error:
                if str(error) != 'unsupported_original_sha256':
                    raise
            else:
                raise ValueError('unsupported_original_accepted')
        print(json.dumps(dict(unsupported_source_refusals=4, executes_original=False)))
    if not args.original:
        if not args.self_test or args.output:
            parser.error('--original and --output are required for export')
        return
    if not args.output:
        parser.error('--output is required for export')
    data = extract(args.original.read_bytes())
    with args.output.open('xb') as output:
        output.write(data)
    print(json.dumps(dict(schema='nicolai-m49-private-policy-export-v1',
        bytes=len(data), source_sha256=ORIGINAL_SHA, export_sha256=hashlib.sha256(data).hexdigest(),
        executes_original=False, proprietary_data_private=True)))


if __name__ == '__main__':
    main()
