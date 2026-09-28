"""Voice-free synthetic contracts; never count these as live oracle results."""
import argparse
import copy
from pathlib import Path
import unittest
from audit_m36_routes import audit, SCHEMA


def fixture(count=2, cross=0, started=0, writer="initial", index=0, nodes=2):
    def row(event, **values):
        return {"schema": SCHEMA, "thread_id": 1, "call": 1, "event": event, **values}
    rows = [row("entry", current={"node_count": nodes, "source_position": list(range(nodes))})]
    rows.append(row("decision", interval_index=index, step=[index, count, 100, 0, 7],
                    cross_pending=cross, already_started=started))
    if writer:
        rows.append(row(writer))
    rows.extend([row("interval_end"), row("post_loop"), row("return", return_value=1)])
    return rows


class Contracts(unittest.TestCase):
    def test_initial(self):
        self.assertEqual(audit(fixture(), PROBE)["mismatches"], 0)

    def test_cross_priority(self):
        self.assertEqual(audit(fixture(cross=1, started=1, writer="cross"), PROBE)["mismatches"], 0)

    def test_drop_priority(self):
        self.assertEqual(audit(fixture(count=0, cross=1, writer=None), PROBE)["mismatches"], 0)

    def test_deferred(self):
        self.assertEqual(audit(fixture(started=1, writer=None), PROBE)["mismatches"], 0)

    def test_ordinary_and_complete_intervals(self):
        rows = fixture(started=1, writer="ordinary", nodes=3)
        tail = fixture(started=1, writer=None, index=1, nodes=3)[1:3]
        rows[4:4] = tail
        self.assertEqual(audit(rows, PROBE)["decisions"], 2)

    def test_wrong_writer(self):
        self.assertEqual(audit(fixture(writer="cross"), PROBE)["mismatches"], 1)

    def test_empty(self):
        with self.assertRaises(ValueError): audit([], PROBE)

    def test_truncated(self):
        with self.assertRaises(ValueError): audit(fixture()[:-1], PROBE)

    def test_missing_interval(self):
        with self.assertRaises(ValueError): audit(fixture(nodes=3), PROBE)

    def test_duplicate_writer(self):
        rows = fixture()
        rows.insert(3, copy.deepcopy(rows[2]))
        with self.assertRaises(ValueError): audit(rows, PROBE)

    def test_orphan_event(self):
        with self.assertRaises(ValueError): audit(fixture()[1:], PROBE)

    def test_schema(self):
        rows = fixture(); rows[0]["schema"] = "other"
        with self.assertRaises(ValueError): audit(rows, PROBE)

    def test_missing_post_loop(self):
        rows = fixture(); rows.pop(-2)
        with self.assertRaises(ValueError): audit(rows, PROBE)

    def test_misplaced_flush(self):
        rows = fixture(); rows[2]["event"] = "terminal_flush"
        with self.assertRaises(ValueError): audit(rows, PROBE)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--probe", type=Path, required=True)
    args, rest = parser.parse_known_args()
    PROBE = args.probe
    unittest.main(argv=[__file__, *rest])
