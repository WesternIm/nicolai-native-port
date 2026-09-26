"""Calibration and sensitivity audit; does not replace historical parity metrics.

Original inputs stay read-only. Controls are in-memory transforms, not audio
artifacts. Raw frame counts are independently reported alongside active bounds.
"""
import argparse
import json
from pathlib import Path
import librosa
import numpy as np
from measure_parity import read_pcm, active_bounds, best_lag, f0_curve, mfcc_dtw


def rms(y):
    return float(np.sqrt(np.mean(y*y))) if y.size else 0.0


def curve(y, sr, high):
    signal=(y/32768.0).astype(np.float32)
    signal,_=librosa.effects.trim(signal,top_db=35)
    if signal.size < 1024:
        return np.full(12,np.nan)
    f0=librosa.yin(signal,fmin=55,fmax=high,sr=sr,frame_length=1024,hop_length=160)
    level=librosa.feature.rms(y=signal,frame_length=1024,hop_length=160)[0][:f0.size]
    f0=np.where(level>max(float(np.percentile(level,30)),0.005),f0[:level.size],np.nan)
    bins=[]
    for i in range(12):
        a=int(f0.size*i/12); b=max(a+1,int(f0.size*(i+1)/12))
        values=f0[a:b]; values=values[np.isfinite(values)]
        bins.append(float(np.median(values)) if values.size else np.nan)
    return np.asarray(bins)


def pair_metrics(a,b,sr):
    aa=a[slice(*active_bounds(a,sr))]; bb=b[slice(*active_bounds(b,sr))]
    x,y=f0_curve(a,sr),f0_curve(b,sr)
    valid=np.isfinite(x)&np.isfinite(y)
    lag,corr=best_lag(aa,bb,sr)
    return dict(correlation=corr,lag_ms=lag*1000/sr,
        total_duration_ratio=b.size/a.size,active_duration_ratio=bb.size/aa.size,
        total_rms_ratio=rms(b)/rms(a),active_rms_ratio=rms(bb)/rms(aa),
        f0_mae_percent=100*float(np.mean(np.abs(y[valid]/x[valid]-1))),
        matched_f0_bins=int(valid.sum()),mfcc_dtw=mfcc_dtw(aa,bb,sr))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("reference_pack",type=Path)
    p.add_argument("output_root",type=Path)
    p.add_argument("--json",type=Path,required=True)
    args=p.parse_args()
    manifest=json.loads((args.reference_pack/"manifest.json").read_text(encoding="utf-8-sig"))
    if {r["id"] for r in manifest["results"]}!={f"{i:03}" for i in range(1,23)} or len(manifest["results"])!=22:
        raise ValueError("Expected unique reference IDs 001..022")
    refs={r["id"]:read_pcm(args.reference_pack/r["wav"]) for r in manifest["results"]}
    identity=[dict(id=i,**pair_metrics(y,y,sr)) for i,(sr,y) in refs.items()]
    sr,y=refs["001"]
    controls={
        "identity":pair_metrics(y,y,sr),
        "half_gain":pair_metrics(y,y*0.5,sr),
        "polarity_flip":pair_metrics(y,-y,sr),
        "append_300ms_silence":pair_metrics(y,np.pad(y,(0,int(sr*0.3))),sr),
    }
    tones=[]
    for hz in (83,120,166,220):
        tone=12000*np.sin(2*np.pi*hz*np.arange(sr)/sr)
        x=f0_curve(tone,sr); wide=curve(tone,sr,400)
        tones.append(dict(actual_hz=hz,narrow_median_hz=float(np.nanmedian(x)),
            wide_median_hz=float(np.nanmedian(wide))))
    summaries=[]; rows=[]
    # Reuse saved historical outputs, never rerender/relabel candidates here.
    settings=json.loads((args.output_root/"summary.json").read_text(encoding="utf-8-sig"))
    ref_curves={i:{h:curve(y,sr,h) for h in (150,400)} for i,(sr,y) in refs.items()}
    for setting in settings:
        name=setting["candidate"]; errors={150:[],400:[]}; rms_errors=[]; total_errors=[]; active_errors=[]
        for i,(sr,a) in refs.items():
            ps,b=read_pcm(args.output_root/name/(i+".wav"))
            if ps!=sr: raise ValueError("sample-rate mismatch")
            aa=a[slice(*active_bounds(a,sr))]; bb=b[slice(*active_bounds(b,sr))]
            total=b.size/a.size; active=bb.size/aa.size
            active_rms=rms(bb)/rms(aa)
            row=dict(candidate=name,id=i,reference_frames=int(a.size),candidate_frames=int(b.size),
                raw_total_duration_ratio=total,active_duration_ratio=active,active_rms_ratio=active_rms)
            for high in (150,400):
                x=ref_curves[i][high]; yc=curve(b,sr,high)
                valid=np.isfinite(x)&np.isfinite(yc)&(x!=0)
                e=np.abs(yc[valid]/x[valid]-1); errors[high].extend(e.tolist())
                row[f"yin_{high}_matched_bins"]=int(valid.sum())
                row[f"yin_{high}_mae_percent"]=float(np.mean(e))*100 if e.size else None
            rows.append(row); total_errors.append(abs(total-1)); active_errors.append(abs(active-1)); rms_errors.append(abs(active_rms-1))
        summaries.append(dict(candidate=name,raw_total_duration_mae_percent=100*float(np.mean(total_errors)),
            active_duration_mae_percent=100*float(np.mean(active_errors)),
            active_rms_mae_percent=100*float(np.mean(rms_errors)),
            narrow_f0_mae_percent=100*float(np.mean(errors[150])),
            wide_f0_mae_percent=100*float(np.mean(errors[400])),
            narrow_matched_f0_bins=len(errors[150]),wide_matched_f0_bins=len(errors[400])))
    result=dict(definition="Historical metric calibration + sensitivity, not a new promotion gate",
        caveat="Wide-band YIN is not ground truth or voiced/unvoiced classification. Controls use phrase 001; identity uses all 22.",
        identity=identity,controls=controls,tone_calibration=tones,candidate_summary=summaries,rows=rows)
    args.json.parent.mkdir(parents=True,exist_ok=True)
    args.json.write_text(json.dumps(result,ensure_ascii=False,indent=2,allow_nan=False)+"\n",encoding="utf-8")
    print(json.dumps(dict(controls=controls,tone_calibration=tones,candidate_summary=summaries),indent=2))


if __name__=="__main__":
    main()
