"""Local scalar controls; synthetic original calls never execute full NLP."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--probe', type=Path, required=True)
    p.add_argument('--voice', type=Path, required=True)
    p.add_argument('--original', type=Path)
    p.add_argument('--report', type=Path, required=True)
    args = p.parse_args()
    controls = {
        'хорошая': 1, 'хороший': 1, 'хорошее': 1, 'хорошие': 1,
        'длинный': 0, 'длинная': 0, 'длинное': 0, 'длинные': 0,
        'длинному': 0, 'длинном': 0, 'длинными': 0,
        'спокойно': 1, 'быстро': 0,
        'проверяем': 2, 'проверяешь': 2, 'проверяют': 2, 'делаем': 0,
        'акустика': 1, 'акустику': 1, 'физику': 0, 'логику': 0,
        'будет': None, 'голоса': None, 'замки': None, 'воды': None,
        '123': None, 'Хорошая': None, 'хорошая!': None, 'мини-хорошая': None,
    }
    with tempfile.TemporaryDirectory(prefix='nicolai-m47-controls-') as root:
        corpus = Path(root) / 'words.tsv'
        corpus.write_text(''.join(f'{i}\t{word}\n' for i, word in enumerate(controls)), encoding='utf-8')
        command = [str(args.probe.resolve()), str(args.voice.resolve()), str(corpus)]
        if args.original: command += ['--original', str(args.original.resolve())]
        result = subprocess.run(command, capture_output=True, timeout=30, check=True)
    seen = set(); values = list(controls.values())
    for line in result.stdout.decode('utf-8').splitlines():
        index, _, _, _, status, stress, _ = line.split('\t'); index = int(index)
        assert index not in seen and 0 <= index < len(values)
        seen.add(index)
        actual = None if stress == '-' else int(stress)
        assert actual == values[index], (list(controls)[index], status, actual, values[index])
        assert (status == 'accepted') == (actual is not None)
    assert len(seen) == len(values)
    exact = reserved = plus = 0
    if args.original:
        oracle = next(s for s in result.stderr.decode().splitlines() if s.startswith('ORIGINAL\t'))
        exact, reserved, plus = map(int, oracle.split('\t')[1:])
        assert (exact, reserved, plus) == (810, 20, 464)
    report = dict(schema='nicolai-m47-local-contracts-v1', controls=len(values),
        accepted_controls=sum(s is not None for s in values),
        declined_controls=sum(s is None for s in values),
        original_exact_candidate_matches=exact, original_reserved_refusals=reserved,
        original_plus_candidate_matches=plus,
        voice_sha256=hashlib.sha256(args.voice.read_bytes()).hexdigest(),
        scope='Synthetic intermediate primitives and fixed ordinary-word controls; no full NLP or perceptual claim.')
    if args.original: report['original_sha256'] = hashlib.sha256(args.original.read_bytes()).hexdigest()
    with args.report.open('x', encoding='utf-8') as output:
        json.dump(report, output, indent=2); output.write('\n')
    print(json.dumps(report))

if __name__ == '__main__': main()
