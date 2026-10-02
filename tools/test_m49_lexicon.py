"""Private original selector oracle and fixed ordinary controls, scalar report."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--probe',type=Path,required=True)
    p.add_argument('--voice',type=Path,required=True)
    p.add_argument('--policy',type=Path,required=True)
    p.add_argument('--original',type=Path)
    p.add_argument('--report',type=Path,required=True)
    args=p.parse_args()
    # Spelling, stress ordinal, recovered ё character index. These are tests,
    # not production lookup overrides.
    controls={
        'живете':(1,3),'найдете':(1,4),'пьете':(0,2),'льете':(0,2),
        'вернете':(1,4),'вернет':(1,4),'поете':(1,2),'споет':(1,3),
        'споешь':(1,3),'споем':(1,3),'поет':(1,2),
        'столе':(1,None),'окне':(1,None),'письме':(1,None),'руке':(1,None),
        'будет':(0,None),'было':(0,None),'словами':(1,None),'хорошая':(1,None),
        'проверяем':(2,None),'акустика':(1,None),'акустику':(1,None),
        'поешь':(None,None),'поем':(None,None),'земле':(None,None),
        'голоса':(None,None),'воды':(None,None),'123':(None,None),
        'Живете':(None,None),'живете!':(None,None),
    }
    with tempfile.TemporaryDirectory(prefix='nicolai-m49-controls-') as root:
        corpus=Path(root)/'words.tsv'
        corpus.write_text(''.join(f'{i}\t{word}\n' for i,word in enumerate(controls)),encoding='utf-8')
        command=[str(args.probe.resolve()),str(args.voice.resolve()),str(corpus),str(args.policy.resolve())]
        if args.original:
            command+=['--original',str(args.original.resolve())]
        result=subprocess.run(command,capture_output=True,timeout=30,check=True)
    seen=set()
    values=list(controls.values())
    for line in result.stdout.decode().splitlines():
        index,_,_,_,status,stress,_,yo=line.split('\t')
        index=int(index)
        assert index not in seen and 0<=index<len(values)
        seen.add(index)
        actual=(None if stress=='-' else int(stress),None if yo=='-' else int(yo))
        assert actual==values[index],(list(controls)[index],actual,values[index])
        assert (status=='accepted')==(actual[0] is not None)
    assert len(seen)==len(values)
    full=yo=noun=0
    if args.original:
        oracle=next(s for s in result.stderr.decode().splitlines() if s.startswith('ORIGINAL\t'))
        full,yo,noun=map(int,oracle.split('\t')[1:])
        assert (full,yo,noun)==(545480,6458,2630)
    report=dict(schema='nicolai-m49-local-contracts-v1',controls=len(values),
        accepted_controls=sum(v[0] is not None for v in values),
        accepted_yo_controls=sum(v[1] is not None for v in values),
        original_complete_candidate_matches=full,original_positive_yo_cases=yo,
        unresolved_noun_filter_refusals=noun,
        voice_sha256=hashlib.sha256(args.voice.read_bytes()).hexdigest(),
        policy_sha256=hashlib.sha256(args.policy.read_bytes()).hexdigest(),
        scope='Synthetic authoring with private original selector data and ordinary controls; unknown noun-positive filters declined. No full NLP/perceptual claim.')
    if args.original:
        report['original_sha256']=hashlib.sha256(args.original.read_bytes()).hexdigest()
    with args.report.open('x',encoding='utf-8') as output:
        json.dump(report,output,indent=2)
        output.write('\n')
    print(json.dumps(report))


if __name__=='__main__':
    main()
