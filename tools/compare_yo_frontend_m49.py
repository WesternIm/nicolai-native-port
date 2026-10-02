"""PRIVATE exactly folded-aligned original/port е/ё and lexical-stress audit.

Folding е/ё is used ONLY to align input spellings. Correctness compares the
original annotated letters against the port's actual pronunciation spelling.
Reports contain counts/hashes/case IDs, not raw words or candidate bytes.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from audit_m46_linguistics import audit, require
from compare_original_frontend import annotation, rendered_words


def pronunciations(log):
    words=rendered_words(log)
    values={}
    for line in log.splitlines():
        if line.startswith('frontend_pronunciation_utf8_hex='):
            index,hex_value=line.split('=',1)[1].split(',',1)
            index=int(index)
            require(index not in values,'duplicate_pronunciation_index')
            values[index]=bytes.fromhex(hex_value).decode('utf-8')
    if not values:
        return words
    require(set(values)==set(range(len(words))),'pronunciation_index_alignment')
    return [(values[i],stress) for i,(_,stress) in enumerate(words)]


def compare(original,baseline,candidate):
    folded=lambda word:word.replace('ё','е')
    require([folded(w[0]) for w in original]==[folded(w[0]) for w in baseline]==
        [folded(w[0]) for w in candidate],'only_e_yo_spelling_alignment')
    result=dict(aligned_words=len(original),marked_positions=0,stress_fixed=0,
        stress_regressed=0,baseline_stress_mismatches=0,candidate_stress_mismatches=0,
        e_yo_positions=0,original_yo_positions=0,baseline_e_yo_mismatches=0,
        candidate_e_yo_mismatches=0,e_yo_fixed=0,e_yo_regressed=0)
    for (word,stress,_),(old,old_stress),(new,new_stress) in zip(original,baseline,candidate):
        if stress is not None:
            result['marked_positions']+=1
            result['stress_fixed']+=old_stress!=stress and new_stress==stress
            result['stress_regressed']+=old_stress==stress and new_stress!=stress
            result['baseline_stress_mismatches']+=old_stress!=stress
            result['candidate_stress_mismatches']+=new_stress!=stress
        for source,before,after in zip(word,old,new):
            if source not in 'её':
                continue
            result['e_yo_positions']+=1
            result['original_yo_positions']+=source=='ё'
            result['baseline_e_yo_mismatches']+=source!=before
            result['candidate_e_yo_mismatches']+=source!=after
            result['e_yo_fixed']+=source!=before and source==after
            result['e_yo_regressed']+=source==before and source!=after
    return result


def self_test():
    result=compare([('бабё',1,1),('бабе',1,1)],[('бабе',0),('бабе',1)],[('бабё',1),('бабё',1)])
    require(result['e_yo_fixed']==result['e_yo_regressed']==result['stress_fixed']==1,'paired_counts')
    require(result['stress_regressed']==0,'stress_independent_of_spelling')
    log=f"stress_word=0,1,fixture\nfrontend_word_utf8_hex=0,{'бабе'.encode().hex()}\n"
    require(pronunciations(log)==[('бабе',1)],'old_log_uses_actual_source')
    diagnostic=f"frontend_pronunciation_utf8_hex=0,{'бабё'.encode().hex()}\n"
    require(pronunciations(log+diagnostic)==[('бабё',1)],'actual_pronunciation_used')
    failures=(lambda:compare([('бабё',1,1)],[('бобе',0)],[('бабё',1)]),
              lambda:pronunciations(log+diagnostic+diagnostic),
              lambda:pronunciations(log+diagnostic.replace('=0,','=1,')))
    for operation in failures:
        try:
            operation()
        except ValueError:
            continue
        raise ValueError('invalid_alignment_or_diagnostic_accepted')
    print(json.dumps(dict(synthetic_comparisons=1,diagnostic_contracts=4,alignment_refusals=1)))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--self-test',action='store_true')
    p.add_argument('--exe',type=Path)
    p.add_argument('--baseline-exe',type=Path)
    p.add_argument('--voice',type=Path)
    p.add_argument('--capture-root',type=Path)
    p.add_argument('--output',type=Path)
    p.add_argument('--report',type=Path)
    p.add_argument('--comparison',nargs=3,action='append',
        metavar=('CAPTURES','BASELINE_LOGS','CANDIDATE_LOGS'),
        help='Reuse already verified private logs instead of rendering them again')
    args=p.parse_args()
    if args.self_test:
        self_test()
    if not args.exe and not args.comparison:
        require(args.self_test,'arguments_required')
        return
    require(args.exe and args.baseline_exe and args.report,'arguments_required')
    require(not args.report.exists(),'fresh_report_required')
    if args.comparison:
        require(not args.output and not args.capture_root,'render_and_reuse_are_exclusive')
        groups=[tuple(map(Path,paths)) for paths in args.comparison]
    else:
        require(args.voice and args.capture_root and args.output,'render_arguments_required')
        require(not args.output.exists(),'fresh_output_required')
        args.output.mkdir(parents=True)
        groups=[(args.capture_root,None,None)]
    rows=[]
    captures=((group,capture,baseline_logs,candidate_logs)
        for group,(capture_root,baseline_logs,candidate_logs) in enumerate(groups)
        for capture in sorted(capture_root.glob('*-trace')))
    for group,capture,baseline_logs,candidate_logs in captures:
        identity=capture.name[:-6]
        snapshots=[json.loads(s) for s in (capture/'linguistics-m46.jsonl').read_text(encoding='utf-8').splitlines()]
        audit(snapshots)
        logs=[]
        for label,exe,profile in (('baseline',args.baseline_exe,'m48-lexicon'),('candidate',args.exe,'m49-lexicon')):
            if baseline_logs is not None:
                root=baseline_logs if label=='baseline' else candidate_logs
                logs.append((root/identity/'render.log').read_text(encoding='utf-8'))
                continue
            directory=args.output/identity/label
            directory.mkdir(parents=True)
            result=subprocess.run([str(exe.resolve()),'--render',str(args.voice.resolve()),
                str((capture/'input.txt').resolve()),profile,str((directory/'port.wav').resolve())],
                capture_output=True,timeout=65)
            require(result.returncode==0,f'render_failed:{identity}:{label}')
            log=result.stdout.decode('utf-8')
            (directory/'render.log').write_text(log,encoding='utf-8')
            logs.append(log)
        # Malformed diagnostic records are errors, not unaligned exclusions.
        pronounced=[pronunciations(log) for log in logs]
        row=dict(group=group,case_id=identity,aligned=False)
        try:
            original=[annotation(w['annotated_cp866_hex']) for r in snapshots if r['event']=='after_markers'
                for w in r['state']['words']]
            row.update(compare(original,*pronounced),aligned=True)
        except ValueError:
            row['reason']='non_orthographic_or_non_e_yo_expansion'
        rows.append(row)
    fields=list(compare([],[],[]))
    report=dict(schema='nicolai-m49-e-yo-original-comparison-v1',cases=len(rows),
        aligned_cases=sum(r['aligned'] for r in rows),rows=rows,
        exe_sha256=hashlib.sha256(args.exe.read_bytes()).hexdigest(),
        baseline_exe_sha256=hashlib.sha256(args.baseline_exe.read_bytes()).hexdigest(),
        scope='Exact source-word alignment allowing only е/ё spelling variants; actual pronunciation letters and marked lexical positions. Not full phoneme/acoustic/prominence parity.')
    report.update({field:sum(row.get(field,0) for row in rows) for field in fields})
    with args.report.open('x',encoding='utf-8') as out:
        json.dump(report,out,indent=2)
        out.write('\n')
    require(report['aligned_cases'],'no_aligned_cases')
    print(json.dumps({k:v for k,v in report.items() if k!='rows'}))


if __name__=='__main__':
    main()
