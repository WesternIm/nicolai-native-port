"""Account for M34 step target, carry, writer and terminal-flush clocks.

No waveform fitting, proportional correction or changes to the renderer.
Render logs must come from the M35 diagnostic build of all six M34 settings.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import wave
import numpy as np


CASES=("baseline","stateful-unit","stateful-shared","stateful-length","stateful-energy","stateful-both")


def wav_frames(path):
    with wave.open(str(path),"rb") as wav:
        if (wav.getnchannels(),wav.getsampwidth(),wav.getframerate())!=(1,2,16000):
            raise ValueError("Expected mono PCM16 16 kHz: "+str(path))
        return wav.getnframes()


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def audit(reference_pack,output_root,prior_root=None):
    items=json.loads((reference_pack/"manifest.json").read_text(encoding="utf-8-sig"))["results"]
    expected={f"{n:03}" for n in range(1,23)}
    if len(items)!=22 or {r["id"] for r in items}!=expected:
        raise ValueError("Expected 22 unique reference IDs")
    refs={r["id"]:wav_frames(reference_pack/r["wav"]) for r in items}
    rows=[]; summaries=[]
    for case in CASES:
        root=output_root/case
        clock={}; carry={}; pcm={}; units={}
        for line in (root/"render.log").read_text(encoding="utf-8-sig").splitlines():
            fields=line.split("\t")
            if fields[0]=="CLOCK": clock[fields[1]]=list(map(int,fields[2:]))
            elif fields[0]=="TDS": carry[fields[1]]=int(fields[-1])
            elif fields[0]=="P": pcm[fields[1]]=int(fields[2])
            elif fields[0]=="D": units.setdefault(fields[1],[]).append(float(fields[4])*16)
        if set(pcm)!=expected or (case!="baseline" and (set(clock)!=expected or set(carry)!=expected)):
            raise ValueError("Incomplete diagnostic log: "+case)
        selected=[]
        for key in sorted(expected):
            path=root/(key+".wav")
            total=wav_frames(path)
            if total!=pcm[key]: raise ValueError("WAV/log length mismatch")
            row=dict(candidate=case,id=key,reference_frames=refs[key],wav_frames=total,
                duration_error_frames=total-refs[key],raw_duration_ratio=total/refs[key])
            if prior_root:
                row["prior_sha256"]=digest(prior_root/case/(key+".wav"))
                row["current_sha256"]=digest(path)
                row["audio_unchanged"]=row["prior_sha256"]==row["current_sha256"]
                if not row["audio_unchanged"]: raise ValueError("Instrumentation changed audio: "+case+"/"+key)
            if case!="baseline":
                before,target,budget,emitted,clamped=clock[key]
                if before!=emitted or target-carry[key]!=budget:
                    raise ValueError("Clock conservation contract failed: "+case+"/"+key)
                if total-before!=4800: raise ValueError("Unexpected terminal flush")
                row.update(target_frames=target,budget_consumed_frames=budget,emitted_frames=emitted,
                    final_carry_frames=carry[key],writer_minus_budget_frames=emitted-budget,
                    writer_minus_target_frames=emitted-target,terminal_flush_frames=total-before,
                    target_plus_flush_error_frames=target+4800-refs[key],
                    clamped_delta_records=clamped,
                    requested_unit_duration_frames=sum(units[key]))
            selected.append(row); rows.append(row)
        summary=dict(candidate=case,phrases=22,total_duration_mae_percent=float(np.mean([abs(r["raw_duration_ratio"]-1)*100 for r in selected])),
            net_duration_error_frames=sum(r["duration_error_frames"] for r in selected))
        if prior_root: summary["unchanged_audio_count"]=sum(r["audio_unchanged"] for r in selected)
        if case!="baseline":
            summary.update(target_plus_flush_mae_percent=float(np.mean([abs(r["target_plus_flush_error_frames"])/r["reference_frames"]*100 for r in selected])),
                sum_target_frames=sum(r["target_frames"] for r in selected),sum_emitted_frames=sum(r["emitted_frames"] for r in selected),
                max_abs_writer_minus_budget_frames=max(abs(r["writer_minus_budget_frames"]) for r in selected),
                max_abs_writer_minus_target_frames=max(abs(r["writer_minus_target_frames"]) for r in selected),
                mean_abs_writer_minus_target_ms=float(np.mean([abs(r["writer_minus_target_frames"])/16 for r in selected])),
                clamped_delta_records=sum(r["clamped_delta_records"] for r in selected))
        summaries.append(summary)
    return dict(version="m35-clock-v1",contracts_passed=True,
        definitions=dict(target="sum floor(source interval width * selected duration Q11 / 2048), zero-target fallback to period",
            budget="sum previous carry + target - new carry; telescopes to target - final carry",
            emitted="sum periods actually written, before the 4800-frame terminal silence",
            unit_duration="portable duration request from D log; not captured PC feature records"),
        caveats=["Matching local clocks does not validate the target against PC phone coefficients",
            "WORD delta saturation and rounded writes can make emitted differ slightly from step budget",
            "This audit does not validate missing runtime rollback/source-selection behavior"],summary=summaries,rows=rows)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference_pack",type=Path)
    parser.add_argument("output_root",type=Path)
    parser.add_argument("--prior-root",type=Path)
    parser.add_argument("--json",type=Path,required=True)
    args=parser.parse_args()
    result=audit(args.reference_pack,args.output_root,args.prior_root)
    args.json.parent.mkdir(parents=True,exist_ok=True)
    args.json.write_text(json.dumps(result,indent=2,allow_nan=False)+"\n",encoding="utf-8")
    print(json.dumps(result["summary"],indent=2))


if __name__=="__main__": main()
