"""Reproducible local M51 original oracle; raw captures/exports stay PRIVATE."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from audit_m46_linguistics import require
from audit_m51_analysis import audit


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture-root', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--original', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='Fresh PRIVATE directory')
    parser.add_argument('--report', type=Path, required=True, help='Fresh scalar-only output')
    args = parser.parse_args()
    require(not args.output.exists() and not args.report.exists(), 'outputs_must_be_fresh')
    args.output.mkdir(parents=True)
    files = sorted(args.capture_root.glob('*-trace/linguistics-m46.jsonl'))
    require(0 < len(files) <= 128, 'bounded_capture_cases')
    summaries, originals = [], []
    for index, file in enumerate(files):
        require(file.stat().st_size <= 64 * 1024 * 1024, 'capture_too_large')
        private = args.output / f'payloads-{index}.bin'
        with file.open(encoding='utf-8') as stream:
            summaries.append(audit((json.loads(line) for line in stream if line.strip()), private))
        result = subprocess.run([str(args.probe.resolve()), '--original', str(args.original.resolve()),
                                 '--input', str(private.resolve())], capture_output=True, timeout=30)
        require(result.returncode == 0, 'original_candidate_gate_probe_failed')
        oracle = json.loads(result.stdout)
        require(oracle['unexpected_buffer_mutations'] == 0 and oracle['synthetic_original_matches'] == 2076 and
                oracle['synthetic_normalized_payload_matches'] == 65536 and
                oracle['real_normalization_word_matches'] == summaries[-1]['analyzed_words'] and
                oracle['captured_payload_matches'] == summaries[-1]['private_probe_rows'], 'original_probe_count')
        originals.append(oracle)
    rejected = 0
    # No DLL code executes with a mismatched whole-file hash.
    wrong_dll = args.output / 'not-an-original.dll'
    wrong_dll.write_bytes(b'authored hash-refusal fixture')
    result = subprocess.run([str(args.probe.resolve()), '--original', str(wrong_dll)],
                            capture_output=True, timeout=10)
    require(result.returncode != 0 and b'unsupported_original_sha256' in result.stderr, 'wrong_hash_accepted')
    rejected += 1
    for name, data in [('oversized', b'N51CAND1' + (1).to_bytes(4, 'little') + bytes([71])),
                       ('truncated', b'N51CAND1' + (1).to_bytes(4, 'little') + bytes([1])),
                       ('trailing', b'N51CAND1' + bytes(4) + b'x'),
                       ('normalizer-truncated', b'N51CAND2' + bytes(4) +
                        (1).to_bytes(4, 'little') + bytes([1])),
                       ('normalizer-mismatch', b'N51CAND2' + bytes(4) +
                        (1).to_bytes(4, 'little') + bytes([1]) + bytes(20) + bytes([1]) + bytes(20))]:
        file = args.output / f'{name}.bin'; file.write_bytes(data)
        result = subprocess.run([str(args.probe.resolve()), '--original', str(args.original.resolve()),
                                 '--input', str(file.resolve())], capture_output=True, timeout=30)
        require(result.returncode != 0, 'invalid_payload_export_accepted')
        rejected += 1
    counts = {key: sum(row[key] for row in summaries) for key, value in summaries[0].items()
              if type(value) is int}
    report = dict(schema='nicolai-m51-local-oracle-v1', cases=len(summaries), **counts,
                  synthetic_original_matches=sum(row['synthetic_original_matches'] for row in originals),
                  captured_payload_matches=sum(row['captured_payload_matches'] for row in originals),
                  synthetic_normalized_payload_matches=sum(row['synthetic_normalized_payload_matches'] for row in originals),
                  captured_normalized_payload_matches=sum(row['captured_normalized_payload_matches'] for row in originals),
                  real_normalization_word_matches=sum(row['real_normalization_word_matches'] for row in originals),
                  real_normalization_payload_matches=sum(row['real_normalization_payload_matches'] for row in originals),
                  negative_probe_refusals=rejected, unexpected_buffer_mutations=0,
                  exe_sha256=hashlib.sha256(args.probe.read_bytes()).hexdigest(),
                  original_sha256=hashlib.sha256(args.original.read_bytes()).hexdigest(),
                  scope='Actual candidate layout, original form-normalization/conflict primitives and selected annotations; no complete morphology/context or audio-parity claim.')
    require(report['selected_annotation_matches'] == report['selected_annotation_checked_words'] and
            report['common_triple_score_mismatches'] == report['scan_model_mismatches'] == 0,
            'actual_selection_observation_mismatch')
    with args.report.open('x', encoding='utf-8') as stream:
        json.dump(report, stream, indent=2); stream.write('\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
