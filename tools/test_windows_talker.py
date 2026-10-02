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
    parser.add_argument("--baseline-exe", type=Path,
                        help="Optional previous EXE for old-profile PCM isolation")
    parser.add_argument("--baseline-includes-m44", action="store_true",
                        help="Compare M44 too when the baseline EXE already supports it")
    parser.add_argument("--report", type=Path, help="Fresh scalar-only local test report")
    args = parser.parse_args()
    if args.baseline_exe and not args.voice:
        parser.error("--baseline-exe requires --voice and --batch")
    if args.baseline_includes_m44 and not args.baseline_exe:
        parser.error("--baseline-includes-m44 requires --baseline-exe")
    wrapper_pairs = old_profile_pairs = gui_jobs = 0
    subprocess.run([str(args.exe.resolve()), "--ui-smoke"], timeout=15, check=True)
    subprocess.run([str(args.exe.resolve()), "--original-trace-smoke"], timeout=15, check=True)
    with tempfile.TemporaryDirectory(prefix="nicolai-talker-test-") as root:
        # Whitespace, Cyrillic paths, multi-line text and quote-safe argv.
        temp = Path(root) / "Проверка пути с пробелами"
        temp.mkdir()
        records = temp / "linguistics-m46.jsonl"
        for mode in ("--linguistics", "--linguistics-render"):
            worker = subprocess.run([str(args.exe.resolve()), "--m46-capture-worker", "0",
                                     str(records), str(temp / "stop"), mode, "0"],
                                    capture_output=True, timeout=10)
            assert worker.returncode != 0 and not records.exists(), "invalid capture owner accepted"
        text = temp / "текст.txt"
        text.write_text('Привет, мир!\nЁжик читает «текст».', encoding="utf-8")
        result = invoke(args.exe, temp / "missing", text, "stable", temp / "missing.wav")
        assert result.returncode != 0 and not (temp / "missing.wav").exists()
        result = invoke(args.exe, temp, text, "unknown", temp / "bad.wav")
        assert result.returncode != 0 and not (temp / "bad.wav").exists()
        result = subprocess.run([str(args.exe.resolve()), "--render-trace", str(temp),
                                 str(text), "stable", str(temp / "wrong-trace.wav")],
                                capture_output=True, timeout=10)
        assert result.returncode != 0 and not (temp / "wrong-trace.wav").exists()
        assert not records.exists(), "trace CLI accepted a port profile"
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
                "single": "Мама.",
                "startup": "Привет! Это Николай. Проверяем голос и акустику.",
                "initial": "Акусти\u0301ка. Аппара\u0301т. Оборо\u0301на. Огоро\u0301д. А\u0301том. Молоко\u0301.",
                "street": "На улице было тихо. Уговори\u0301л друга выйти.",
                "number": "2026 год",
                "acoustic": "Акустика. Акустику. Физику. Логику.",
                "acoustic-marked": "Аку\u0301стика. Аку\u0301стику. Фи\u0301зику. Ло\u0301гику.",
                "verbs": "Проверяем. Проверяешь. Проверяют. Делаем.",
                "verbs-marked": "Проверя\u0301ем. Проверя\u0301ешь. Проверя\u0301ют. Де\u0301лаем.",
                "m47-stress": "Хорошая. Спокойно. Длинное. Быстро.",
                "m47-stress-marked": "Хоро\u0301шая. Споко\u0301йно. Дли\u0301нное. Бы\u0301стро.",
            }
            corpus = temp / "corpus.tsv"
            corpus.write_text("".join(f"{name}\t{phrase}\n" for name, phrase in cases.items()), encoding="utf-8")
            hashes = {}
            single_word_hashes = {}
            punctuation_hashes = {}
            number_hashes = {}
            profiles = ("stable", "m36-local", "m36-chain", "m38-boundary", "m40-transient", "m41-preserve", "m42-join-pitch", "m43-word-rhythm", "m44-lexicon", "m47-lexicon")
            for profile in profiles:
                subprocess.run([str(args.exe.resolve()), "--ui-job-test", str(args.voice.resolve()), profile],
                               capture_output=True, timeout=70, check=True)
                gui_jobs += 1
                contaminated = dict(os.environ, NICOLAI_M36_TRANSITION_EXECUTOR="1",
                                    NICOLAI_M36_CHAIN_EXECUTOR="1", NICOLAI_M40_BLEND_UNCOVERED="1",
                                    NICOLAI_M41_PRESERVE_RUNS="1", NICOLAI_M41_PRESERVE_JOINS="1",
                                    NICOLAI_M41_INTERPOLATE_UNCOVERED="1",
                                    NICOLAI_M42_JOIN_PERIOD_CONTINUITY="1", NICOLAI_M42_LOCAL_JOIN_PITCH="1",
                                    NICOLAI_M43_WORD_RHYTHM="1", NICOLAI_M43_FIXED_IKA_STRESS="1", NICOLAI_M44_LEXICON_STRESS="1", NICOLAI_M47_LEXICON_STRESS="1", NICOLAI_PC_SEG_TIMELINE="1")
                env = dict(os.environ)
                for name in list(env):
                    if name.startswith("NICOLAI_"):
                        env.pop(name)
                if profile == "m36-local": env["NICOLAI_M36_TRANSITION_EXECUTOR"] = "1"
                if profile == "m36-chain": env["NICOLAI_M36_CHAIN_EXECUTOR"] = "1"
                if profile == "m38-boundary": env["NICOLAI_M38_BOUNDARY_SPEECH_SHARE"] = "0.5"
                if profile == "m40-transient": env["NICOLAI_M40_BLEND_UNCOVERED"] = "1"
                if profile in ("m41-preserve", "m42-join-pitch", "m43-word-rhythm", "m44-lexicon", "m47-lexicon"):
                    env["NICOLAI_M40_BLEND_UNCOVERED"] = "1"
                    env["NICOLAI_M41_PRESERVE_JOINS"] = "1"
                    env["NICOLAI_M41_INTERPOLATE_UNCOVERED"] = "1"
                if profile in ("m42-join-pitch", "m43-word-rhythm", "m44-lexicon", "m47-lexicon"):
                    env["NICOLAI_M42_JOIN_PERIOD_CONTINUITY"] = "0.5"
                    env["NICOLAI_M42_LOCAL_JOIN_PITCH"] = "1"
                if profile in ("m43-word-rhythm", "m44-lexicon", "m47-lexicon"):
                    env["NICOLAI_M43_WORD_RHYTHM"] = "0.25"
                    env["NICOLAI_M43_FIXED_IKA_STRESS"] = "1"
                if profile == "m44-lexicon": env["NICOLAI_M44_LEXICON_STRESS"] = "1"
                if profile == "m47-lexicon": env["NICOLAI_M47_LEXICON_STRESS"] = "1"
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
                    if profile in ("stable", "m38-boundary", "m40-transient", "m41-preserve", "m42-join-pitch", "m43-word-rhythm", "m44-lexicon", "m47-lexicon"):
                        assert joins and all(len(join) == 7 for join in joins), (profile, name)
                        assert all(0 <= int(join[2]) < frames for join in joins), (profile, name)
                        assert all(int(joins[i][2]) <= int(joins[i + 1][2])
                                   for i in range(len(joins) - 1)), (profile, name)
                    else:
                        assert not joins, (profile, name)
                    assert output.read_bytes() == (target / f"{name}.wav").read_bytes(), (profile, name)
                    wrapper_pairs += 1
                    if args.baseline_exe and profile != "m47-lexicon" and (profile != "m44-lexicon" or args.baseline_includes_m44):
                        previous = temp / f"previous-{profile}-{name}.wav"
                        old = invoke(args.baseline_exe, args.voice, text, profile, previous)
                        assert old.returncode == 0, ("old profile failed", profile, name)
                        assert previous.read_bytes() == output.read_bytes(), ("old profile changed", profile, name)
                        old_profile_pairs += 1
                    stress_words = [line.removeprefix("stress_word=").split(",") for line in
                                    result.stdout.decode("utf-8", errors="replace").splitlines()
                                    if line.startswith("stress_word=")]
                    batch_words = [line.split("\t")[2:] for line in batch.stdout.decode("utf-8").splitlines()
                                   if line.startswith(f"W\t{name}\t")]
                    assert stress_words and stress_words == batch_words, (profile, name, "stress diagnostics")
                    if name == "startup":
                        assert stress_words[-1][1:] == (["1", "legacy-lexicon-m47"] if profile == "m47-lexicon" else ["1", "legacy-lexicon-m44"] if profile == "m44-lexicon" else ["1", "dictionary-ika-m43"] if profile == "m43-word-rhythm"
                                                       else ["3", "heuristic"]), (profile, stress_words[-1])
                        assert stress_words[3][1:] == (["2", "legacy-lexicon-m47"] if profile == "m47-lexicon" else ["2", "legacy-lexicon-m44"] if profile == "m44-lexicon" else ["3", "heuristic"]), (profile, stress_words[3])
                    budgets = [line.removeprefix("word_budget=").split(",") for line in
                               result.stdout.decode("utf-8", errors="replace").splitlines()
                               if line.startswith("word_budget=")]
                    if profile in ("m43-word-rhythm", "m44-lexicon", "m47-lexicon"):
                        assert budgets and all(len(word) == 5 for word in budgets), (profile, name)
                        assert all(abs(float(word[2]) - float(word[3])) < 0.02 for word in budgets)
                    else:
                        assert not budgets, (profile, name)
                    if name == "startup":
                        hashes[profile] = hashlib.sha256(output.read_bytes()).hexdigest()
                    if name == "single":
                        single_word_hashes[profile] = hashlib.sha256(output.read_bytes()).hexdigest()
                    if name == "initial":
                        punctuation_hashes[profile] = hashlib.sha256(output.read_bytes()).hexdigest()
                    if name == "number":
                        number_hashes[profile] = hashlib.sha256(output.read_bytes()).hexdigest()
                plain = (target / "acoustic.wav").read_bytes()
                marked = (target / "acoustic-marked.wav").read_bytes()
                assert (plain == marked) == (profile in ("m43-word-rhythm", "m44-lexicon", "m47-lexicon")), (profile, "stress family activation/isolation")
                assert ((target / "verbs.wav").read_bytes() == (target / "verbs-marked.wav").read_bytes()) == (profile in ("m44-lexicon", "m47-lexicon")), (profile, "general verb lexicon activation/isolation")
                assert ((target / "m47-stress.wav").read_bytes() == (target / "m47-stress-marked.wav").read_bytes()) == (profile == "m47-lexicon"), (profile, "M47 actual activation/isolation")
            assert hashes["stable"] != hashes["m36-local"], "experimental mode is accidentally stable"
            assert hashes["stable"] != hashes["m38-boundary"], "boundary experiment is accidentally stable"
            assert single_word_hashes["stable"] == single_word_hashes["m38-boundary"], "single word changed"
            assert punctuation_hashes["stable"] == punctuation_hashes["m38-boundary"], "punctuation boundary changed"
            assert number_hashes["stable"] != number_hashes["m40-transient"], "transient experiment is accidentally stable"
            assert number_hashes["m40-transient"] != number_hashes["m41-preserve"], "preservation experiment is accidentally M40"
            assert hashes["m41-preserve"] != hashes["m42-join-pitch"], "join-pitch experiment is accidentally M41"
            assert hashes["m42-join-pitch"] != hashes["m43-word-rhythm"], "rhythm experiment is accidentally M42"
            assert hashes["m43-word-rhythm"] != hashes["m44-lexicon"], "lexicon experiment is accidentally M43"
            print(f"local voice: startup GUI jobs and {len(cases)} phrases x {len(profiles)} profiles pass; child WAVs match batch byte-for-byte")
    if args.zip:
        with zipfile.ZipFile(args.zip) as package:
            manifest = json.loads(package.read("build.json"))
            names = {"NicolaiTalker.exe", "README.txt", "build.json"}
            if manifest.get("original_trace_launcher_included", False):
                names.add("Trace-Original.cmd")
                assert package.read("Trace-Original.cmd").decode("utf-8").splitlines() == [
                    '@echo off', 'start "" "%~dp0NicolaiTalker.exe" --original-trace']
            assert set(package.namelist()) == names
            assert manifest["proprietary_inputs_included"] is False
            assert manifest["original_sapi_engine_included"] is False
            assert manifest["original_sapi_requires_installed_voice"] is True
            assert manifest["profiles"] == ["stable", "m36-local", "m36-chain", "m38-boundary", "m40-transient", "m41-preserve", "m42-join-pitch", "m43-word-rhythm", "m44-lexicon", "m47-lexicon", "original-sapi"]
            image = package.read("NicolaiTalker.exe")
            nt = int.from_bytes(image[0x3c:0x40], "little")
            assert int.from_bytes(image[nt + 4:nt + 6], "little") == 0x14c
            assert manifest["exe_sha256"] == hashlib.sha256(package.read("NicolaiTalker.exe")).hexdigest()
            assert package.read("NicolaiTalker.exe") == args.exe.read_bytes()
        print("ZIP contents and executable hash verified")
    if args.report:
        report = {
            "schema": "nicolai-m44-wrapper-isolation-v1",
            "exe_sha256": hashlib.sha256(args.exe.read_bytes()).hexdigest(),
            "gui_startup_jobs": gui_jobs,
            "wrapper_batch_pcm_pairs": wrapper_pairs,
            "unchanged_old_profile_pcm_pairs": old_profile_pairs,
            "package_checked": bool(args.zip),
            "limits": "Wrapper and old-profile PCM isolation, not listening superiority or full original NLP parity.",
        }
        if args.baseline_exe:
            report["baseline_exe_sha256"] = hashlib.sha256(args.baseline_exe.read_bytes()).hexdigest()
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open("x", encoding="utf-8") as stream:
            json.dump(report, stream, indent=2)
            stream.write("\n")
    print("Windows test talker contracts passed")


if __name__ == "__main__":
    main()
