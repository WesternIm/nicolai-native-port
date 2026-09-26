"""Record input/code fingerprints and parity dependency versions, not binaries."""
import argparse
import hashlib
import importlib.metadata
import json
import platform
from pathlib import Path


def fingerprint(path):
    return dict(bytes=path.stat().st_size, sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def source_fingerprint(path):
    data = path.read_bytes().replace(b"\r\n", b"\n")
    return dict(lf_normalized_bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("voice_data", type=Path)
    parser.add_argument("reference_pack", type=Path)
    parser.add_argument("--json", type=Path, required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    source_paths = [repo / "CMakeLists.txt"]
    for folder in ("src", "include", "tools", "tests"):
        source_paths.extend(p for p in (repo / folder).rglob("*") if
                            p.is_file() and p.suffix in {".cpp", ".hpp", ".py", ".ps1", ".txt", ".tsv"})
    reference_wavs = sorted(args.reference_pack.rglob("*.wav"))
    if len(reference_wavs) != 22:
        raise ValueError(f"Expected exactly 22 reference WAVs, found {len(reference_wavs)}")
    result = dict(
        python=platform.python_version(), platform=platform.platform(),
        dependencies={name: importlib.metadata.version(name) for name in ("numpy", "librosa", "scipy", "numba")},
        voice_inputs={name: fingerprint(args.voice_data / name) for name in ("nicolai16.dat", "exc_rus.txt", "abb_rus.txt")},
        reference_wavs={str(p.relative_to(args.reference_pack)).replace("\\", "/"): fingerprint(p) for p in reference_wavs},
        source_line_endings="LF normalized; input/audio fingerprints are raw bytes",
        source_files={str(p.relative_to(repo)).replace("\\", "/"): source_fingerprint(p) for p in sorted(source_paths)},
    )
    args.json.parent.mkdir(parents=True, exist_ok=True)
    args.json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("Recorded source/input hashes and dependency versions (no audio or voice data)")


if __name__ == "__main__":
    main()
