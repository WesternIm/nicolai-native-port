import unittest
from audit_rhythm_m43 import gap_summary, read_budgets, summarize


class RhythmTest(unittest.TestCase):
    def test_budget_validation(self):
        result = read_budgets("WT\t001\t1\t3\t600\t600\t0.5\n")
        self.assertEqual(result["words"], 1)
        self.assertEqual(result["max_logged_budget_delta_samples"], 0)
        self.assertEqual(read_budgets("")["words"], 0)
        for bad in ("WT\t001\t1\t3\t600\t600", "WT\t001\t3\t1\t600\t600\t0.5",
                    "WT\t001\t1\t3\tnan\t600\t0.5", "WT\t001\t1\t3\t600\t600\t2",
                    "WT\t001\t1\t3\t600\t600\t0.5\nWT\t001\t1\t3\t600\t600\t0.5"):
            with self.assertRaises(ValueError):
                read_budgets(bad)

    def test_word_gap_screen_keeps_missing_alignment(self):
        base = {"nearest_join": {"phone": "#", "distance_ms": 0}, "aligned": True,
                "portable_quiet_ms": 90, "reference_quiet_ms_in_span": 30}
        report = {"portable_gaps": [base, {"nearest_join": base["nearest_join"], "aligned": False},
                                    {**base, "nearest_join": {"phone": "t", "distance_ms": 0}}],
                  "stable_port": {"low_energy_total_ms": 200, "duration_ms": 1000},
                  "original": {"low_energy_total_ms": 150, "duration_ms": 1100}, "mean_shape_cost": 40}
        result = gap_summary(report)
        self.assertEqual(result["word_join_intervals"], 2)
        self.assertEqual(result["word_join_mapped_intervals"], 1)
        self.assertEqual(result["word_join_unmapped_intervals"], 1)
        self.assertEqual(result["word_join_excess_quiet_ms"], 60)
        self.assertEqual(result["duration_error_ms"], -100)
        total = summarize([{"baseline": result, "trial": result}])
        self.assertEqual(total["baseline"]["duration_mae_ms"], 100)
        self.assertEqual(total["higher_shape_cost"], 0)
        with self.assertRaises(ValueError):
            summarize([])


if __name__ == "__main__":
    unittest.main()
