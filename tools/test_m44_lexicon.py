"""Optional local lexical controls; no proprietary inputs are redistributed."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--voice', type=Path, required=True)
    parser.add_argument('--original', type=Path)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()
    # Ordinary Russian controls, not entries copied from the voice database.
    # A zero-based vowel ordinal is required for the supported stem-stress lane.
    controls = {
        'проверяем': 2, 'проверяешь': 2, 'проверяют': 2, 'делаем': 0,
        'акустика': 1, 'акустику': 1, 'акустики': 1,
        'физику': 0, 'логику': 0, 'руку': 0,
        'молоко': None, 'воды': None, 'звонит': None, 'говорим': None,
        'замки': None, 'проверяемся': None,
        '123': None, 'мини-акустика': None, 'Проверяем': None,
        'проверяем!': None,
    }
    with tempfile.TemporaryDirectory(prefix='nicolai-m44-controls-') as root:
        corpus = Path(root) / 'words.tsv'
        corpus.write_text(''.join(f'{i}\t{word}\n' for i, word in enumerate(controls)), encoding='utf-8')
        command = [str(args.probe.resolve()), str(args.voice.resolve()), str(corpus)]
        if args.original:
            command += ['--original', str(args.original.resolve())]
        result = subprocess.run(command, capture_output=True, timeout=30, check=True)
    expected = list(controls.values())
    seen = set()
    for line in result.stdout.decode('utf-8').splitlines():
        index, status, stress, candidates = line.split('\t')
        index = int(index)
        assert index not in seen and 0 <= index < len(expected)
        seen.add(index)
        actual = None if stress == '-' else int(stress)
        assert actual == expected[index], (list(controls)[index], status, actual, expected[index])
        assert (status == 'accepted') == (actual is not None)
    assert len(seen) == len(expected)
    diagnostics = result.stderr.decode('utf-8').splitlines()
    geometry = next(line for line in diagnostics if line.startswith('LEXICON\t')).split('\t')[1:]
    assert [int(x) for x in geometry] == [68, 63294, 55585]
    original_cells = original_fields = 0
    if args.original:
        oracle = next(line for line in diagnostics if line.startswith('ORIGINAL\t')).split('\t')[1:]
        original_cells, original_fields = map(int, oracle)
        assert (original_cells, original_fields) == (3680, 70)
    report = {
        'schema': 'nicolai-m44-local-contracts-v1',
        'voice_sha256': hashlib.sha256(args.voice.read_bytes()).hexdigest(),
        'controls': len(expected), 'accepted_controls': sum(x is not None for x in expected),
        'declined_controls': sum(x is None for x in expected),
        'original_type_pointer_matches': original_cells,
        'original_stem_candidate_field_matches': original_fields,
        'limits': 'Original checks cover two synthetic intermediate primitives, not full text analysis, phonemes, prosody or PCM.',
    }
    if args.original:
        report['original_sha256'] = hashlib.sha256(args.original.read_bytes()).hexdigest()
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        with args.out.open('x', encoding='utf-8') as stream:
            json.dump(report, stream, indent=2)
            stream.write('\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
