"""Read-only x86 DLL inspection. Requires local pefile and capstone packages.

No DLL, byte dump, or generated disassembly is intended for version control.
Addresses are virtual addresses at the PE's preferred image base.
"""
import argparse
import struct
from pathlib import Path

import capstone
import pefile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dll", type=Path)
    parser.add_argument("address", type=lambda s: int(s, 0))
    parser.add_argument("--size", type=lambda s: int(s, 0), default=0x200)
    parser.add_argument("--xrefs", action="store_true")
    args = parser.parse_args()
    pe = pefile.PE(str(args.dll))
    base = pe.OPTIONAL_HEADER.ImageBase
    dis = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    dis.skipdata = True
    if args.xrefs:
        for section in pe.sections:
            if not section.Characteristics & 0x20000000:
                continue
            # Embedded data/jump tables can desynchronise a linear sweep.
            # Scan rel32 encodings, then decode each candidate independently.
            data = section.get_data()
            for offset in range(len(data) - 4):
                if data[offset] not in (0xE8, 0xE9):
                    continue
                address = base + section.VirtualAddress + offset
                target = address + 5 + struct.unpack_from("<i", data, offset + 1)[0]
                if target == args.address:
                    ins = next(dis.disasm(data[offset:offset + 5], address))
                    print(f"{ins.address:08x} {ins.mnemonic:8} {ins.op_str}")
    else:
        for ins in dis.disasm(pe.get_data(args.address - base, args.size), args.address):
            print(f"{ins.address:08x} {ins.mnemonic:8} {ins.op_str}")


if __name__ == "__main__":
    main()
