"""Validate PRIVATE seven-stage captures; emit scalar evidence, not voice data.

M51 does not implement context scoring. The probe export is PRIVATE original
candidate data and must never be committed, packaged or uploaded.
"""
import argparse
import copy
import json
from pathlib import Path
import struct

import audit_m46_linguistics as m46
from audit_m46_linguistics import require, integer, unhex
from compare_original_frontend import annotation

SCHEMA = 'nicolai-m51-analysis-v1'
STAGES = ('before_normalization', 'before_context', 'after_context') + m46.STAGES
LAYOUT = 'payload20-at-stride-plus4-v1'


def payloads(word):
    count = integer(word['candidate_count'], 0, 70)
    require(word['candidate_layout'] == LAYOUT, 'candidate_layout')
    for field in ('candidates_hex', 'candidate_payloads_hex'):
        require(isinstance(word[field], list) and len(word[field]) == count, 'candidate_count')
    old = [unhex(value, 20, 20) for value in word['candidates_hex']]
    rows = [unhex(value, 20, 20) for value in word['candidate_payloads_hex']]
    for i, row in enumerate(rows):
        require(old[i][4:] == row[:16], 'shifted_window_overlap')
        if i + 1 < count:
            require(old[i + 1][:4] == row[16:], 'record_identity_overlap')
    return rows


def conflict(rows):
    return any(row[:3] != rows[0][:3] for row in rows[1:])


def audit(rows, probe_output=None):
    calls, old_rows, probe_rows, normalization_pairs = {}, [], [], []
    report = dict(candidate_payloads=0, analyzed_words=0, pronunciation_conflicts=0,
                  normalization_changed_words=0, context_changed_words=0,
                  scored_candidates=0, unique_highest_words=0, tied_highest_words=0,
                  context_non_score_changed_words=0, common_triple_fast_path_words=0,
                  common_triple_score_mismatches=0, selected_annotation_checked_words=0,
                  selected_annotation_matches=0, selected_annotation_mismatches=0,
                  selected_annotation_alignment_exclusions=0)
    for row in rows:
        require(row['schema'] == SCHEMA, 'unsupported_schema')
        identity = (integer(row['call'], 1, 2**63 - 1), integer(row['thread_id'], 1, 2**32 - 1))
        stages = calls.setdefault(identity, [])
        require(len(stages) < len(STAGES) and row['event'] == STAGES[len(stages)], 'stage_order')
        state = row['state']
        count = integer(state['word_count'], 0, 256)
        require(isinstance(state['words'], list) and len(state['words']) == count, 'word_count')
        if stages:
            require(count == stages[0]['word_count'], 'changing_word_count')
        for i, word in enumerate(state['words'], 1):
            require(word['index'] == i, 'word_index')
            unhex(word['annotated_cp866_hex'], 4095)
            values = payloads(word)
            report['candidate_payloads'] += len(values)
            probe_rows.append(values)
        stages.append(state)
        if len(stages) > 3:
            compatible = dict(row, schema=m46.SCHEMA)
            old_rows.append(compatible)
    require(calls and all(len(stages) == len(STAGES) for stages in calls.values()), 'incomplete_call')
    for stages in calls.values():
        generated, normalized, selected = stages[:3]
        for a, b, c in zip(generated['words'], normalized['words'], selected['words']):
            aa, bb, cc = payloads(a), payloads(b), payloads(c)
            normalization_pairs.append((aa, bb))
            report['analyzed_words'] += 1
            report['pronunciation_conflicts'] += conflict(bb)
            report['normalization_changed_words'] += aa != bb
            report['context_changed_words'] += bb != cc or b['annotated_cp866_hex'] != c['annotated_cp866_hex']
            report['context_non_score_changed_words'] += (
                [v[:8] + v[12:] for v in bb] != [v[:8] + v[12:] for v in cc])
            scores = [struct.unpack_from('<i', value, 8)[0] for value in cc]
            report['scored_candidates'] += sum(score != 0 for score in scores)
            if scores:
                tied = scores.count(max(scores)) > 1
                report['unique_highest_words'] += not tied
                report['tied_highest_words'] += tied
                if not conflict(bb):
                    report['common_triple_fast_path_words'] += 1
                    report['common_triple_score_mismatches'] += scores != [1] * len(scores)
                # Pinned 10228270 selects the first greatest signed score.
                # Check only exactly folded-aligned surface words; do not
                # manufacture successes for context-driven word replacement.
                source, _, _ = annotation(b['annotated_cp866_hex'])
                target, stress, _ = annotation(c['annotated_cp866_hex'])
                if source.replace('ё', 'е') != target.replace('ё', 'е'):
                    report['selected_annotation_alignment_exclusions'] += 1
                else:
                    chosen = cc[scores.index(max(scores))]
                    yo = [i + 1 for i, letter in enumerate(target) if letter == 'ё']
                    expected_yo = [chosen[0]] if chosen[0] else []
                    expected_stress = chosen[1] - 1 if chosen[1] else None
                    matched = yo == expected_yo and stress == expected_stress
                    report['selected_annotation_checked_words'] += 1
                    report['selected_annotation_matches'] += matched
                    report['selected_annotation_mismatches'] += not matched
            # 101a1830 clears every score before the context pass.
            require(all(value[8:12] == bytes(4) for value in bb), 'normalizer_score_not_zero')
    report.update(m46.audit(old_rows))
    report['schema'] = 'nicolai-m51-analysis-audit-v1'
    report['limits'] = 'Candidate layout, conflict gate and observed stages only; no portable context-scoring or audible parity claim.'
    if probe_output:
        require(len(probe_rows) <= 100000, 'probe_row_limit')
        with Path(probe_output).open('xb') as stream:
            stream.write(b'N51CAND2' + struct.pack('<I', len(probe_rows)))
            for values in probe_rows:
                stream.write(bytes([len(values)]) + b''.join(values))
            stream.write(struct.pack('<I', len(normalization_pairs)))
            for before, after in normalization_pairs:
                for values in (before, after):
                    stream.write(bytes([len(values)]) + b''.join(values))
        report['private_probe_rows'] = len(probe_rows)
    return report


