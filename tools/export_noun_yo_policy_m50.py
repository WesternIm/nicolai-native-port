"""Export PRIVATE noun-form filters from a full-SHA-pinned original as data.

The small inspected initializer instruction subset is decoded, never executed.
No original DLL/server/activation or installed voice file is modified.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib
from export_yo_policy_m49 import ORIGINAL_SHA, extract as verify_original

START = 0x101025de
END = 0x10102770
HELPER = 0x10105a00


def decode_initializer(code):
    # The allocation preceding this lane uses HeapAlloc(HEAP_ZERO_MEMORY).
    # Paradigms 3..99 and 100..202 own distinct 20-byte buffers; empty is known.
    rows = [bytearray(9) for _ in range(200)]
    registers = {}
    frame = []
    assigned = set()
    at = stack_bytes = 0
    while at < len(code):
        if code[at:at+2] in (b'\x8b\x85', b'\x8b\x8d', b'\x8b\x95'):
            register = (code[at+1] >> 3) & 7
            if at+6 > len(code):
                raise ValueError('initializer_mov_bounds')
            offset = struct.unpack_from('<I', code, at+2)[0]
            if offset < 0x3858 or offset > 0x3b74 or (offset-0x384c) % 4:
                raise ValueError('initializer_pointer_bounds')
            registers[register] = ('pointer', (offset-0x384c)//4)
            at += 6
        elif code[at] == 0x6a:
            if at+2 > len(code):
                raise ValueError('initializer_push_bounds')
            frame.append(('value', code[at+1]))
            stack_bytes += 4
            at += 2
        elif 0x50 <= code[at] <= 0x52:
            register = code[at]-0x50
            if register not in registers:
                raise ValueError('initializer_unknown_register')
            frame.append(registers[register])
            stack_bytes += 4
            at += 1
        elif code[at] == 0xe8:
            if at+5 > len(code) or START+at+5+struct.unpack_from('<i',code,at+1)[0] != HELPER:
                raise ValueError('initializer_call_target')
            arguments = list(reversed(frame))
            if (len(arguments)<3 or arguments[0][0]!='pointer' or arguments[-1]!=('value',0)
                    or any(kind!='value' for kind,_ in arguments[1:])):
                raise ValueError('initializer_arguments')
            paradigm = arguments[0][1]
            values = [value for _,value in arguments[1:-1]]
            maximum = 8 if paradigm <= 99 else 6
            if (paradigm in assigned or len(values)>maximum or len(set(values))!=len(values)
                    or any(value<1 or value>maximum for value in values)):
                raise ValueError('initializer_form_values')
            rows[paradigm-3][:1+len(values)] = bytes([len(values),*values])
            assigned.add(paradigm)
            frame = []
            registers = {}  # inspected helper clobbers eax/ecx/edx
            at += 5
        elif code[at:at+2] == b'\x83\xc4':
            if at+3 > len(code) or code[at+2] > stack_bytes or frame:
                raise ValueError('initializer_stack_cleanup')
            stack_bytes -= code[at+2]
            at += 3
        else:
            raise ValueError('unsupported_initializer_instruction')
    if frame or stack_bytes != 56:
        raise ValueError('initializer_final_stack')
    return b''.join(rows), len(assigned)


def extract(image):
    verify_original(image)  # complete SHA / pinned PE / existing selector checks
    nt = struct.unpack_from('<I',image,0x3c)[0]
    sections = struct.unpack_from('<H',image,nt+6)[0]
    optional = nt+24
    optional_size = struct.unpack_from('<H',image,nt+20)[0]
    chunks = []
    rva, size = START-0x10000000, END-START
    for index in range(sections):
        header = optional+optional_size+40*index
        virtual_size,virtual,raw_size,raw = struct.unpack_from('<IIII',image,header+8)
        if virtual<=rva and rva+size<=virtual+min(virtual_size,raw_size):
            offset=raw+rva-virtual
            if offset+size>len(image):
                raise ValueError('initializer_file_bounds')
            chunks.append(image[offset:offset+size])
    if len(chunks)!=1:
        raise ValueError('initializer_section_bounds')
    payload,count = decode_initializer(chunks[0])
    if count != 22:
        raise ValueError('initializer_assignment_count')
    return (b'N50NOUN\0'+bytes.fromhex(ORIGINAL_SHA)+
            struct.pack('<HHI',len(payload),0,zlib.crc32(payload))+payload)


def self_test():
    valid=bytearray()
    for paradigm in range(3,17):
        valid+=b'\x8b\x8d'+struct.pack('<I',0x384c+4*paradigm)+b'\x6a\0\x6a\x01\x51'
        valid+=b'\xe8'+struct.pack('<i',HELPER-(START+len(valid)+5))
    valid+=b'\x83\xc4\x70'
    rows,count=decode_initializer(bytes(valid))
    assert count==14 and all(rows[9*i:9*i+2]==b'\x01\x01' for i in range(14))
    assert not any(rows[9*14:])
    for source in (b'',b'MZ',bytes(1848)):
        try:
            extract(source)
        except ValueError as error:
            assert str(error)=='unsupported_original_sha256'
        else:
            raise ValueError('unsupported_original_accepted')
    # Invented two-form list for an invented initializer, not original rows.
    prefix=b'\x8b\x8d'+struct.pack('<I',0x384c+4*7)+b'\x6a\0\x6a\x05\x6a\x01\x51'
    call=b'\xe8'+struct.pack('<i',HELPER-(START+len(prefix)+5))
    # Leave 56 stack bytes as the inspected entry/exit contract requires.
    padding=b'\x6a\0'*10
    code=prefix+call+padding
    # Uncalled argument frames must never be silently accepted.
    for fixture in (b'',code,b'\x90',prefix+b'\xe8\0\0\0\0'):
        try:
            decode_initializer(fixture)
        except ValueError:
            continue
        raise ValueError('invalid_initializer_accepted')
    print(json.dumps(dict(synthetic_positive_initializers=1,invalid_initializer_refusals=4,
        unsupported_source_refusals=3,executes_original=False)))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test',action='store_true')
    parser.add_argument('--original',type=Path)
    parser.add_argument('--output',type=Path)
    args=parser.parse_args()
    if args.self_test:
        self_test()
    if not args.original:
        if not args.self_test or args.output:
            parser.error('--original and --output are required for export')
        return
    if not args.output:
        parser.error('--output is required for export')
    data=extract(args.original.read_bytes())
    with args.output.open('xb') as output:
        output.write(data)
    print(json.dumps(dict(schema='nicolai-m50-private-noun-export-v1',bytes=len(data),
        source_sha256=ORIGINAL_SHA,export_sha256=hashlib.sha256(data).hexdigest(),
        executes_original=False,proprietary_data_private=True)))


if __name__=='__main__':
    main()
