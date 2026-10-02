"""Private end-to-end refusal checks; never modify the caller's voice files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--batch', type=Path, required=True)
    parser.add_argument('--voice', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--profile',choices=('m49-lexicon','m50-lexicon'),default='m49-lexicon')
    args = parser.parse_args()
    if args.report.exists():
        raise ValueError('fresh_report_required')
    env = {key: value for key, value in os.environ.items() if not key.startswith('NICOLAI_')}
    refusals = 0
    with tempfile.TemporaryDirectory(prefix='nicolai-m49-policy-') as root:
        directory = Path(root)
        for name in ('nicolai16.dat', 'exc_rus.txt', 'abb_rus.txt'):
            shutil.copyfile(args.voice/name, directory/name)
        text = directory/'input.txt'
        text.write_text('Мама.', encoding='utf-8')
        corpus = directory/'corpus.tsv'
        corpus.write_text('test\tМама.\n', encoding='utf-8')
        m50=args.profile=='m50-lexicon'
        if m50:
            shutil.copyfile(args.voice/'nicolai-yo-m49.bin',directory/'nicolai-yo-m49.bin')
        # The previous profile must work without the new private file.
        old = directory/'m48.wav'
        subprocess.run([str(args.exe.resolve()), '--render', str(directory), str(text),
                        'm49-lexicon' if m50 else 'm48-lexicon', str(old)], env=env, capture_output=True, timeout=20, check=True)
        assert old.is_file()
        policy = directory/('nicolai-noun-yo-m50.bin' if m50 else 'nicolai-yo-m49.bin')
        good = (args.voice/policy.name).read_bytes()
        corrupt = bytearray(good)
        corrupt[44] ^= 1
        cases = ((None, 'missing_local_noun_yo_policy' if m50 else 'missing_local_yo_policy'),
                 (b'bad', 'noun_yo_policy_header' if m50 else 'yo_policy_header'),
                 (bytes(corrupt), 'noun_yo_policy_checksum' if m50 else 'yo_policy_checksum'))
        for index, (payload, error) in enumerate(cases):
            if payload is not None:
                policy.write_bytes(payload)  # owned disposable fixture only
            output = directory/f'refused-{index}.wav'
            child = subprocess.run([str(args.exe.resolve()), '--render', str(directory), str(text),
                args.profile, str(output)], env=env, capture_output=True, timeout=20)
            assert child.returncode != 0 and not output.exists()
            assert error.encode() in child.stdout+child.stderr, (index, child.stderr)
            batch = subprocess.run([str(args.batch.resolve()), str(directory/'nicolai16.dat'),
                str(directory/'exc_rus.txt'), str(directory/'abb_rus.txt'), str(corpus),
                str(directory/f'batch-{index}')], env=dict(env,**{'NICOLAI_M50_LEXICON_STRESS' if m50 else 'NICOLAI_M49_LEXICON_STRESS':'1'}),
                capture_output=True, timeout=20)
            assert batch.returncode != 0 and error.encode() in batch.stdout+batch.stderr
            assert not list((directory/f'batch-{index}').glob('*.wav'))
            refusals += 2
    report = dict(schema='nicolai-yo-policy-failure-v2',profile=args.profile,runtime_refusals=refusals,
        previous_profile_without_policy=True, caller_files_modified=False,
        exe_sha256=hashlib.sha256(args.exe.read_bytes()).hexdigest())
    with args.report.open('x', encoding='utf-8') as output:
        json.dump(report, output, indent=2)
        output.write('\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
