"""Synthetic UI/package contracts; optional local voice comparison stays private."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import wave
import zipfile


def invoke(exe, voice, text, profile, output, env=None):
    result = subprocess.run([str(exe.resolve()), "--render", str(voice.resolve()),
                             str(text.resolve()), profile, str(output.resolve())],
                            capture_output=True, timeout=65, env=env)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--voice", type=Path)
    parser.add_argument("--batch", type=Path)
    parser.add_argument("--zip", type=Path)
    parser.add_argument("--check-original", action="store_true")
    args = parser.parse_args()
    subprocess.run([str(args.exe.resolve()), "--ui-smoke"], timeout=15, check=True)
    with tempfile.TemporaryDirectory(prefix="nicolai-talker-test-") as root:
        # Whitespace, Cyrillic paths, multi-line text and quote-safe argv.
        temp = Path(root) / "Проверка пути с пробелами"
        temp.mkdir()
        text = temp / "текст.txt"
        text.write_text('Привет, мир!\nЁжик читает «текст».', encoding="utf-8")
        result = invoke(args.exe, temp / "missing", text, "stable", temp / "missing.wav")
        assert result.returncode != 0 and not (temp / "missing.wav").exists()
        result = invoke(args.exe, temp, text, "unknown", temp / "bad.wav")
        assert result.returncode != 0 and not (temp / "bad.wav").exists()
        output = temp / "existing.wav"
        output.write_bytes(b"keep previous evidence")
        result = invoke(args.exe, temp, text, "stable", output)
        assert result.returncode != 0 and output.read_bytes() == b"keep previous evidence"
        if args.check_original:
            try:
                result = subprocess.run([str(args.exe.resolve()), "--render", str(temp), str(text),
                                         "original-sapi", str(temp / "original.wav")],
                                        capture_output=True, timeout=5)
                print("original SAPI returned", result.returncode,
                      result.stdout.decode("utf-8", errors="replace"),
                      result.stderr.decode("utf-8", errors="replace"))
            except subprocess.TimeoutExpired as error:
                print("original SAPI bounded diagnostic timed out; child cleaned up:",
                      (error.stdout or b"").decode("utf-8", errors="replace"))
        if args.voice:
            if not args.batch:
                raise ValueError("--batch is required for local parity checks")
            # Match the existing A/B path exactly, not a newly invented recipe.
            cases = {
                "test": "Мама мыла раму.",
                "startup": "Привет! Это Николай. Проверяем голос и акустику.",
                "initial": "Акусти\u0301ка. Аппара\u0301т. Оборо\u0301на. Огоро\u0301д. А\u0301том. Молоко\u0301.",
                "street": "На улице было тихо. Уговори\u0301л друга выйти.",
            }
            corpus = temp / "corpus.tsv"
            corpus.write_text("".join(f"{name}\t{phrase}\n" for name, phrase in cases.items()), encoding="utf-8")
            hashes = {}
            for profile in ("stable", "m36-local", "m36-chain"):
                subprocess.run([str(args.exe.resolve()), "--ui-job-test", str(args.voice.resolve()), profile],
                               capture_output=True, timeout=70, check=True)
                contaminated = dict(os.environ, NICOLAI_M36_TRANSITION_EXECUTOR="1", NICOLAI_M36_CHAIN_EXECUTOR="1")
                env = dict(os.environ)
                for name in list(env):
                    if name.startswith("NICOLAI_"):
                        env.pop(name)
                if profile == "m36-local": env["NICOLAI_M36_TRANSITION_EXECUTOR"] = "1"
                if profile == "m36-chain": env["NICOLAI_M36_CHAIN_EXECUTOR"] = "1"
                target = temp / f"batch-{profile}"
                batch = subprocess.run([str(args.batch.resolve()), str(args.voice / "nicolai16.dat"),
                                        str(args.voice / "exc_rus.txt"), str(args.voice / "abb_rus.txt"),
                                        str(corpus), str(target)], capture_output=True, check=True, timeout=65, env=env)
                for name in cases:
                    assert (target / f"{name}.wav").is_file(), (profile, name,
                        batch.stdout.decode("utf-8", errors="replace")[-1200:],
                        batch.stderr.decode("utf-8", errors="replace")[-1200:])
                for name, phrase in cases.items():
                    text.write_text(phrase, encoding="utf-8")
                    output = temp / f"{profile}-{name}.wav"
                    result = invoke(args.exe, args.voice, text, profile, output, contaminated)
                    assert result.returncode == 0, (profile, name, result.stderr.decode("utf-8", errors="replace"))
                    with wave.open(str(output)) as wav:
                        assert (wav.getnchannels(), wav.getsampwidth(), wav.getframerate()) == (1, 2, 16000)
                        assert wav.getnframes() > 0
                        frames = wav.getnframes()
                    joins = [line.removeprefix("join=").split(",") for line in
                             result.stdout.decode("utf-8", errors="replace").splitlines()
                             if line.startswith("join=")]
                    if profile == "stable":
                        assert joins and all(len(join) == 7 for join in joins), (profile, name)
                        assert all(0 <= int(join[2]) < frames for join in joins), (profile, name)
                        assert all(int(joins[i][2]) <= int(joins[i + 1][2])
                                   for i in range(len(joins) - 1)), (profile, name)
                    else:
                        assert not joins, (profile, name)
                    assert output.read_bytes() == (target / f"{name}.wav").read_bytes(), (profile, name)
                    if name == "test":
                        hashes[profile] = hashlib.sha256(output.read_bytes()).hexdigest()
            assert hashes["stable"] != hashes["m36-local"], "experimental mode is accidentally stable"
            print("local voice: startup GUI job and 4 phrases x 3 profiles pass; child WAVs match batch byte-for-byte")
    if args.zip:
        with zipfile.ZipFile(args.zip) as package:
            assert set(package.namelist()) == {"NicolaiTalker.exe", "README.txt", "build.json"}
            manifest = json.loads(package.read("build.json"))
            assert manifest["proprietary_inputs_included"] is False
            assert manifest["original_sapi_engine_included"] is False
            assert manifest["original_sapi_requires_installed_voice"] is True
            assert manifest["profiles"] == ["stable", "m36-local", "m36-chain", "original-sapi"]
            image = package.read("NicolaiTalker.exe")
            nt = int.from_bytes(image[0x3c:0x40], "little")
            assert int.from_bytes(image[nt + 4:nt + 6], "little") == 0x14c
            assert manifest["exe_sha256"] == hashlib.sha256(package.read("NicolaiTalker.exe")).hexdigest()
            assert package.read("NicolaiTalker.exe") == args.exe.read_bytes()
        print("ZIP contents and executable hash verified")
    print("Windows test talker contracts passed")


if __name__ == "__main__":
    main()
