#!/usr/bin/env python3
from __future__ import annotations
import argparse, csv, json, math, struct, wave
from pathlib import Path


def read_pcm(path: Path):
    with wave.open(str(path), 'rb') as w:
        if w.getnchannels()!=1 or w.getsampwidth()!=2:
            raise ValueError(f'{path}: expected mono PCM16')
        sr=w.getframerate(); raw=w.readframes(w.getnframes())
    return sr,list(struct.unpack('<'+'h'*(len(raw)//2),raw))

def peak(x): return max((abs(v) for v in x), default=0)
def rms(x): return math.sqrt(sum(v*v for v in x)/len(x)) if x else 0.0

def active_bounds(x, sr):
    if not x: return 0,0
    pk=peak(x); th=max(32.0, pk*0.005)
    hits=[i for i,v in enumerate(x) if abs(v)>th]
    if not hits: return 0,0
    pad=int(sr*0.010)
    return max(0,hits[0]-pad), min(len(x),hits[-1]+1+pad)

def corr_offset(a,b,lag,sub=4):
    if lag>=0:
        ai=lag; bi=0; n=min(len(a)-lag,len(b))
    else:
        ai=0; bi=-lag; n=min(len(a),len(b)+lag)
    if n < 128: return -2.0
    # subsample and compute Pearson in one pass
    cnt=0; sa=sb=saa=sbb=sab=0.0
    end=n
    j=0
    while j<end:
        x=float(a[ai+j]); y=float(b[bi+j])
        cnt+=1; sa+=x; sb+=y; saa+=x*x; sbb+=y*y; sab+=x*y
        j+=sub
    if cnt<32: return -2.0
    num=sab - sa*sb/cnt
    da=saa - sa*sa/cnt; db=sbb - sb*sb/cnt
    den=math.sqrt(max(0.0,da)*max(0.0,db))
    return num/den if den else 0.0

def best_lag(a,b,sr,max_ms=250.0):
    ml=int(sr*max_ms/1000.0)
    # coarse grid <=~500 candidates, then fine around winner
    coarse=max(1,(2*ml)//500)
    best=(-2.0,0)
    for lag in range(-ml,ml+1,coarse):
        c=corr_offset(a,b,lag,sub=8)
        if c>best[0]: best=(c,lag)
    lo=max(-ml,best[1]-coarse); hi=min(ml,best[1]+coarse)
    for lag in range(lo,hi+1):
        c=corr_offset(a,b,lag,sub=4)
        if c>best[0]: best=(c,lag)
    return best[1],best[0]

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('reference_pack',type=Path)
    ap.add_argument('portable_dir',type=Path)
    ap.add_argument('--json',dest='json_out',type=Path)
    ap.add_argument('--csv',dest='csv_out',type=Path)
    args=ap.parse_args()
    manifest=json.loads((args.reference_pack/'manifest.json').read_text(encoding='utf-8-sig'))
    rows=[]
    for item in manifest['results']:
        i=item['id']; text=item['text']; rp=args.reference_pack/item['wav']; pp=args.portable_dir/f'{i}.wav'
        row={'id':i,'text':text,'portable_status':'missing'}
        rr,ref=read_pcm(rp); rb,re=active_bounds(ref,rr)
        row.update(reference_total_ms=1000*len(ref)/rr,reference_active_ms=1000*(re-rb)/rr,
                   reference_lead_ms=1000*rb/rr,reference_trail_ms=1000*(len(ref)-re)/rr,
                   reference_peak=peak(ref),reference_rms=rms(ref))
        if pp.exists():
            pr,port=read_pcm(pp)
            if pr!=rr: raise ValueError(f'{i}: sample rate mismatch {rr} vs {pr}')
            pb,pe=active_bounds(port,pr); ar=ref[rb:re]; apm=port[pb:pe]
            lag,c=best_lag(ar,apm,rr)
            ppk=peak(port); rpk=peak(ref); prms=rms(port); rrms=rms(ref)
            row.update(portable_status='ok',portable_total_ms=1000*len(port)/pr,portable_active_ms=1000*(pe-pb)/pr,
                       portable_lead_ms=1000*pb/pr,portable_trail_ms=1000*(len(port)-pe)/pr,
                       portable_peak=ppk,portable_rms=prms,
                       active_duration_delta_ms=1000*((pe-pb)-(re-rb))/rr,
                       active_duration_ratio=((pe-pb)/(re-rb)) if re>rb else None,
                       peak_ratio=(ppk/rpk) if rpk else None,
                       rms_ratio=(prms/rrms) if rrms else None,
                       active_best_lag_ms=1000*lag/rr,active_search_corr=c)
        rows.append(row)
    ok=[r for r in rows if r['portable_status']=='ok']
    summary={'reference_voice':manifest.get('selectedVoice',{}).get('description'),
             'count':len(rows),'portable_ok':len(ok),'portable_missing':len(rows)-len(ok)}
    if ok:
        vals=[r['active_duration_ratio'] for r in ok if r.get('active_duration_ratio') is not None]
        gains=[1/r['peak_ratio'] for r in ok if r.get('peak_ratio')]
        summary.update(mean_active_duration_ratio=sum(vals)/len(vals),
                       median_peak_gain_needed=sorted(gains)[len(gains)//2],
                       mean_active_corr=sum(r['active_search_corr'] for r in ok)/len(ok))
    obj={'summary':summary,'rows':rows}
    print(json.dumps(obj,ensure_ascii=False,indent=2))
    if args.json_out: args.json_out.write_text(json.dumps(obj,ensure_ascii=False,indent=2),encoding='utf-8')
    if args.csv_out:
        keys=[]
        for r in rows:
            for k in r:
                if k not in keys: keys.append(k)
        with args.csv_out.open('w',newline='',encoding='utf-8-sig') as f:
            w=csv.DictWriter(f,fieldnames=keys); w.writeheader(); w.writerows(rows)
if __name__=='__main__': main()
