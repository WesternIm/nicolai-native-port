#!/usr/bin/env python3
"""Synthetic contracts for the M39 spectral screen (no proprietary inputs)."""
import unittest

import numpy as np

from audit_spectral_balance_m39 import summarize


class SpectralBalanceTests(unittest.TestCase):
    def test_gain_invariance_and_high_band_sensitivity(self):
        rate = 16000
        t = np.arange(rate * 2) / rate
        low = np.sin(2 * np.pi * 800 * t)
        high = low + 0.25 * np.sin(2 * np.pi * 4000 * t)
        low_pcm = np.rint(low * 12000).astype(np.int16).astype(np.float64)
        high_pcm = np.rint(high * 12000).astype(np.int16).astype(np.float64)
        quiet_pcm = np.rint(high * 6000).astype(np.int16).astype(np.float64)
        base = summarize(low_pcm, rate)
        changed = summarize(high_pcm, rate)
        quiet = summarize(quiet_pcm, rate)
        self.assertGreater(changed["high_to_low_median_db"], base["high_to_low_median_db"] + 20)
        self.assertAlmostEqual(changed["high_to_low_median_db"], quiet["high_to_low_median_db"], delta=0.2)

    def test_rejects_wrong_rate_and_short_signal(self):
        with self.assertRaises(ValueError):
            summarize(np.ones(4000), 8000)
        with self.assertRaises(ValueError):
            summarize(np.ones(200), 16000)


if __name__ == "__main__":
    unittest.main()
