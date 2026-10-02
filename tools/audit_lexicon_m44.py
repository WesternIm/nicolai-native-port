"""Compare the partial original-lexicon reader with exception stress labels.

This is lexical resource agreement, NOT original NLP or perceptual parity.
Inputs and per-word diagnostics stay local; the output contains scalars only.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--voice', type=Path, required=True)
    parser.add_argument('--exceptions', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--dual-m47', action='store_true', help='Use the M47 probe and compare both lookup lanes')
    parser.add_argument('--dual-m48', action='store_true', help='Use the M48 probe and compare M47/M48 lookup lanes')
    parser.add_argument('--dual-m49', action='store_true', help='Use the M49 probe and compare M48/M49 lookup lanes')
    parser.add_argument('--policy', type=Path, help='PRIVATE selector data for M49')
    args = parser.parse_args()
    if sum((args.dual_m47,args.dual_m48,args.dual_m49))>1:
        parser.error('Choose only one dual comparison')
    if args.dual_m49 and not args.policy:
        parser.error('--dual-m49 requires --policy')
    labels = {}
    for line in args.exceptions.read_bytes().decode('cp1251').splitlines():
        if ':' not in line or line.lstrip().startswith('//'):
            continue
        word, rhs = line.split(':', 1)
        word = word.strip().lower()
        if not re.fullmatch('[а-яё]+', word):
            continue
        first = rhs.find('<')
        second = rhs.find('<', first + 1)
        if first < 0 or second < 0:
            continue
        stress = sum(c.lower() in 'аеёиоуыэюя' for c in rhs[first+1:second]) - 1
        if stress >= 0:
            labels[word] = stress
    words = list(labels)
    with tempfile.TemporaryDirectory(prefix='nicolai-lexicon-m44-') as root:
        corpus = Path(root) / 'words.tsv'
        corpus.write_text(''.join(f'{i}\t{word}\n' for i, word in enumerate(words)), encoding='utf-8')
        command=[str(args.probe.resolve()),str(args.voice.resolve()),str(corpus)]
        if args.dual_m49:
            command.append(str(args.policy.resolve()))
        result = subprocess.run(command,
                                capture_output=True, timeout=60, check=True)
    counts = Counter()
    agree = disagree = 0
    old_agree = old_disagree = added = lost = 0
    changed = 0
    yo_accepted = 0
    seen = set()
    for line in result.stdout.decode('utf-8').splitlines():
        fields = line.split('\t')
        if args.dual_m49:
            fields,yo=fields[:7],fields[7]
            yo_accepted += fields[4]=='accepted' and yo!='-'
        if args.dual_m47 or args.dual_m48 or args.dual_m49:
            index, old_status, old_stress, _, status, stress, candidates = fields
            if old_status == 'accepted':
                old_agree += int(old_stress) == labels[words[int(index)]]
                old_disagree += int(old_stress) != labels[words[int(index)]]
            added += old_status != 'accepted' and status == 'accepted'
            lost += old_status == 'accepted' and status != 'accepted'
            changed += old_status == status == 'accepted' and old_stress != stress
        else:
            index, status, stress, candidates = fields
        index = int(index)
        assert 0 <= index < len(words) and index not in seen
        seen.add(index)
        counts[status] += 1
        if status == 'accepted':
            if int(stress) == labels[words[index]]:
                agree += 1
            else:
                disagree += 1
    assert len(seen) == len(words)
    geometry = result.stderr.decode('utf-8').strip().split('\t')
    assert geometry[0] == 'LEXICON'
    summary = {
        'schema': 'nicolai-m44-lexicon-resource-agreement-v1',
        'voice_sha256': hashlib.sha256(args.voice.read_bytes()).hexdigest(),
        'exceptions_sha256': hashlib.sha256(args.exceptions.read_bytes()).hexdigest(),
        'blocks': int(geometry[1]), 'records': int(geometry[2]), 'folded_stems': int(geometry[3]),
        'exception_words': len(words), 'status_counts': dict(counts),
        'accepted_agree': agree, 'accepted_disagree': disagree,
        'accepted_agreement_percent': 100 * agree / (agree + disagree) if agree + disagree else None,
        'limits': 'Partial stem-stress subset; exception dictionary wins in rendering. Resource agreement is not full original NLP or perceptual parity.',
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    if args.dual_m47:
        summary.update(schema='nicolai-m47-lexicon-resource-agreement-v1',
                       m44_accepted_agree=old_agree, m44_accepted_disagree=old_disagree,
                       m47_additional_accepted=added, m47_declined_previous_accepted=lost)
    if args.dual_m48:
        summary.update(schema='nicolai-m48-lexicon-resource-agreement-v1',
                       m47_accepted_agree=old_agree, m47_accepted_disagree=old_disagree,
                       m48_additional_accepted=added, m48_declined_previous_accepted=lost,
                       m48_changed_previous_accepted=changed,
                       limits='Partial ending-stress subset; selected е and ambiguous analyses are declined. Exception dictionary wins in rendering. Resource agreement is not full NLP/perceptual parity.')
    if args.dual_m49:
        summary.update(schema='nicolai-m49-lexicon-resource-agreement-v1',
                       m48_accepted_agree=old_agree,m48_accepted_disagree=old_disagree,
                       m49_additional_accepted=added,m49_declined_previous_accepted=lost,
                       m49_changed_previous_accepted=changed,m49_accepted_yo_choices=yo_accepted,
                       policy_sha256=hashlib.sha256(args.policy.read_bytes()).hexdigest(),
                       limits='Ending selector subset only; unknown noun-positive filters are refused. Exact exception entries win rendering. Resource stress agreement is not yo correctness, full NLP or acoustic parity.')
    args.out.write_text(json.dumps(summary, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(summary))


if __name__ == '__main__':
    main()
