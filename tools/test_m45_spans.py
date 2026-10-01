"""Bounded synthetic controls for the optional local original-span oracle.

No DLL, dictionary strings, disassembly or real linguistic records are saved.
The model covers the inspected count/search passes and two table-free grammar
predicates; it is not a substitute for complete original text analysis.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


COUNTS = (1, 2, 3, 4, 5, 6, 7, 8, 10, 14, 22, 32)
SEEDS = ' _=+'
SHA = 'f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7'


def split_codes(raw, counted):
    """One-based index translation of the two inspected scan/search loops."""
    count = len(raw) - 1
    codes = [''] * (count + 1)
    codes[count] = '///'
    for index in range(1, count):
        if raw[index] == ',':
            codes[index] = '//'

    def choose(first, last, separator):
        middle = (first + last) // 2
        eligible = [i for i in range(first, last + 1)
                    if not codes[i] and raw[i] == separator]
        return min(eligible, key=lambda i: (abs(i - middle), i)) if eligible else 0

    first, scan = 1, 2
    budget = 4096
    while first <= count - 1:
        budget -= 1
        assert budget > 0, 'minor_model_did_not_terminate'
        last, effective = scan, scan
        while last < count and not codes[last]:
            last += 1
            effective += bool(counted[last])
        selected = choose(first, last, ' ') if effective - first >= 5 else 0
        if selected:
            codes[selected] = '/'
            first -= 1
            scan -= 1
        first += 1
        scan += 1

    first = 1
    while first <= count - 1:
        budget -= 1
        assert budget > 0, 'hard_model_did_not_terminate'
        last, effective = first, first
        while last < count and not codes[last]:
            last += 1
            effective += bool(counted[last])
        selected = choose(first, last, '_') if effective - first > 6 else 0
        if selected:
            codes[selected] = '(/)'
            first -= 1
        first += 1
    return codes[1:]


def controls():
    expected = {}
    for count in COUNTS:
        for seed in SEEDS:
            for lane in ('empty', 'candidate', 'marked'):
                raw = [seed] * (count + 1)
                raw[count] = ','
                counted = [lane != 'candidate'] * (count + 1)
                expected[f'{lane}_{ord(seed)}_{count}'] = (
                    count, ''.join(raw[1:]), split_codes(raw, counted))
    for punctuation in ',:()_.!?':
        raw = [' '] * 5
        raw[4] = ','
        if punctuation in ',:()_':
            raw[2] = ','
        expected[f'punct_{ord(punctuation)}'] = (
            4, ''.join(raw[1:]), split_codes(raw, [False] * 5))
    for seed in ' =+':
        for kind in (4, 10):
            for answer in ('yes', 'no'):
                raw = [seed] * 4
                raw[3] = ','
                if seed == ' ':
                    if answer == 'yes':
                        raw[1] = '_'
                    if kind == 4:  # following kind-6 item joins its own successor
                        raw[2] = '='
                expected[f'kind{kind}_{answer}_{ord(seed)}'] = (
                    3, ''.join(raw[1:]), split_codes(raw, [False] * 4))
    return expected


def self_test():
    cases = controls()
    assert len(cases) == 164
    assert cases['empty_32_5'][2] == ['', '', '', '', '///']
    assert cases['empty_32_6'][2] == ['', '', '/', '', '', '///']
    assert cases['candidate_32_6'][2] == ['', '', '', '', '', '///']
    assert cases['marked_32_6'][2] == cases['empty_32_6'][2]
    assert cases['empty_95_7'][2] == [''] * 6 + ['///']
    assert cases['empty_95_8'][2] == ['', '', '', '(/)', '', '', '', '///']
    assert cases['punct_44'][2] == ['', '//', '', '///']
    assert cases['kind4_yes_32'][1] == '_=,'
    assert cases['kind4_no_32'][1] == ' =,'
    assert cases['kind10_yes_32'][1] == '_ ,'
    assert cases['kind10_no_32'][1] == '  ,'
    return cases


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path)
    parser.add_argument('--original', type=Path)
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()
    expected = self_test()
    if bool(args.probe) != bool(args.original):
        parser.error('--probe and --original must be supplied together')
    report = {'schema': 'nicolai-m45-synthetic-spans-v1', 'model_cases': len(expected),
              'original_matches': 0}
    if args.probe:
        sha = hashlib.sha256(args.original.read_bytes()).hexdigest()
        if sha != SHA:
            raise ValueError('unsupported_original_sha256')
        result = subprocess.run([str(args.probe.resolve()), str(args.original.resolve())],
                                capture_output=True, timeout=30, check=True, text=True)
        usage = subprocess.run([str(args.probe.resolve())], capture_output=True,
                               timeout=10, text=True)
        rejected = subprocess.run([str(args.probe.resolve()), str(args.probe.resolve())],
                                  capture_output=True, timeout=10, text=True)
        assert usage.returncode == 2 and 'usage:' in usage.stderr
        assert rejected.returncode == 1 and 'unsupported_original_sha256' in rejected.stderr
        seen = set()
        for line in result.stdout.splitlines():
            name, count, raw, code = line.split('\t')
            assert name in expected and name not in seen, name
            actual = (int(count), raw, code.split('|'))
            assert actual == expected[name], (name, actual, expected[name])
            seen.add(name)
        assert seen == set(expected)
        report.update(original_sha256=sha, original_matches=len(seen),
                      uniform_controls=144, punctuation_controls=8, grammar_controls=12,
                      buffer_guards_passed=True, negative_invocations_passed=2)
    report['limits'] = ('Synthetic intermediate inputs only: no complete morphology, '
                        'physical duration/pitch authoring, pause milliseconds or PCM parity.')
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        with args.out.open('x', encoding='utf-8') as output:
            json.dump(report, output, indent=2)
            output.write('\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
