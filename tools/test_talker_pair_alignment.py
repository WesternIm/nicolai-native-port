"""Voice-free checks for paired original/port gap alignment diagnostics."""
import json
from pathlib import Path
import unittest

import numpy as np

from audit_talker_pair_alignment import align, inspect_gap, read_joins, reference_quiet_ms


class TalkerPairAlignmentTest(unittest.TestCase):
    def test_identity_alignment_and_gain(self):
        t = np.arange(16000) / 16000
        source = (8000 * np.sin(2 * np.pi * (115 * t + 45 * t * t))).astype(np.float64)
        source[6000:6800] = 0
        identity = align(source, source)
        self.assertAlmostEqual(identity["mean_shape_cost"], 0, places=4)
        gap = inspect_gap(source, identity, 6000, 6800)
        self.assertTrue(gap["aligned"])
        self.assertLessEqual(abs(gap["reference_quiet_ms_in_span"] - 50), 20)
        gained = align(source, source * 0.5)
        self.assertLess(gained["mean_shape_cost"], 0.01)

    def test_quiet_overlap_never_exceeds_span_and_batch_log(self):
        samples = np.full(320, 4000.0)
        samples[160:] = 0
        self.assertAlmostEqual(reference_quiet_ms(samples, 155, 170), 0.625, places=1)
        self.assertEqual(read_joins("J\t012\t3\t#\t100\t80\t2\t4\t0.5\n",
                                    320, "012")[0]["phone"], "#")
        with self.assertRaises(ValueError):
            read_joins("J\t012\t3\t#\t100\t80\t2\t4\t0.5\n", 320, "013")

    def test_committed_scalar_summary_is_consistent(self):
        path = Path(__file__).resolve().parents[1] / "docs/metrics/m38-boundary-trial-20260928.json"
        report = json.loads(path.read_text(encoding="utf-8"))
        rows = report["changed_phrase_rows"]
        self.assertEqual(len(rows), 10)
        stable = report["reference_corpus"]["stable"]
        m38 = report["reference_corpus"]["m38_boundary_share_0_5"]
        self.assertAlmostEqual(sum(r["stable_shape"] for r in rows) / 10,
                               stable["mean_mfcc_dtw_cost"], places=3)
        self.assertAlmostEqual(sum(r["m38_shape"] for r in rows) / 10,
                               m38["mean_mfcc_dtw_cost"], places=3)
        self.assertAlmostEqual(sum(abs(r["stable_duration_error_ms"]) for r in rows) / 10,
                               stable["duration_mae_ms"], places=1)
        self.assertAlmostEqual(sum(abs(r["m38_duration_error_ms"]) for r in rows) / 10,
                               m38["duration_mae_ms"], places=1)
        self.assertAlmostEqual(sum(r["stable_word_excess_ms"] for r in rows),
                               stable["word_join_excess_quiet_ms"], places=1)
        self.assertAlmostEqual(sum(r["m38_word_excess_ms"] for r in rows),
                               m38["word_join_excess_quiet_ms"], places=1)
        self.assertEqual(sum(r["m38_shape"] < r["stable_shape"] for r in rows),
                         m38["mfcc_improved_phrases"])


if __name__ == "__main__":
    unittest.main()
