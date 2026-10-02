"""Compare already rendered PRIVATE frontend logs on identical original words.

No source text, original bytes or individual dictionary entries enter reports.
"""
import argparse
import json
from pathlib import Path
from compare_original_frontend import annotation, rendered_words, compare
from audit_m46_linguistics import audit, require

def compare_delta(original, baseline, candidate):
    compare(original, baseline); compare(original, candidate)
    fixed = regressed = unchanged_wrong = unchanged_right = 0
    for (_, stress, _), (_, old), (_, new) in zip(original, baseline, candidate):
        if stress is None: continue
        fixed += old != stress and new == stress
        regressed += old == stress and new != stress
        unchanged_wrong += old != stress and new != stress
        unchanged_right += old == stress and new == stress
    return dict(fixed=fixed, regressed=regressed, unchanged_wrong=unchanged_wrong,
                unchanged_right=unchanged_right)

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--comparison', nargs=3, action='append', metavar=('CAPTURES', 'BASELINE', 'CANDIDATE'))
    p.add_argument('--report', type=Path)
    p.add_argument('--self-test', action='store_true')
    args = p.parse_args()
    if args.self_test:
        result = compare_delta([('мама', 0, 1), ('рама', 0, 1), ('и', None, 0)],
                               [('мама', 1), ('рама', 0), ('и', 0)],
                               [('мама', 0), ('рама', 1), ('и', 0)])
        require(result == dict(fixed=1, regressed=1, unchanged_wrong=0, unchanged_right=0), 'delta_counts')
        print(json.dumps(result))
    if not args.comparison:
        require(args.self_test, 'arguments_required'); return
    require(args.report and not args.report.exists(), 'fresh_report_required')
    rows = []
    for group, paths in enumerate(args.comparison):
        captures, baseline, candidate = map(Path, paths)
        for capture in sorted(captures.glob('*-trace')):
            rowsrc = [json.loads(s) for s in (capture / 'linguistics-m46.jsonl').read_text(encoding='utf-8').splitlines()]
            audit(rowsrc)
            identity = capture.name[:-6]
            row = dict(group=group, case_id=identity, aligned=False)
            try:
                original = [annotation(w['annotated_cp866_hex']) for r in rowsrc if r['event']=='after_markers'
                            for w in r['state']['words']]
                old = rendered_words((baseline / identity / 'render.log').read_text(encoding='utf-8'))
                new = rendered_words((candidate / identity / 'render.log').read_text(encoding='utf-8'))
                row.update(compare_delta(original, old, new), aligned=True)
            except ValueError: row['reason'] = 'non_orthographic_or_expanded_word_alignment'
            rows.append(row)
    report = dict(schema='nicolai-m47-frontend-delta-v1', cases=len(rows),
                  aligned_cases=sum(r['aligned'] for r in rows), rows=rows,
                  scope='Same captured original words and private render logs; lexical positions only, not listening/PCM superiority.')
    require(rows and report['aligned_cases'], 'no_aligned_captures')
    report.update({key: sum(r.get(key,0) for r in rows) for key in ('fixed','regressed','unchanged_wrong','unchanged_right')})
    with args.report.open('x',encoding='utf-8') as out:
        json.dump(report,out,indent=2); out.write('\n')
    print(json.dumps({k:v for k,v in report.items() if k!='rows'}))

if __name__ == '__main__': main()
