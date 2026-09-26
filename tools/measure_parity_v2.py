"""M35 diagnostic metrics, versioned separately from historical measure_parity.

pYIN voicing + independent normalized-autocorrelation cross-check. Shape DTW
uses RMS-normalized MFCC without C0; loudness is measured separately on the
same alignment. These are diagnostic estimates, not PC feature ground truth.
"""
from __future__ import annotations
import argparse
import hashlib
import importlib.metadata
import json
from pathlib import Path
import librosa
import numpy as np
from scipy.signal import find_peaks
from measure_parity import read_pcm, active_bounds

VERSION="m35-v2.2"
SR=16000
FRAME=2048
ENERGY_FRAME=1024
HOP=160
LOW=55
HIGH=400


def average(values):
    return float(np.mean(values)) if len(values) else None


def autocorrelation_pitch(y):
    """Earliest strong local peak near the best normalized peak, not YIN."""
    padded=np.pad(y,(FRAME//2,FRAME//2))
    f0=[]; confidence=[]
    lo=int(np.floor(SR/HIGH)); hi=int(np.ceil(SR/LOW))
    for start in range(0,len(y)+1,HOP):
        frame=padded[start:start+FRAME]
        frame=frame-np.mean(frame)
        energy=float(np.dot(frame,frame))
        if energy < 1e-12:
            f0.append(np.nan); confidence.append(0.0); continue
        corr=np.correlate(frame,frame,mode="full")[FRAME-1:]
        power=np.concatenate(([0.0],np.cumsum(frame*frame)))
        lag=np.arange(lo,hi+1)
        den=np.sqrt(np.maximum(0,(power[FRAME-lag])*(power[FRAME]-power[lag])))
        score=corr[lag]/np.maximum(den,1e-12)
        peaks,_=find_peaks(score)
        if not len(peaks):
            f0.append(np.nan); confidence.append(0.0); continue
        best=float(np.max(score[peaks]))
        eligible=peaks[(score[peaks]>=0.70)&(score[peaks]>=best-0.02)]
        if not len(eligible):
            f0.append(np.nan); confidence.append(best); continue
        k=int(eligible[0]); pitch_lag=float(lag[k])
        a,b,c=score[k-1:k+2]
        denom=a-2*b+c
        if abs(denom)>1e-12:
            pitch_lag+=float(np.clip(0.5*(a-c)/denom,-0.5,0.5))
        f0.append(SR/pitch_lag); confidence.append(float(score[k]))
    return np.asarray(f0),np.asarray(confidence)


def extract(samples,sr,cache=None):
    if sr!=SR or not len(samples): raise ValueError("Expected non-empty 16-kHz audio")
    begin,end=active_bounds(samples,sr)
    if end<=begin: # retain silence for calibration, never invent voiced frames
        begin,end=0,len(samples)
    y=(samples[begin:end]/32768.0).astype(np.float64)
    versions=tuple(importlib.metadata.version(k) for k in ("librosa","numpy","scipy","numba"))
    key=hashlib.sha256(repr((VERSION,versions,FRAME,ENERGY_FRAME,HOP,LOW,HIGH,
        sr,len(samples),begin,end)).encode()+y.tobytes()).hexdigest()
    target=cache/(key+".npz") if cache else None
    if target and target.exists():
        with np.load(target,allow_pickle=False) as stored:
            return {k:stored[k] for k in stored.files}
    # Global gain normalization is only for shape/pitch. Physical RMS stays raw.
    rms=float(np.sqrt(np.mean(y*y)))
    normalized=y/rms if rms>1e-12 else y
    f0,voiced,prob=librosa.pyin(normalized,fmin=LOW,fmax=HIGH,sr=sr,
        frame_length=FRAME,hop_length=HOP,fill_na=np.nan,center=True)
    ac,confidence=autocorrelation_pitch(normalized)
    shape=librosa.feature.mfcc(y=normalized.astype(np.float32),sr=sr,n_mfcc=13,
        n_fft=512,hop_length=HOP,center=True)[1:]
    level=librosa.feature.rms(y=y,frame_length=ENERGY_FRAME,hop_length=HOP,center=True)[0]
    if not (len(f0)==len(ac)==len(level)==shape.shape[1]):
        raise ValueError("Frame grids disagree")
    result=dict(f0=f0,voiced=voiced,voiced_prob=prob,ac_f0=ac,ac_confidence=confidence,
        shape=shape,level=level,bounds=np.array([begin,end]),total_frames=np.array(len(samples)))
    if target:
        target.parent.mkdir(parents=True,exist_ok=True)
        np.savez_compressed(target,**result)
    return result


def aligned_rows(ref,port):
    _,path=librosa.sequence.dtw(X=ref["shape"],Y=port["shape"],metric="euclidean",
        global_constraints=True,band_rad=0.20)
    # Collapse many-to-one DTW matches once per reference frame. Do not
    # overweight long repeated candidate regions by counting path entries.
    groups=[[] for _ in ref["f0"]]
    for r,p in path: groups[int(r)].append(int(p))
    if any(not g for g in groups): raise ValueError("Incomplete reference DTW coverage")
    def finite_median(values):
        finite=values[np.isfinite(values)]
        return float(np.median(finite)) if len(finite) else np.nan
    mapped={}
    for name in ("f0","ac_f0","level"):
        mapped[name]=np.array([finite_median(port[name][g]) for g in groups])
    mapped["voiced"]=np.array([np.mean(port["voiced"][g])>=0.5 for g in groups])
    shape_error=[float(np.mean(np.linalg.norm(ref["shape"][:,r,None]-port["shape"][:,g],axis=0)))
        for r,g in enumerate(groups)]
    return mapped,shape_error


def compare(ref,port):
    mapped,shape_error=aligned_rows(ref,port)
    rv=ref["voiced"]&np.isfinite(ref["f0"])
    pv=mapped["voiced"]&np.isfinite(mapped["f0"])
    matched=rv&pv
    cents=1200*np.log2(mapped["f0"][matched]/ref["f0"][matched])
    ref_speech=ref["level"]>0.003 # fixed raw amplitude floor, reported separately
    energy_db=20*np.log10(np.maximum(mapped["level"][ref_speech],1e-8)/ref["level"][ref_speech])
    acmatched=matched&np.isfinite(ref["ac_f0"])&np.isfinite(mapped["ac_f0"])
    ac_cents=1200*np.log2(mapped["ac_f0"][acmatched]/ref["ac_f0"][acmatched])
    independent_ac=np.isfinite(ref["ac_f0"])&np.isfinite(mapped["ac_f0"])
    independent_cents=1200*np.log2(mapped["ac_f0"][independent_ac]/ref["ac_f0"][independent_ac])
    extractor_agree=rv&np.isfinite(ref["ac_f0"])
    disagreement=1200*np.log2(ref["ac_f0"][extractor_agree]/ref["f0"][extractor_agree])
    return dict(shape_mfcc_dtw=average(shape_error),
        aligned_energy_db_mae=average(np.abs(energy_db)),energy_reference_frames=int(ref_speech.sum()),
        voiced_reference_frames=int(rv.sum()),voiced_matched_frames=int(matched.sum()),
        voiced_match_coverage=float(matched.sum()/rv.sum()) if rv.sum() else None,
        vuv_mismatch_fraction=float(np.mean(rv!=pv)),
        voiced_missed_frames=int((rv&~pv).sum()),voiced_false_positive_frames=int((~rv&pv).sum()),
        pyin_f0_mae_cents=average(np.abs(cents)),
        pyin_gross_pitch_error_fraction=average(np.abs(cents)>1200*np.log2(1.2)),
        pyin_octave_error_fraction=average(np.abs(np.abs(cents)-1200)<=100),
        ac_f0_mae_cents=average(np.abs(ac_cents)),ac_matched_frames=int(acmatched.sum()),
        independent_ac_f0_mae_cents=average(np.abs(independent_cents)),
        independent_ac_reference_frames=int(np.isfinite(ref["ac_f0"]).sum()),
        independent_ac_matched_frames=int(independent_ac.sum()),
        reference_extractors_disagree_fraction=average(np.abs(disagreement)>1200*np.log2(1.2)),
        reference_extractor_crosscheck_frames=int(extractor_agree.sum()),
        raw_total_duration_ratio=float(port["total_frames"]/ref["total_frames"]))


def calibration(cache):
    t=np.arange(SR)/SR
    rows=[]
    for hz in (83,120,166,220):
        y=12000*np.sin(2*np.pi*hz*t)
        features=extract(y,SR,cache)
        interior=slice(8,-8)
        fp=features["f0"][interior]; fa=features["ac_f0"][interior]
        rows.append(dict(actual_hz=hz,pyin_median_hz=float(np.nanmedian(fp)),
            ac_median_hz=float(np.nanmedian(fa)),voiced_fraction=float(np.mean(features["voiced"][interior]))))
    rng=np.random.default_rng(35)
    for name,y in (("silence",np.zeros(SR)),("white_noise",rng.normal(0,5000,SR))):
        f=extract(y,SR,cache)
        rows.append(dict(signal=name,pyin_voiced_fraction=float(np.mean(f["voiced"])),
            ac_voiced_fraction=float(np.mean(np.isfinite(f["ac_f0"])))))
    ref=extract(12000*np.sin(2*np.pi*166*t),SR,cache)
    controls={}
    for name,y in (("identity",12000*np.sin(2*np.pi*166*t)),
        ("half_gain",6000*np.sin(2*np.pi*166*t)),("octave_up",12000*np.sin(2*np.pi*332*t))):
        controls[name]=compare(ref,extract(y,SR,cache))
    # Contracts are against known synthetic truth, never fitted to golden WAVs.
    passed=all(abs(r["pyin_median_hz"]/r["actual_hz"]-1)<0.02 and
        abs(r["ac_median_hz"]/r["actual_hz"]-1)<0.02 and r["voiced_fraction"]>0.95 for r in rows[:4])
    passed=passed and all(r["pyin_voiced_fraction"]<0.05 and r["ac_voiced_fraction"]<0.05 for r in rows[4:6])
    passed=passed and controls["identity"]["pyin_f0_mae_cents"]==0 and abs(controls["half_gain"]["aligned_energy_db_mae"]-6.0205999)<0.05
    passed=passed and controls["half_gain"]["shape_mfcc_dtw"]<0.001 and controls["octave_up"]["pyin_octave_error_fraction"]>0.95
    # Nonstationary known truth and harmonics: a pure-tone-only pass is weak.
    phase=2*np.pi*(120*t+30*t*t) # instantaneous 120 -> 180 Hz
    harmonic=sum(np.sin(k*phase)/k for k in range(1,6))
    y=8000*harmonic*(0.65+0.35*np.sin(np.pi*t)**2)
    f=extract(y,SR,cache)
    centers=(f["bounds"][0]+np.arange(len(f["f0"]))*HOP)/SR
    truth=120+60*centers
    interior=(centers>0.15)&(centers<0.85)
    errors=1200*np.log2(f["f0"][interior]/truth[interior])
    ac_errors=1200*np.log2(f["ac_f0"][interior]/truth[interior])
    row=dict(signal="harmonic_chirp_120_to_180",pyin_mae_cents=float(np.nanmean(np.abs(errors))),
        ac_mae_cents=float(np.nanmean(np.abs(ac_errors))),
        pyin_voiced_fraction=float(np.mean(f["voiced"][interior])),
        ac_valid_fraction=float(np.mean(np.isfinite(ac_errors))))
    rows.append(row)
    passed=passed and row["pyin_mae_cents"]<100 and row["ac_mae_cents"]<100 and row["pyin_voiced_fraction"]>0.95 and row["ac_valid_fraction"]>0.95
    # Voiced, noisy-unvoiced, voiced. Ignore window-straddling transitions.
    y=10000*np.sin(2*np.pi*120*t)
    y[(t>=0.35)&(t<0.65)]=rng.normal(0,5000,np.sum((t>=0.35)&(t<0.65)))
    f=extract(y,SR,cache)
    centers=(f["bounds"][0]+np.arange(len(f["f0"]))*HOP)/SR
    v=((centers>0.15)&(centers<0.25))|((centers>0.75)&(centers<0.85))
    u=(centers>0.45)&(centers<0.55)
    row=dict(signal="voiced_noise_voiced",pyin_voiced_recall=float(np.mean(f["voiced"][v])),
        pyin_noise_false_positive=float(np.mean(f["voiced"][u])),
        ac_voiced_recall=float(np.mean(np.isfinite(f["ac_f0"][v]))),
        ac_noise_false_positive=float(np.mean(np.isfinite(f["ac_f0"][u]))))
    rows.append(row)
    passed=passed and row["pyin_voiced_recall"]>0.95 and row["ac_voiced_recall"]>0.95 and row["pyin_noise_false_positive"]<0.05 and row["ac_noise_false_positive"]<0.05
    return dict(passed=bool(passed),signals=rows,controls=controls)


def main():
    global FRAME
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference_pack",type=Path)
    parser.add_argument("output_root",type=Path)
    parser.add_argument("--candidates",nargs="+",default=["baseline","stateful-unit","stateful-shared"])
    parser.add_argument("--json",type=Path,required=True)
    parser.add_argument("--cache",type=Path)
    parser.add_argument("--pitch-frame",type=int,choices=(1024,2048),default=2048,
        help="Diagnostic sensitivity; shape and energy frame lengths remain fixed")
    args=parser.parse_args()
    FRAME=args.pitch_frame
    check=calibration(args.cache)
    if not check["passed"]: raise ValueError("Synthetic metric calibration failed: "+json.dumps(check))
    print("Synthetic calibration passed",flush=True)
    manifest=json.loads((args.reference_pack/"manifest.json").read_text(encoding="utf-8-sig"))
    items=manifest["results"]
    if len(items)!=22 or {i["id"] for i in items}!={f"{n:03}" for n in range(1,23)}:
        raise ValueError("Expected 22 unique reference IDs")
    refs={}
    for item in items:
        sr,y=read_pcm(args.reference_pack/item["wav"])
        refs[item["id"]]=extract(y,sr,args.cache)
    identity_rows=[dict(id=k,**compare(v,v)) for k,v in refs.items()]
    for row in identity_rows:
        if row["shape_mfcc_dtw"]!=0 or row["aligned_energy_db_mae"]!=0 or row["vuv_mismatch_fraction"]!=0 or row["raw_total_duration_ratio"]!=1 or row["pyin_f0_mae_cents"] not in (0,None):
            raise ValueError("Reference identity contract failed: "+json.dumps(row))
    summaries=[]; rows=[]
    for name in args.candidates:
        print("Measuring "+name,flush=True)
        selected=[]
        for item in items:
            sr,y=read_pcm(args.output_root/name/(item["id"]+".wav"))
            row=dict(candidate=name,id=item["id"],**compare(refs[item["id"]],extract(y,sr,args.cache)))
            selected.append(row); rows.append(row)
        summaries.append(dict(candidate=name,phrases=len(selected),
            phrases_with_matched_pyin=sum(r["voiced_matched_frames"]>0 for r in selected),
            pooled_voiced_match_coverage=sum(r["voiced_matched_frames"] for r in selected)/sum(r["voiced_reference_frames"] for r in selected) if sum(r["voiced_reference_frames"] for r in selected) else None,
            **{k:average([r[k] for r in selected if r[k] is not None]) for k in selected[0] if k not in ("candidate","id")}))
    result=dict(version=VERSION,
        definitions=dict(pyin_range_hz=[LOW,HIGH],pitch_frame_samples=FRAME,energy_frame_samples=ENERGY_FRAME,hop_samples=HOP,
            alignment="RMS-normalized MFCC 1..12, DTW band_rad=0.20, one vote per reference frame",
            vuv="pYIN Viterbi voiced flags; no percentile RMS gate",
            energy="raw RMS on reference frames >0.003, compared on the same shape alignment",
            calibration_scope="four tones, harmonic chirp, voiced-noise transitions, silence, white noise, gain and octave controls; not speech ground truth"),
        caveats=["pYIN and autocorrelation are estimates, not captured PC feature records",
            "Conditional F0 errors exclude missing frames; coverage and VUV errors must be read together",
            "DTW can align wrong phones; no independent phone alignment is available",
            "Candidate summary is equal phrase means, not globally frame weighted"],
        calibration=check,reference_identity=dict(phrases=22,passed=True,rows=identity_rows),summary=summaries,rows=rows)
    args.json.parent.mkdir(parents=True,exist_ok=True)
    args.json.write_text(json.dumps(result,indent=2,allow_nan=False)+"\n",encoding="utf-8")
    print(json.dumps(summaries,indent=2),flush=True)


if __name__=="__main__":
    main()
