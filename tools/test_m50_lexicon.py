"""Private initialized noun-filter oracle and ordinary controls; scalar report."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('probe','voice','policy','noun-policy','report'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--original',type=Path)
    args=p.parse_args()
    controls={
        'бельем':(1,4),'копьем':(1,4),'ружьем':(1,4),'мытьем':(1,4),
        'землей':(1,4),'семьей':(1,4),'земле':(1,None),'семье':(1,None),
        'живете':(1,3),'найдете':(1,4),'пьете':(0,2),'столе':(1,None),
        'окне':(1,None),'руке':(1,None),'будет':(0,None),'хорошая':(1,None),
        'белье':(None,None),'копье':(None,None),'ружье':(None,None),'мытье':(None,None),
        'голоса':(None,None),'воды':(None,None),'поешь':(None,None),'поем':(None,None),
        'несет':(None,None),'ведет':(None,None),'':(None,None),
        'Бельем':(None,None),'бельем!':(None,None),'123':(None,None),
    }
    with tempfile.TemporaryDirectory(prefix='nicolai-m50-controls-') as root:
        corpus=Path(root)/'words.tsv'
        corpus.write_text(''.join(f'{i}\t{word}\n' for i,word in enumerate(controls)),encoding='utf-8')
        command=[str(args.probe.resolve()),str(args.voice.resolve()),str(corpus),
            str(args.policy.resolve()),str(args.noun_policy.resolve())]
        if args.original:
            command+=['--original',str(args.original.resolve())]
        result=subprocess.run(command,capture_output=True,timeout=30,check=True)
    seen=set();values=list(controls.values());added=0
    for line in result.stdout.decode().splitlines():
        index,old,_,_,_,status,stress,_,yo=line.split('\t');index=int(index)
        assert index not in seen and 0<=index<len(values);seen.add(index)
        actual=(None if stress=='-' else int(stress),None if yo=='-' else int(yo))
        assert actual==values[index],(list(controls)[index],actual,values[index])
        assert (status=='accepted')==(actual[0] is not None)
        added += old!='accepted' and status=='accepted'
    assert len(seen)==len(values)
    rows=full=positive=0
    if args.original:
        oracle=next(s for s in result.stderr.decode().splitlines() if s.startswith('ORIGINAL\t'))
        rows,full,positive=map(int,oracle.split('\t')[1:])
        assert (rows,full,positive)==(200,111520,1070)
    report=dict(schema='nicolai-m50-local-contracts-v1',controls=len(values),
        accepted_controls=sum(v[0] is not None for v in values),additional_accepted_controls=added,
        accepted_yo_controls=sum(v[1] is not None for v in values),
        original_initialized_list_matches=rows,original_complete_candidate_matches=full,
        original_positive_yo_cases=positive,
        scope='Actual import-free initializer lane on owned buffers plus synthetic noun authoring and ordinary controls; not full contextual morphology or audio parity.')
    for key,path in [('voice',args.voice),('policy',args.policy),('noun_policy',args.noun_policy),('original',args.original)]:
        if path:
            report[key+'_sha256']=hashlib.sha256(path.read_bytes()).hexdigest()
    with args.report.open('x',encoding='utf-8') as output:
        json.dump(report,output,indent=2);output.write('\n')
    print(json.dumps(report))


if __name__=='__main__':
    main()