def synthetic():
    legacy = m46.synthetic(1)
    values = [bytes([4, 2, 0, 1, 7, 3, 0, 0]) + bytes(4) +
              bytes([9, 0, 0, 0]) + struct.pack('<I', 123),
              bytes([0, 1, 0, 1, 7, 5, 0, 0]) + bytes(4) +
              bytes([9, 0, 0, 0]) + struct.pack('<I', 456)]
    rows = []
    for i, stage in enumerate(STAGES):
        row = copy.deepcopy(legacy[max(0, i - 3)])
        row.update(schema=SCHEMA, event=stage)
        word = row['state']['words'][0]
        data = list(values)
        if i >= 2:
            data[0] = data[0][:8] + struct.pack('<i', 17) + data[0][12:]
            data[1] = data[1][:8] + struct.pack('<i', -4) + data[1][12:]
        # Authored Cyrillic fixture with yo at character four and stress two.
        word['annotated_cp866_hex'] = ('бабе' if i < 2 else 'бабё<').encode('cp866').hex()
        if i < 3:
            for field in ('punctuation_cp866_hex', 'raw_separator', 'code_hex'):
                word.pop(field)
        word.update(candidate_count=2, candidate_layout=LAYOUT,
                    candidate_payloads_hex=[v.hex() for v in data],
                    candidates_hex=[(bytes(4) + data[0][:16]).hex(),
                                    (data[0][16:] + data[1][:16]).hex()])
        rows.append(row)
    return rows


def self_test():
    report = audit(synthetic())
    require(report['complete_calls'] == report['pronunciation_conflicts'] ==
            report['unique_highest_words'] == 1 and report['candidate_payloads'] == 14 and
            report['scan_model_mismatches'] == 0, 'synthetic_audit')
    empty = synthetic()
    for row in empty:
        row['state'] = dict(word_count=0, words=[])
    require(audit(empty)['zero_word_calls'] == 1, 'zero_word_call')
    bad = []
    rows = synthetic(); bad.append(rows[:-1])
    rows = synthetic(); rows[0]['event'] = 'before_context'; bad.append(rows)
    rows = synthetic(); rows[0]['state']['words'][0]['candidate_layout'] = 'old'; bad.append(rows)
    rows = synthetic(); rows[0]['state']['words'][0]['candidate_payloads_hex'][0] = '00'; bad.append(rows)
    rows = synthetic(); rows[0]['state']['words'][0]['candidates_hex'][1] = '00' * 20; bad.append(rows)
    rows = synthetic(); rows[0]['state']['words'][0]['candidate_count'] = 71; bad.append(rows)
    rows = synthetic(); rows[1]['state'] = copy.deepcopy(rows[2]['state']); bad.append(rows)
    rejected = 0
    for rows in bad:
        try:
            audit(rows)
        except ValueError:
            rejected += 1
    require(rejected == len(bad), 'negative_contracts')
    print(json.dumps(dict(synthetic_positive_cases=2, synthetic_negative_cases=rejected,
                          actual_original_captures=0)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--input', type=Path)
    parser.add_argument('--report', type=Path, help='Fresh scalar-only output')
    parser.add_argument('--private-probe-output', type=Path, help='PRIVATE raw payloads, never upload')
    args = parser.parse_args()
    if args.self_test:
        self_test()
    if args.input:
        require(args.input.stat().st_size <= 64 * 1024 * 1024, 'capture_too_large')
        with args.input.open(encoding='utf-8') as stream:
            report = audit((json.loads(line) for line in stream if line.strip()), args.private_probe_output)
        if args.report:
            with args.report.open('x', encoding='utf-8') as stream:
                json.dump(report, stream, indent=2); stream.write('\n')
        print(json.dumps(report))
    elif args.report or args.private_probe_output or not args.self_test:
        parser.error('--input is required except for --self-test')


if __name__ == '__main__':
    main()
