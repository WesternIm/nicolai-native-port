"""Voice-free contracts for reproducible low-energy/join diagnostics."""
from array import array
import unittest

from audit_talker_joins import low_energy_intervals, parse_joins, summarize


class JoinAuditTest(unittest.TestCase):
    def test_gap_and_join_position(self):
        samples = array("h", [4000] * 1600 + [0] * 640 + [4000] * 1600)
        log = "profile=stable\njoin=2,a1,1800,80,7,8,0.75\n"
        joins = parse_joins(log, len(samples))
        self.assertEqual(low_energy_intervals(samples, 16000), [(1600, 2240)])
        result = summarize(16000, samples, joins)
        self.assertEqual(result["low_energy_interval_count"], 1)
        self.assertEqual(result["gaps"][0]["nearest_join"]["phone"], "a1")
        self.assertEqual(result["gaps"][0]["nearest_join"]["distance_ms"], 0)

    def test_short_gap_and_bad_coordinates(self):
        samples = array("h", [4000] * 1600 + [0] * 320 + [4000] * 1600)
        self.assertEqual(low_energy_intervals(samples, 16000), [])
        with self.assertRaises(ValueError):
            parse_joins("join=1,#,9999,80,0,0,0.5", len(samples))
        with self.assertRaises(ValueError):
            parse_joins("profile=stable", len(samples))


if __name__ == "__main__":
    unittest.main()
