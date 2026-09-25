#!/usr/bin/env python3
import argparse, hashlib, json, re, struct
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("voice_db")
p.add_argument("--json", dest="json_path")
a = p.parse_args()
b = Path(a.voice_db).read_bytes()

def u32(o):
    return struct.unpack_from("<I", b, o)[0] if o + 4 <= len(b) else 0

def find(x: bytes):
    o = b.find(x)
    return None if o < 0 else o

ascii_strings = [m.group().decode("latin1") for m in re.finditer(rb"[ -~]{8,}", b)]
keys = ("nicolai", "russian", "russkij", "psola", "diphone", ".dsc", ".rgl", ".axm", ".seg", ".ana")
evidence=[]
for s in ascii_strings:
    if any(k in s.lower() for k in keys) and s not in evidence:
        evidence.append(s)
    if len(evidence) >= 64: break

report = {
    "path": str(Path(a.voice_db)),
    "size_bytes": len(b),
    "sha256": hashlib.sha256(b).hexdigest(),
    "header_word0": f"0x{u32(0):08x}",
    "header_word1": f"0x{u32(4):08x}",
    "tagged_magic_at_8": f"0x{u32(8):08x}",
    "nicolai_offset": find(b"Nicolai"),
    "tempo_psola_offset": find(b"tempo-psola"),
    "russian_data_dir_offset": find(b"Syc.Russkij.DataDir"),
    "sample_rate_hint": 16000 if b"16aci" in b else None,
    "evidence_strings": evidence,
}
print(json.dumps(report, ensure_ascii=False, indent=2))
if a.json_path:
    Path(a.json_path).write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
