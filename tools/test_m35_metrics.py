"""Voice-free metric contracts; run separately from the 20 portable C++ tests."""
import tempfile
import unittest
from pathlib import Path
import numpy as np
import measure_parity_v2 as metric


class MetricContracts(unittest.TestCase):
    def setUp(self):
        metric.FRAME=2048
        self.t=np.arange(metric.SR)/metric.SR
        self.tone=12000*np.sin(2*np.pi*166*self.t)

    def test_known_truth_1024(self):
        metric.FRAME=1024
        self.assertTrue(metric.calibration(None)["passed"])

    def test_known_truth_2048(self):
        self.assertTrue(metric.calibration(None)["passed"])

    def test_cache_does_not_hide_silence_duration(self):
        # Same active signal but a different raw duration must not hit the same
        # cached total_frames/bounds. No audio is shipped or used as a fixture.
        with tempfile.TemporaryDirectory() as directory:
            cache=Path(directory)
            a=metric.extract(self.tone,metric.SR,cache)
            b=metric.extract(np.concatenate((self.tone,np.zeros(4800))),metric.SR,cache)
            repeated=metric.extract(self.tone,metric.SR,cache)
            self.assertEqual(int(a["total_frames"]),16000)
            self.assertEqual(int(b["total_frames"]),20800)
            self.assertEqual(int(repeated["total_frames"]),16000)
            self.assertAlmostEqual(metric.compare(a,b)["raw_total_duration_ratio"],1.3)

    def test_missing_f0_is_null_not_zero(self):
        ref=metric.extract(self.tone,metric.SR)
        port={key:value.copy() for key,value in ref.items()}
        port["voiced"][:]=False
        port["f0"][:]=np.nan
        row=metric.compare(ref,port)
        self.assertIsNone(row["pyin_f0_mae_cents"])
        self.assertEqual(row["voiced_match_coverage"],0)
        self.assertGreater(row["voiced_missed_frames"],0)
        # The AC cross-check is also reported independently of pYIN admission.
        self.assertEqual(row["independent_ac_f0_mae_cents"],0)
        self.assertGreater(row["independent_ac_matched_frames"],0)

    def test_dtw_each_reference_frame_once(self):
        ref=metric.extract(self.tone,metric.SR)
        port={key:value.copy() for key,value in ref.items()}
        for name in ("f0","voiced","ac_f0","level"):
            port[name]=np.repeat(port[name],2,axis=0)
        port["shape"]=np.repeat(port["shape"],2,axis=1)
        mapped,shape=metric.aligned_rows(ref,port)
        self.assertEqual(len(mapped["f0"]),len(ref["f0"]))
        self.assertEqual(len(shape),len(ref["f0"]))
        self.assertEqual(metric.compare(ref,port)["pyin_f0_mae_cents"],0)

    def test_wrong_rate_rejected(self):
        with self.assertRaises(ValueError): metric.extract(self.tone,8000)


if __name__=="__main__": unittest.main()
