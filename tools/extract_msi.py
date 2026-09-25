#!/usr/bin/env python3
"""Extract embedded CAB payloads from this style of MSI without executing it.

Pure Python, no third-party modules. Supports CFB v3/v4 containers and CABs using
no compression or MSZIP. It intentionally does not run registry scripts, EXEs,
or any installer custom actions.
"""
import argparse, os, struct, zlib
from pathlib import Path

FREE=0xffffffff; END=0xfffffffe

def cfb_streams(path):
    b=Path(path).read_bytes()
    if b[:8] != bytes.fromhex('d0cf11e0a1b11ae1'):
        raise ValueError('not an OLE/CFB MSI')
    u16=lambda o: struct.unpack_from('<H',b,o)[0]
    u32=lambda o: struct.unpack_from('<I',b,o)[0]
    sec_size=1<<u16(30); mini_size=1<<u16(32)
    num_fat=u32(44); first_dir=u32(48); cutoff=u32(56)
    first_mini=u32(60); num_mini=u32(64); first_difat=u32(68); num_difat=u32(72)
    def secoff(s): return 512+s*sec_size
    difat=[x for x in struct.unpack_from('<109I',b,76) if x not in (FREE,END)]
    cur=first_difat
    for _ in range(num_difat):
        if cur in (FREE,END): break
        vals=list(struct.unpack_from('<%dI'%(sec_size//4),b,secoff(cur)))
        difat.extend(x for x in vals[:-1] if x not in (FREE,END)); cur=vals[-1]
    fat=[]
    for s in difat[:num_fat]: fat.extend(struct.unpack_from('<%dI'%(sec_size//4),b,secoff(s)))
    def chain(start, table):
        out=[]; seen=set(); s=start
        while s not in (END,FREE) and 0 <= s < len(table) and s not in seen:
            out.append(s); seen.add(s); s=table[s]
        return out
    def read_chain(start,size=None):
        x=b''.join(b[secoff(s):secoff(s)+sec_size] for s in chain(start,fat))
        return x if size is None else x[:size]
    d=read_chain(first_dir); entries=[]
    for i in range(0,len(d),128):
        e=d[i:i+128]
        if len(e)<128: break
        nlen=struct.unpack_from('<H',e,64)[0]
        name=e[:max(0,nlen-2)].decode('utf-16le','replace') if nlen>=2 else ''
        entries.append((name,e[66],struct.unpack_from('<I',e,116)[0],struct.unpack_from('<Q',e,120)[0]))
    root=next((x for x in entries if x[1]==5),None)
    rootdata=read_chain(root[2],root[3]) if root else b''
    minifat=[]
    if num_mini and first_mini not in (END,FREE):
        md=read_chain(first_mini,num_mini*sec_size)
        minifat=list(struct.unpack_from('<%dI'%(len(md)//4),md,0))
    def read_mini(start,size):
        out=[]; seen=set(); s=start
        while s not in (END,FREE) and 0 <= s < len(minifat) and s not in seen:
            seen.add(s); o=s*mini_size; out.append(rootdata[o:o+mini_size]); s=minifat[s]
        return b''.join(out)[:size]
    for name,typ,start,size in entries:
        if typ!=2: continue
        data=read_mini(start,size) if size<cutoff else read_chain(start,size)
        yield name,data

def extract_cab_bytes(b,outdir):
    if b[:4]!=b'MSCF': raise ValueError('not CAB')
    rd=lambda fmt,off: struct.unpack_from(fmt,b,off)
    coffFiles=rd('<I',16)[0]
    cFolders,cFiles,flags,_,_=rd('<HHHHH',26)
    off=36; cbCFHeader=cbCFFolder=cbCFData=0
    if flags & 0x0004:
        cbCFHeader,cbCFFolder,cbCFData=rd('<HBB',off); off+=4+cbCFHeader
    def cstr(pos):
        e=b.index(0,pos); return b[pos:e].decode('latin1','replace'),e+1
    if flags & 1: _,off=cstr(off); _,off=cstr(off)
    if flags & 2: _,off=cstr(off); _,off=cstr(off)
    folders=[]
    for _ in range(cFolders):
        coff,cdata,ctype=rd('<IHH',off); off+=8+cbCFFolder; folders.append((coff,cdata,ctype))
    files=[]; pos=coffFiles
    for _ in range(cFiles):
        size,uoff,ifolder,_,_,_=rd('<IIHHHH',pos); pos+=16
        name,pos=cstr(pos); files.append((name,size,uoff,ifolder))
    streams=[]
    for coff,cdata,ctype in folders:
        pos=coff; data=bytearray(); hist=b''; alg=ctype&0xF
        for _ in range(cdata):
            _,cbData,cbUncomp=rd('<IHH',pos); pos+=8+cbCFData
            chunk=b[pos:pos+cbData]; pos+=cbData
            if alg==0: dec=chunk
            elif alg==1:
                if not chunk.startswith(b'CK'): raise ValueError('bad MSZIP block')
                raw=chunk[2:]
                if hist:
                    d=zlib.decompressobj(-15,zdict=hist[-32768:]); dec=d.decompress(raw)+d.flush()
                else: dec=zlib.decompress(raw,-15)
            else: raise NotImplementedError(f'CAB compression type {alg} is unsupported')
            if len(dec)!=cbUncomp: raise ValueError('CAB block length mismatch')
            data.extend(dec); hist=(hist+dec)[-32768:]
        streams.append(bytes(data))
    outdir=Path(outdir); outdir.mkdir(parents=True,exist_ok=True)
    written=[]
    for name,size,uoff,ifolder in files:
        if ifolder>=len(streams): continue
        payload=streams[ifolder][uoff:uoff+size]
        dest=outdir/os.path.basename(name)
        dest.write_bytes(payload); written.append(dest)
    return written

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('msi'); ap.add_argument('outdir'); a=ap.parse_args()
    out=Path(a.outdir); out.mkdir(parents=True,exist_ok=True)
    cabs=[]
    for _,data in cfb_streams(a.msi):
        if data[:4]==b'MSCF': cabs.append(data)
    if not cabs: raise SystemExit('no embedded CAB streams found')
    all_written=[]
    for i,cab in enumerate(cabs,1):
        cabdir=out/f'cab{i:02d}'
        all_written += extract_cab_bytes(cab,cabdir)
    # Convenience: copy unique payload names to flat/ without executing anything.
    flat=out/'flat'; flat.mkdir(exist_ok=True)
    for src in all_written:
        dest=flat/src.name
        if not dest.exists() or src.stat().st_size>dest.stat().st_size:
            dest.write_bytes(src.read_bytes())
    print(f'CABs: {len(cabs)}')
    print(f'Files: {len(all_written)}')
    print(f'Flat payload: {flat}')

if __name__=='__main__': main()
