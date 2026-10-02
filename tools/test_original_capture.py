"""Local-only original SAPI capture/oracle checks. Requires installed x86 voice.

Use --windows-managed-launch explicitly if the inherited background launch
context hangs the legacy SDK. This never changes Windows jobs/permissions or
the installation. No words, candidate/phone bytes or WAV data enter the report.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import wave

from audit_m46_linguistics import audit, require


def pcm(path):
    with wave.open(str(path), 'rb') as stream:
        require((stream.getnchannels(), stream.getsampwidth(), stream.getframerate()) ==
                (1, 2, 16000), 'original_pcm_format')
        frames = stream.getnframes()
        data = stream.readframes(frames)
    require(frames > 0 and len(data) == frames * 2, 'original_pcm_length')
    return frames, hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--baseline-exe', type=Path, required=True)
    parser.add_argument('--corpus', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='Fresh PRIVATE artifact directory')
    parser.add_argument('--report', type=Path, required=True, help='Fresh scalar-only report')
    parser.add_argument('--windows-managed-launch', action='store_true')
    args = parser.parse_args()
    require(not args.output.exists() and not args.report.exists(), 'outputs_must_be_fresh')
    args.output.mkdir(parents=True)
    root = Path(__file__).resolve().parent.parent
    rows = []
    for line in args.corpus.read_text(encoding='utf-8-sig').splitlines():
        identity, text = line.split('\t', 1)
        require(identity and all(c.isascii() and (c.isalnum() or c in '-_') for c in identity), 'case_id')
        require(identity not in [row['case_id'] for row in rows], 'duplicate_case_id')
        source = args.output / f'{identity}.txt'
        source.write_text(text, encoding='utf-8')
        dirs = []
        for trace, exe in ((False, args.baseline_exe), (True, args.exe)):
            directory = args.output / f'{identity}-{"trace" if trace else "normal"}'
            command = ['powershell.exe', '-NoProfile', '-NonInteractive', '-File',
                       str(root / 'tools/run_original_capture.ps1'), '-Exe', str(exe.resolve()),
                       '-TextFile', str(source.resolve()), '-OutputDir', str(directory.resolve())]
            if trace:
                command.append('-Trace')
            if args.windows_managed_launch:
                command.append('-WindowsManagedLaunch')
            result = subprocess.run(command, capture_output=True, timeout=65)
            require(result.returncode == 0, f'owned_probe_failed:{identity}:{"trace" if trace else "normal"}')
            report = json.loads((directory / 'completion.json').read_text(encoding='utf-8-sig'))
            require(report['success'] and report['exit_code'] == 0, 'owned_exit')
            dirs.append(directory)
        with (dirs[1] / 'linguistics-m46.jsonl').open(encoding='utf-8') as stream:
            validation = audit(json.loads(line) for line in stream if line.strip())
        worker = (dirs[1] / 'linguistics-m46-worker.log').read_text(encoding='utf-8')
        final = json.loads(worker.strip().splitlines()[-1])
        require('M46_HOOKS_READY pinned_original=1' in worker and
                final['skipped'] == 0 and
                final['completed_calls'] == validation['complete_calls'] and
                final['records'] == 4 * validation['complete_calls'], 'complete_worker_capture')
        normal, traced = (pcm(directory / 'result.wav') for directory in dirs)
        row = dict(validation, case_id=identity, records=final['records'], skipped=final['skipped'],
                   normal_samples=normal[0], traced_samples=traced[0], pcm_equal=normal == traced)
        rows.append(row)
        print(json.dumps({key: row[key] for key in ('case_id', 'linguistic_calls',
            'zero_word_calls', 'scan_model_mismatches', 'pcm_equal')}), flush=True)
    report = {'schema': 'nicolai-m46b-original-corpus-v1', 'cases': len(rows),
              'exe_sha256': hashlib.sha256(args.exe.read_bytes()).hexdigest(),
              'baseline_exe_sha256': hashlib.sha256(args.baseline_exe.read_bytes()).hexdigest(),
              'corpus_sha256': hashlib.sha256(args.corpus.read_bytes()).hexdigest(),
              'windows_managed_launch': args.windows_managed_launch,
              'pcm_equal_pairs': sum(row['pcm_equal'] for row in rows),
              'complete_calls': sum(row['complete_calls'] for row in rows),
              'linguistic_calls': sum(row['linguistic_calls'] for row in rows),
              'zero_word_calls': sum(row['zero_word_calls'] for row in rows),
              'source_phone_records': sum(row['source_phone_records'] for row in rows),
              'scan_model_matches': sum(row['scan_model_matches'] for row in rows),
              'scan_model_mismatches': sum(row['scan_model_mismatches'] for row in rows),
              'skipped': sum(row['skipped'] for row in rows), 'rows': rows,
              'scope': 'Real stage/scan capture and original PCM isolation, not complete morphology or port acoustic parity.'}
    with args.report.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, indent=2); stream.write('\n')
    require(rows and report['pcm_equal_pairs'] == report['cases'], 'trace_changed_original_pcm')
    print(json.dumps({key: value for key, value in report.items() if key != 'rows'}), flush=True)


if __name__ == '__main__':
    main()
