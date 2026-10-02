"""PRIVATE real-capture/actual-M44-renderer stress alignment, scalar reports only.

Unmarked original words are counted separately: a port lexical stress index
is NOT itself a measurement of perceived prominence. Original marker groups
and splitter codes are diagnostic evidence, not an implemented acoustic fix.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess

from audit_m46_linguistics import audit, require

VOWELS = frozenset('аеёиоуыэюя')


def annotation(value):
    text = bytes.fromhex(value).decode('cp866').lower()
    plain = text.replace('<', '').replace('>', '')
    # Do not align phonetic/bracket overrides or multiple separated primary
    # markers as if they were ordinary orthographic words.
    require(plain and all('а' <= c <= 'я' or c == 'ё' for c in plain), 'non_orthographic_word')
    markers = list(re.finditer('<+', text))
    require(len(markers) <= 1, 'multiple_primary_positions')
    stress = None
    if markers:
        prefix = text[:markers[0].start()]
        require(prefix and prefix[-1] in VOWELS, 'marker_not_after_vowel')
        stress = sum(c in VOWELS for c in prefix) - 1
    return plain, stress, len(markers[0].group()) if markers else 0


def rendered_words(log):
    stress = {}; words = {}
    for line in log.splitlines():
        if line.startswith('stress_word='):
            index, ordinal, _ = line.split('=', 1)[1].split(',', 2)
            require(int(index) not in stress, 'duplicate_stress_index')
            stress[int(index)] = int(ordinal)
        elif line.startswith('frontend_word_utf8_hex='):
            index, value = line.split('=', 1)[1].split(',', 1)
            require(int(index) not in words, 'duplicate_word_index')
            words[int(index)] = bytes.fromhex(value).decode('utf-8').lower()
    require(words and set(words) == set(stress) == set(range(len(words))), 'frontend_index_alignment')
    return [(words[i], stress[i]) for i in range(len(words))]


def compare(original, port):
    require([word[0] for word in original] == [word[0] for word in port], 'expanded_word_alignment')
    marked = matches = unmarked = resolved_unmarked = terminal = 0
    for (_, primary, group), (_, candidate) in zip(original, port):
        if primary is None:
            unmarked += 1; resolved_unmarked += candidate >= 0
        else:
            marked += 1; matches += primary == candidate
            terminal += group >= 2
    return dict(aligned_words=len(original), original_marked_words=marked,
                marked_stress_matches=matches, marked_stress_mismatches=marked-matches,
                original_unmarked_words=unmarked, port_resolved_unmarked_words=resolved_unmarked,
                original_multi_angle_groups=terminal)


def self_test():
    # Constructed annotations, not a dump of an original dictionary.
    encode = lambda text: text.encode('cp866').hex()
    original = [annotation(encode('ма<<ма')), annotation(encode('и')), annotation(encode('ра<му'))]
    result = compare(original, [('мама', 0), ('и', 0), ('раму', 1)])
    require(result['marked_stress_matches'] == result['marked_stress_mismatches'] == 1 and
            result['original_unmarked_words'] == result['port_resolved_unmarked_words'] == 1 and
            result['original_multi_angle_groups'] == 1, 'scalar_accounting')
    rejected = 0
    for bad in ('ма<м<а', '<мама', 'мам<а', '[t0]мама'):
        try: annotation(encode(bad))
        except ValueError: rejected += 1
    require(rejected == 4, 'annotation_guards')
    try: compare(original, [('мама', 0)])
    except ValueError: rejected += 1
    require(rejected == 5, 'alignment_guard')
    print(json.dumps({'synthetic_comparisons': 1, 'negative_guards': rejected, 'actual_captures': 0}))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--exe', type=Path)
    parser.add_argument('--voice', type=Path)
    parser.add_argument('--capture-root', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    if args.self_test: self_test()
    if not args.exe:
        require(args.self_test, 'arguments_required'); return
    require(all((args.voice, args.capture_root, args.output, args.report)), 'arguments_required')
    require(not args.output.exists() and not args.report.exists(), 'fresh_outputs_required')
    args.output.mkdir(parents=True)
    rows = []
    for capture in sorted(args.capture_root.glob('*-trace')):
        identity = capture.name[:-6]
        with (capture / 'linguistics-m46.jsonl').open(encoding='utf-8') as stream:
            snapshots = [json.loads(line) for line in stream if line.strip()]
        audit(snapshots)
        original = []
        internal = 0
        unsupported = False
        for row in snapshots:
            if row['event'] != 'after_markers': continue
            for word in row['state']['words']:
                internal += bytes.fromhex(word['code_hex']) in (b'/', b'(/)')
                try: original.append(annotation(word['annotated_cp866_hex']))
                except ValueError: unsupported = True
        directory = args.output / identity; directory.mkdir()
        result = subprocess.run([str(args.exe.resolve()), '--render', str(args.voice.resolve()),
            str((capture / 'input.txt').resolve()), 'm44-lexicon', str((directory / 'port.wav').resolve())],
            capture_output=True, timeout=65)
        require(result.returncode == 0, f'port_render_failed:{identity}')
        log = result.stdout.decode('utf-8')
        (directory / 'render.log').write_text(log, encoding='utf-8')
        port = rendered_words(log)
        sources = {int(line.split('=', 1)[1].split(',', 1)[0]): line.rsplit(',', 1)[1]
                   for line in log.splitlines() if line.startswith('stress_word=')}
        row = dict(case_id=identity, original_internal_splits=internal, aligned=False)
        if unsupported:
            row['reason'] = 'non_orthographic_annotation'
        else:
            try:
                row.update(compare(original, port), aligned=True)
                row['heuristic_stress_mismatches'] = sum(primary is not None and primary != target and
                    sources[i] == 'heuristic' for i, ((_, primary, _), (_, target)) in enumerate(zip(original, port)))
            except ValueError: row['reason'] = 'expanded_word_alignment'
        rows.append(row)
    fields = ('aligned_words', 'original_marked_words', 'marked_stress_matches',
        'marked_stress_mismatches', 'original_unmarked_words', 'port_resolved_unmarked_words',
        'original_multi_angle_groups', 'original_internal_splits', 'heuristic_stress_mismatches')
    report = dict(schema='nicolai-m46b-frontend-comparison-v1', cases=len(rows),
                  aligned_cases=sum(row['aligned'] for row in rows), rows=rows,
                  scope='Lexical stress positions on exactly aligned words; markers/splits are not perceived-prominence or acoustic-parity measurements.')
    report.update({field: sum(row.get(field, 0) for row in rows) for field in fields})
    with args.report.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, indent=2); stream.write('\n')
    print(json.dumps({key: value for key, value in report.items() if key != 'rows'}))


if __name__ == '__main__':
    main()
