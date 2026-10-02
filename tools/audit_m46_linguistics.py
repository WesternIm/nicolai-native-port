"""Validate PRIVATE M46 JSONL and compare the M45 scan model with captured codes.

Outputs scalar counts only. The raw producer, complete morphology, acoustic
units and perceptual parity are not validated by this audit. No original data
is required for --self-test; its fixtures are explicitly synthetic.
"""
import argparse
import copy
import json
from pathlib import Path
from test_m45_spans import split_codes

STAGES = ('before_split', 'after_split', 'after_markers', 'after_authoring')
SCHEMA = 'nicolai-m46-linguistic-v1'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def unhex(value, maximum, exact=None):
    require(isinstance(value, str) and len(value) % 2 == 0 and
            len(value) <= maximum * 2, 'invalid_hex_length')
    require(all(c in '0123456789abcdef' for c in value), 'invalid_hex_alphabet')
    result = bytes.fromhex(value)
    require(exact is None or len(result) == exact, 'invalid_record_size')
    return result


def integer(value, low, high):
    require(type(value) is int and low <= value <= high, 'integer_out_of_bounds')
    return value


def audit(rows):
    calls = {}
    phone_records = 0
    for row in rows:
        require(row['schema'] == SCHEMA, 'unsupported_schema')
        identity = (integer(row['call'], 1, 2**63 - 1),
                    integer(row['thread_id'], 1, 2**32 - 1))
        stages = calls.setdefault(identity, [])
        require(len(stages) < 4 and row['event'] == STAGES[len(stages)], 'stage_order')
        state = row['state']
        count = integer(state['word_count'], 0, 256)
        require(isinstance(state['words'], list) and len(state['words']) == count, 'word_count')
        if stages:
            require(count == stages[0]['word_count'], 'changing_word_count')
        total = 0
        for index, word in enumerate(state['words'], 1):
            require(word['index'] == index, 'word_index')
            unhex(word['annotated_cp866_hex'], 4095)
            unhex(word['punctuation_cp866_hex'], 11)
            integer(word['raw_separator'], 0, 255)
            code = unhex(word['code_hex'], 3)
            require(code in (b'', b'/', b'//', b'///', b'(/)'), 'separator_code')
            candidates = integer(word['candidate_count'], 0, 70)
            require(isinstance(word['candidates_hex'], list) and
                    len(word['candidates_hex']) == candidates, 'candidate_count')
            for candidate in word['candidates_hex']:
                unhex(candidate, 20, 20)
            if row['event'] == 'after_authoring':
                phones = integer(word['source_phone_count'], 0, 128)
                require(isinstance(word['source_phone_records_hex'], list) and
                        len(word['source_phone_records_hex']) == phones, 'phone_count')
                for record in word['source_phone_records_hex']:
                    unhex(record, 32, 32)
                total += phones
        require(total <= 4096, 'total_phone_count')
        phone_records += total
        stages.append(state)
    require(bool(calls), 'empty_capture')
    matched = empty = 0
    for stages in calls.values():
        require(len(stages) == 4, 'incomplete_call')
        before, after = stages[:2]
        if not before['word_count']:
            empty += 1
            continue
        require([w['candidate_count'] for w in before['words']] ==
                [w['candidate_count'] for w in after['words']], 'split_changed_candidate_count')
        raw = [' '] + [chr(w['raw_separator']) for w in after['words']]
        counted = [False] + [w['candidate_count'] == 0 or
                    b'<' in bytes.fromhex(w['annotated_cp866_hex']) for w in before['words']]
        expected = split_codes(raw, counted)
        actual = [bytes.fromhex(w['code_hex']).decode('ascii') for w in after['words']]
        matched += expected == actual
    return {'schema': 'nicolai-m46-linguistic-audit-v1',
            'complete_calls': len(calls), 'scan_model_matches': matched,
            'linguistic_calls': len(calls) - empty, 'zero_word_calls': empty,
            'scan_model_mismatches': len(calls) - empty - matched,
            'source_phone_records': phone_records,
            'limits': 'Scan/split only; not complete morphology, raw-producer parity, acoustic units or audio parity.'}


def synthetic(count=6):
    raw = [' '] * count + [',']
    codes = split_codes(raw, [True] * (count + 1))
    rows = []
    for stage in STAGES:
        words = []
        for i in range(1, count + 1):
            word = {'index': i, 'annotated_cp866_hex': b'qzx<qv'.hex(),
                    'punctuation_cp866_hex': b','.hex() if i == count else '',
                    'raw_separator': ord(raw[i]), 'code_hex': '' if stage == 'before_split' else codes[i-1].encode().hex(),
                    'candidate_count': 0, 'candidates_hex': []}
            if stage == 'after_authoring':
                word.update(source_phone_count=1, source_phone_records_hex=['00' * 32])
            words.append(word)
        rows.append({'schema': SCHEMA, 'call': 1, 'thread_id': 1,
                     'event': stage, 'state': {'word_count': count, 'words': words}})
    return rows


def self_test():
    for count in (1, 6, 14):
        report = audit(synthetic(count))
        require(report['complete_calls'] == report['scan_model_matches'] == 1 and
                report['source_phone_records'] == count, 'synthetic_audit')
    report = audit(synthetic(0))
    require(report['complete_calls'] == report['zero_word_calls'] == 1 and
            report['linguistic_calls'] == report['scan_model_matches'] ==
            report['source_phone_records'] == 0, 'zero_word_accounting')
    bad = []
    rows = synthetic(); bad.append(rows[:-1])
    rows = synthetic(); rows[1]['event'] = 'before_split'; bad.append(rows)
    rows = synthetic(); rows[-1]['state']['words'][0]['source_phone_records_hex'] = ['00']; bad.append(rows)
    rows = synthetic(); rows[0]['state']['word_count'] = 257; bad.append(rows)
    rows = synthetic(); rows[0]['state']['word_count'] = -1; bad.append(rows)
    rows = synthetic(); rows[0]['state']['words'][0]['candidate_count'] = 71; bad.append(rows)
    rows = synthetic(); rows[1]['state']['words'][0]['code_hex'] = 'xy'; bad.append(rows)
    rejected = 0
    for rows in bad:
        try:
            audit(rows)
        except ValueError:
            rejected += 1
    require(rejected == len(bad), 'negative_contracts')
    rows = copy.deepcopy(synthetic()); rows[1]['state']['words'][0]['code_hex'] = '2f'
    require(audit(rows)['scan_model_mismatches'] == 1, 'mismatch_accounting')
    print(json.dumps({'synthetic_positive_cases': 4, 'synthetic_negative_cases': rejected,
                      'mismatch_accounting_cases': 1, 'actual_original_captures': 0}))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--input', type=Path)
    parser.add_argument('--report', type=Path, help='Fresh scalar-only output')
    args = parser.parse_args()
    if args.self_test:
        self_test()
    if args.input:
        # Bounded local artifact, never print decoded words or raw records.
        require(args.input.stat().st_size <= 64 * 1024 * 1024, 'capture_too_large')
        with args.input.open(encoding='utf-8') as stream:
            report = audit(json.loads(line) for line in stream if line.strip())
        print(json.dumps(report))
        if args.report:
            with args.report.open('x', encoding='utf-8') as stream:
                json.dump(report, stream, indent=2); stream.write('\n')
    elif args.report or not args.self_test:
        parser.error('--input is required except for --self-test')


if __name__ == '__main__':
    main()
