#!/usr/bin/env python3
"""Compare normalized F0 trajectories for a PC reference pack and portable corpus.
Diagnostic only; requires numpy + librosa. Lower relative MAE is better.
"""
import argparse, os
import numpy as np
import librosa

def curve(path, bins=12):
    y, sr = librosa.load(path, sr=None, mono=True)
    y, _ = librosa.effects.trim(y, top_db=35)
    if len(y) < 1024:
        return np.full(bins, np.nan)
    f0 = librosa.yin(y, fmin=55, fmax=150, sr=sr, frame_length=1024, hop_length=160)
    rms = librosa.feature.rms(y=y, frame_length=1024, hop_length=160)[0][:len(f0)]
    f0 = f0[:len(rms)]
    gate = max(float(np.percentile(rms, 30)), 0.005)
    f0 = np.where(rms > gate, f0, np.nan)
    out=[]
    for k in range(bins):
        a=int(len(f0)*k/bins); b=max(a+1,int(len(f0)*(k+1)/bins))
        v=f0[a:b]; v=v[np.isfinite(v)]
        out.append(float(np.median(v)) if len(v) else np.nan)
    return np.asarray(out)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('reference_dir'); ap.add_argument('portable_dir'); ap.add_argument('--bins',type=int,default=12)
    a=ap.parse_args(); ids=[f'{i:03d}' for i in range(1,23)]
    refs=[]; ports=[]; rel=[]
    for i in ids:
        rp=os.path.join(a.reference_dir,i+'.wav'); pp=os.path.join(a.portable_dir,i+'.wav')
        if not (os.path.exists(rp) and os.path.exists(pp)): continue
        r=curve(rp,a.bins); p=curve(pp,a.bins); m=np.isfinite(r)&np.isfinite(p)
        if m.any(): rel.extend(((p[m]-r[m])/r[m]).tolist())
        refs.append(r); ports.append(p)
    print('phrases:',len(refs))
    print('F0 normalized-bin relative MAE:',float(np.mean(np.abs(rel))))
    print('reference median curve Hz:',np.round(np.nanmedian(np.asarray(refs),axis=0),1).tolist())
    print('portable  median curve Hz:',np.round(np.nanmedian(np.asarray(ports),axis=0),1).tolist())
if __name__=='__main__': main()
