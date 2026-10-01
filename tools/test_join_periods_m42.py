"""Voice-free calibration and missing-evidence contracts for the M42 screen."""
import math
import unittest

import numpy as np

from audit_join_periods_m42 import cents_gap, estimate_period, matched_comparison, parse_periods, side_periods, summarize


class JoinPeriodsTest(unittest.TestCase):
    def test_known_periods_gain_and_dc(self):
        t = np.arange(640) / 16000
        for hz in (65, 80, 100, 125, 180, 240):
            for gain, dc in ((8000, 0), (1000, 4000), (-4000, -1000)):
                pcm = gain * np.sin(2 * np.pi * hz * t + 0.3) + dc
                result = estimate_period(pcm)
                self.assertIsNotNone(result["period"], hz)
                self.assertLess(abs(result["period"] - 16000 / hz), 0.2)
                self.assertGreater(result["confidence"], 0.99)

    def test_no_forced_pitch_for_noise_silence_or_short_window(self):
        for x in (np.zeros(640), np.ones(640) * 4000, np.ones(639),
                  np.random.default_rng(42).normal(0, 4000, 640), np.full(640, np.nan)):
            self.assertIsNone(estimate_period(x)["period"])

    def test_harmonics_do_not_force_half_or_double_period(self):
        t = np.arange(640) / 16000
        for hz in (80, 100, 125, 180):
            pcm = 3000 * np.sin(2 * np.pi * hz * t) + 6000 * np.sin(4 * np.pi * hz * t + 0.2)
            result = estimate_period(pcm)
            self.assertIsNotNone(result["period"])
            self.assertLess(abs(result["period"] - 16000 / hz), 0.3)

    def test_known_join_gap(self):
        t = np.arange(1600) / 16000
        pcm = np.r_[8000 * np.sin(2 * np.pi * 100 * t), 8000 * np.sin(2 * np.pi * 125 * t)]
        sides = side_periods(pcm, 1600, 80)
        self.assertAlmostEqual(sides["gap_cents"], 1200 * math.log2(1.25), delta=1)
        self.assertIsNone(side_periods(pcm, 0, 80)["gap_cents"])
        self.assertIsNone(cents_gap(0, 10))
        self.assertIsNone(cents_gap(math.nan, 10))

    def test_parser_and_coverage(self):
        rows = parse_periods("JP\t001\t1\ta0\t160\t128\nJP\t001\t2\ts\t0\t0\n")
        self.assertAlmostEqual(rows[("001", 1)]["gap_cents"], 386.313714, places=6)
        report = summarize([{"id": key[0], "authored": value} for key, value in rows.items()])
        self.assertEqual(report["authored"]["eligible"], 1)
        self.assertEqual(report["authored"]["missing"], 1)
        self.assertEqual(report["portable"]["eligible"], 0)
        for bad in ("", "JP\t001\t1\ta0\t-2\t128", "JP\t001\t1\ta0\tnan\t128",
                    "JP\t001\t1\ta0\t160", "JP\t001\t1\ta0\t160\t128\nJP\t001\t1\ta0\t160\t128"):
            with self.assertRaises(ValueError):
                parse_periods(bad)

    def test_matched_coverage_cannot_hide_missing_tracks(self):
        a = [{"id": "001", "phone_index": 1, "phone": "a0",
              "authored": {"gap_cents": 100}, "portable": {"gap_cents": 200}},
             {"id": "001", "phone_index": 2, "phone": "m",
              "authored": {"gap_cents": 50}, "portable": {"gap_cents": None}}]
        b = [{**a[0], "portable": {"gap_cents": 150}},
             {**a[1], "portable": {"gap_cents": 20}}]
        report = matched_comparison(a, b)
        self.assertEqual(report["portable"]["matched_eligible"], 1)
        self.assertEqual(report["portable"]["excluded"], 1)
        self.assertEqual(report["portable"]["trial_mean_gap_cents"], 150)
        with self.assertRaises(ValueError):
            matched_comparison(a, b[:1])
        with self.assertRaises(ValueError):
            matched_comparison(a, b + [b[0]])
        with self.assertRaises(ValueError):
            matched_comparison(a, [{**b[0], "phone": "e0"}, b[1]])


if __name__ == "__main__":
    unittest.main()
