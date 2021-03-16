#!/usr/bin/env python3
# pylama:ignore=W0212
import unittest
import re
from unittest.mock import create_autospec

import conditions_db
from conditions_db.runningupdate import RunningTagUpdater, RunningTagUpdaterError, RunningTagUpdateMode as Mode
from conditions_db.cli_management import command_tag_runningupdate


class TestMergeStaging(unittest.TestCase):
    """Test the class to calculate updates to running tags"""
    #: Basic definition of a running tag to be used later
    RUNNING_BASE = {
        # gapless payload with different revisions
        "A": [(0, 0, 0, -1), (1, 0, 1, 10), (1, 11, 1, 11), (1, 12, -1, -1)],
        # simple open payload
        "B": [(0, 0, -1, -1)],
        # simple closed payload
        "C": [(0, 0, 0, -1)],
    }

    def make_payloads(self, start_revision=1, **payloads):
        """Create a list of payloads from all keyword arguments Name of the
        payloads will be the name of the argument, revision will increase
        monotonic starting at start_revision for each argument and the iovs are
        the values of the the argument.

        To create a list with two payloads, A and B with two validities for B
        and one for A one could use::

            make_payloads(A=[(0,0,1,2)], B=[(1,2,3,4), (3,5,6,7)])
        """
        result = []
        for name, iovs in payloads.items():
            for rev, iov in enumerate(iovs, start_revision):
                result.append(conditions_db.PayloadInformation(0, name, rev, None, None, None, None, iov))
        return result

    def create_payloads_from_text(self, text):
        """
        Create payloads from a plain text list of payloads in the form of

        payloadN, rev i, valid from a,b to c,d

        Return a list of lists of payloads where each block of payload definitions
        without empty lines in between is returned as one list.
        """
        payloads = []
        all_payloads = []
        for match in re.findall(r'^\s*(payload\d+), rev (\d+), valid from (\d+),(\d+) to ([0-9-]+),([0-9-]+)$|^\s*$', text, re.M):
            if not match[0]:
                if payloads:
                    all_payloads.append(payloads)
                    payloads = []
                continue
            payloads.append(conditions_db.PayloadInformation(
                0, match[0], int(match[1]), None, None, None, None, tuple(map(int, match[2:]))
            ))

        if payloads:
            all_payloads.append(payloads)

        return all_payloads

    def make_mock_db(self, running_state="RUNNING", staging_state="VALIDATED", running_payloads=None, staging_payloads=None):
        """Create a mock object that behaves close enough to the ConditionsDB class but returns
        fixed values for the globaltags "running" and "staging" for testing"""
        db = create_autospec(conditions_db.ConditionsDB)

        if running_payloads is None:
            running_payloads = []
        if staging_payloads is None:
            staging_payloads = []

        def taginfo(name):
            return {
                "running": {"name": "running", "globalTagStatus": {"name": running_state}},
                "staging": {"name": "staging", "globalTagStatus": {"name": staging_state}},
            }.get(name, None)

        def all_iovs(name):
            return {
                "running": running_payloads,
                "staging": staging_payloads}.get(name, None)

        db.get_globalTagInfo.side_effect = taginfo
        db.get_all_iovs.side_effect = all_iovs
        return db

    def parse_operations(self, operations):
        """Parse the operations into a single list of [operation type, name,
        revision, iov] for each operation"""
        return [[op, p.name, p.revision, p.iov] for (op, p) in operations]

    def test_states(self):
        """Test that we get errors if the tags don't exist or are in the wrong state"""
        db0 = self.make_mock_db()
        db1 = self.make_mock_db(running_state="PUBLISHED")
        db2 = self.make_mock_db(staging_state="OPEN")
        db3 = self.make_mock_db(running_state="INVALID", staging_state="PUBLISHED")

        with self.assertRaisesRegex(RunningTagUpdaterError, "not in RUNNING state"):
            RunningTagUpdater(db1, "running", "staging", (0, 0), Mode.ALLOW_CLOSED)
        with self.assertRaisesRegex(RunningTagUpdaterError, "not in VALIDATED state"):
            RunningTagUpdater(db2, "running", "staging", (0, 0), Mode.ALLOW_CLOSED)
        with self.assertRaisesRegex(RunningTagUpdaterError, "not in RUNNING state"):
            RunningTagUpdater(db3, "running", "staging", (0, 0), Mode.ALLOW_CLOSED)
        with self.assertRaisesRegex(RunningTagUpdaterError, "'funning' cannot be found"):
            RunningTagUpdater(db0, "funning", "staging", (0, 0), Mode.ALLOW_CLOSED)
        with self.assertRaisesRegex(RunningTagUpdaterError, "'caging' cannot be found"):
            RunningTagUpdater(db0, "running", "caging", (0, 0), Mode.ALLOW_CLOSED)
        with self.assertRaisesRegex(RunningTagUpdaterError, "'cunning' cannot be found"):
            RunningTagUpdater(db0, "cunning", "stacking", (0, 0), Mode.ALLOW_CLOSED)

    def test_overlaps(self):
        """Test that we get errors on overlaps in the staging"""
        db = self.make_mock_db()
        updater = RunningTagUpdater(db, "running", "staging", (0, 0), Mode.ALLOW_CLOSED)
        # no overlap between different payload names
        payloads = self.make_payloads(A=[(0, 0, 1, 0)], B=[(0, 1, 1, 0)])
        self.assertIsNone(updater._check_staging_tag("testoverlaps", payloads))

        # but otherwise we raise errors
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (0, 1, 1, 0)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testoverlaps", payloads)
        self.assertEqual(ctx.exception.extra_vars['overlaps'], 1)

        # yes, also a tiny overlap is an overlap
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 0, 1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testoverlaps", payloads)
        self.assertEqual(ctx.exception.extra_vars['overlaps'], 1)

        # and we even raise multiple errors on multiple overlaps
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 0, 1, -1), (1, 1, 1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testoverlaps", payloads)
        self.assertEqual(ctx.exception.extra_vars['overlaps'], 2)

        # and we even raise multiple errors on multiple overlaps ...
        # also if the overlaps are not adjacent
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (3, 0, -1, -1), (1, 0, 1, -1), (2, 0, 2, -1), (1, 1, 1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testoverlaps", payloads)
        self.assertEqual(ctx.exception.extra_vars['overlaps'], 2)

        # unless there's no overlap
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 1, 1, -1)])
        self.assertIsNone(updater._check_staging_tag("testoverlaps", payloads))

    def test_gaps(self):
        """Test that we get errors for gaps in the staging"""
        db = self.make_mock_db()
        updater = RunningTagUpdater(db, "running", "staging", (0, 0), Mode.ALLOW_CLOSED)

        # consecutive payloads should be merged
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 1, -1, -1)])
        self.assertIsNone(updater._check_staging_tag("testgaps", payloads))

        # and otherwise we want errors
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 2, 1, -1), (2, 2, -1, -1)], B=[(0, 0, 1, 2), (2, 1, 3, 3)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testgaps", payloads)
        self.assertEqual(ctx.exception.extra_vars['gaps'], 3)

    def test_closed(self):
        """Test that we get errors on iovs not being open in the staging"""
        db = self.make_mock_db()
        updater = RunningTagUpdater(db, "running", "staging", (0, 0), Mode.STRICT)

        # consecutive payloads should be merged but there could be gaps
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 1, 1, -1)], B=[(0, 0, 1, 2), (1, 3, 3, 3)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testclosed", payloads)
        self.assertEqual(ctx.exception.extra_vars['closed payloads'], 2)

        # we can fix one gap and still get an error
        payloads = self.make_payloads(A=[(0, 0, 1, -1), (2, 0, -1, -1)], B=[(0, 0, 1, 2), (1, 3, 3, 3)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testclosed", payloads)
        self.assertEqual(ctx.exception.extra_vars['closed payloads'], 1)

        # or have a different order ...
        payloads = self.make_payloads(A=[(0, 0, 1, -1), (2, 0, 99999, 999999)], B=[(10, 0, -1, -1), (0, 0, 9, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testclosed", payloads)
        self.assertEqual(ctx.exception.extra_vars['closed payloads'], 1)

    def test_start_early(self):
        """Test that we get errors if the payloads in the staging tag start too early"""
        db = self.make_mock_db()
        updater = RunningTagUpdater(db, "running", "staging", (2, 5), Mode.ALLOW_CLOSED)

        # simple start early
        payloads = self.make_payloads(A=[(2, 0, -1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testearly", payloads)
        self.assertEqual(ctx.exception.extra_vars['starts too early'], 1)

        # but starting at 0,0 is an exception and is fine
        payloads = self.make_payloads(A=[(0, 0, -1, -1)])
        updater._check_staging_tag("testearly", payloads)

        # overlap and gap free and starting at 0,0 ... but still wrong because
        # the next payload starts early
        payloads = self.make_payloads(A=[(0, 0, 1, 0), (1, 1, 1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testearly", payloads)
        self.assertEqual(ctx.exception.extra_vars['starts too early'], 1)

        payloads = self.make_payloads(A=[(1, 1, 2, 4)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testearly", payloads)
        self.assertEqual(ctx.exception.extra_vars['starts too early'], 1)

        # but allow multiple payloads after the starting point
        payloads = self.make_payloads(A=[(0, 0, 2, 5), (2, 6, 2, 6), (2, 7, 2, -1), (3, 0, -1, -1)])
        updater._check_staging_tag("testearly", payloads)

        # check order dependence
        payloads = self.make_payloads(A=[(3, 0, -1, -1), (2, 4, 2, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag("testearly", payloads)
        self.assertEqual(ctx.exception.extra_vars['starts too early'], 1)

    def test_simple_mode(self):
        """Test that we only allow trivial iovs in simple mode"""
        db = self.make_mock_db()
        updater = RunningTagUpdater(db, "running", "staging", (2, 5), Mode.SIMPLE)

        # One payload only but not the correct one
        payloads = self.make_payloads(A=[(1, 0, -1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag_simple("testsimple", payloads)
        self.assertEqual(ctx.exception.extra_vars['wrong validity'], 1)

        payloads = self.make_payloads(A=[(0, 1, -1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag_simple("testsimple", payloads)
        self.assertEqual(ctx.exception.extra_vars['wrong validity'], 1)

        payloads = self.make_payloads(A=[(0, 0, 1, -1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag_simple("testsimple", payloads)
        self.assertEqual(ctx.exception.extra_vars['wrong validity'], 1)

        payloads = self.make_payloads(A=[(0, 0, 1, 1)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag_simple("testsimple", payloads)
        self.assertEqual(ctx.exception.extra_vars['wrong validity'], 1)

        # but 0,0,-1,-1 is fine
        payloads = self.make_payloads(A=[(0, 0, -1, -1)], B=[(0, 0, -1, -1)])
        updater._check_staging_tag_simple("testsimple", payloads)

        # but only one per payload
        payloads = self.make_payloads(A=[(0, 0, -1, -1), (0, 0, -1, -1), (1, 2, 3, 4)],
                                      B=[(0, 0, -1, -1)], C=[(0, 1, 2, 3)])
        with self.assertRaises(RunningTagUpdaterError) as ctx:
            updater._check_staging_tag_simple("testsimple", payloads)
        self.assertEqual(ctx.exception.extra_vars['wrong validity'], 2)
        self.assertEqual(ctx.exception.extra_vars['duplicate payloads'], 2)

    def test_running_check(self):
        """Test that we get errors on iovs not being open in the staging"""
        db = self.make_mock_db()
        updater = RunningTagUpdater(db, "running", "staging", (2, 5), Mode.STRICT)

        # only check we do is that payloads end before the first valid run or are open
        payloads = self.make_payloads(A=[(0, 0, 1, -1), (2, 0, 2, 4)], B=[(0, 0, -1, -1)])
        updater._check_running_tag("testrunning", payloads)

        # so we want an error even if anything closes on the same run
        payloads = self.make_payloads(A=[(0, 0, 1, -1), (2, 0, 2, 5)], B=[(0, 0, -1, -1)], C=[(0, 0, 10, -1)])
        with self.assertRaisesRegex(RunningTagUpdaterError, "Given first valid run conflicts with running tag") as ctx:
            updater._check_running_tag("testrunning", payloads)
        self.assertEqual(ctx.exception.extra_vars['payloads end after first valid run'], 2)

        # also complain if it starts after the first valid run
        payloads = self.make_payloads(A=[(2, 5, -1, -1)], B=[(0, 0, 2, 1), (2, 2, -1, -1)], C=[(10, 0, -1, -1)])
        with self.assertRaisesRegex(RunningTagUpdaterError, "Given first valid run conflicts with running tag") as ctx:
            updater._check_running_tag("testrunning", payloads)
        self.assertEqual(ctx.exception.extra_vars['payloads start after first valid run'], 2)

    def test_operations_simple(self):
        """Test something useful."""
        running = self.make_payloads(**self.RUNNING_BASE)
        staging = self.make_payloads(
            A=[(0, 0, -1, -1)],
            B=[(0, 0, -1, -1)],
            C=[(0, 0, -1, -1)],
            start_revision=10)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (2, 0), Mode.STRICT)
        result = updater.calculate_update()
        self.assertEqual(self.parse_operations(result),
                         [
                             ['CLOSE',  'A',  4, (1, 12,  1, -1)],
                             ['CLOSE',  'B',  1, (0,  0,  1, -1)],
                             ['CREATE', 'A', 10, (2,  0, -1, -1)],
                             ['CREATE', 'B', 10, (2,  0, -1, -1)],
                             ['CREATE', 'C', 10, (2,  0, -1, -1)],
                         ])

    def test_operations_full(self):
        """
        In full mode we close B even if there's nothing in the staging tag
        to allow "removing" payloads from the set of valid ones
        """
        running = self.make_payloads(**self.RUNNING_BASE)
        staging = self.make_payloads(
            A=[(0, 0, -1, -1)],
            C=[(0, 0, -1, -1)],
            start_revision=10)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (2, 0), Mode.FULL_REPLACEMENT)
        result = updater.calculate_update()
        self.assertEqual(self.parse_operations(result),
                         [
                             ['CLOSE',  'A',  4, (1, 12,  1, -1)],
                             ['CLOSE',  'B',  1, (0,  0,  1, -1)],
                             ['CREATE', 'A', 10, (2,  0, -1, -1)],
                             ['CREATE', 'C', 10, (2,  0, -1, -1)],
                         ])

    def test_operations_ragged(self):
        """This time B doesn't start on the spot but a bit later. And A is a list of iovs to be preserved"""
        running = self.make_payloads(**self.RUNNING_BASE)
        staging = self.make_payloads(
            A=[(0, 0, 2, 5), (2, 6, 2, 10), (2, 11, -1, -1)],
            B=[(2, 8, -1, -1)],
            C=[(2, 0, -1, -1)],
            start_revision=10)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (2, 0), Mode.STRICT)
        result = updater.calculate_update()
        self.assertEqual(self.parse_operations(result),
                         [
                             ['CLOSE',  'A',  4, (1, 12,  1, -1)],
                             ['CLOSE',  'B',  1, (0,  0,  2,  7)],
                             ['CREATE', 'A', 10, (2,  0,  2,  5)],
                             ['CREATE', 'A', 11, (2,  6,  2, 10)],
                             ['CREATE', 'A', 12, (2, 11, -1, -1)],
                             ['CREATE', 'B', 10, (2,  8, -1, -1)],
                             ['CREATE', 'C', 10, (2,  0, -1, -1)],
                         ])

    def test_operations_extend(self):
        """If the staging payloads have same revision as the last one in running merge them"""
        running = self.make_payloads(**self.RUNNING_BASE)
        staging = self.make_payloads(
            A=[(0, 0, 2, 5), (2, 6, 2, 10), (2, 11, -1, -1)],
            B=[(2, 8, -1, -1)],
            C=[(2, 0, -1, -1)],
            start_revision=10)
        # we nee to have the same revision for merging though so fudge a bit
        staging[0].revision = 4
        staging[-2].revision = 1
        print([(e.name, e.revision, e.iov) for e in running])
        print([(e.name, e.revision, e.iov) for e in staging])
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (2, 0), Mode.STRICT)
        result = updater.calculate_update()
        self.assertEqual(self.parse_operations(result),
                         [
                             ['CLOSE',  'A',  4, (1, 12,  2, 5)],
                             ['CREATE', 'A', 11, (2,  6,  2, 10)],
                             ['CREATE', 'A', 12, (2, 11, -1, -1)],
                             ['CREATE', 'C', 10, (2,  0, -1, -1)],
                         ])

    def test_operations_fix_open(self):
        """Test automatic opening of the last iov if necessary"""
        running = self.make_payloads(**self.RUNNING_BASE)
        staging = self.make_payloads(
            A=[(0, 0, 2, 5), (2, 6, 2, 10), (2, 11, 2, -1)],
            B=[(2, 8, 2, 10), (2, 11, 2, 28)],
            C=[(2, 0, 2, 100)],
            start_revision=10)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (2, 0), Mode.FIX_CLOSED)
        result = updater.calculate_update()
        self.assertEqual(self.parse_operations(result),
                         [
                             ['CLOSE',  'A',  4, (1, 12,  1, -1)],
                             ['CLOSE',  'B',  1, (0,  0,  2,  7)],
                             ['CREATE', 'A', 10, (2,  0,  2,  5)],
                             ['CREATE', 'A', 11, (2,  6,  2, 10)],
                             ['CREATE', 'A', 12, (2, 11, -1, -1)],
                             ['CREATE', 'B', 10, (2,  8,  2, 10)],
                             ['CREATE', 'B', 11, (2, 11, -1, -1)],
                             ['CREATE', 'C', 10, (2,  0, -1, -1)],
                         ])

    def test_merge_overlapandgaps(self):
        """Test that merging fails if we have gaps or overlaps"""
        running = self.make_payloads(**self.RUNNING_BASE)
        staging = self.make_payloads(
            A=[(0, 0, 2, 5), (2, 5, 2, 10), (2, 11, -1, -1)],
            B=[(2, 8, 2, 10), (2, 12, -1, -1)],
            C=[(2, 0, -1, -1)],
            start_revision=10)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (2, 0), Mode.STRICT)
        with self.assertRaisesRegex(RunningTagUpdaterError, "tag 'staging' not fit for update") as ctx:
            updater.calculate_update()
        # check that we have one overlap and one gap using subset comparison logic
        self.assertLessEqual({'overlaps': 1, 'gaps': 1}.items(), ctx.exception.extra_vars.items())

    def test_doc_example(self):
        """Extract the example from the `b2conditionsdb tag runningupdate` docstring and run it"""
        running, staging, expected = self.create_payloads_from_text(command_tag_runningupdate.__doc__)
        # make a copy of the running payloads just to be able to compare later
        result = running[:]
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (1, 2), Mode.ALLOW_CLOSED)
        operations = updater.calculate_update()
        # check that the update does what we actually wrote in the doc. This will
        # fail if the docs are updated.
        self.assertEqual(self.parse_operations(operations),
                         [
                             ['CLOSE',  'payload1', 2, (1, 1, 1, 1)],
                             ['CLOSE',  'payload2', 1, (0, 1, 1, 4)],
                             ['CLOSE',  'payload4', 1, (0, 1, 1, 20)],
                             ['CREATE', 'payload1', 3, (1, 2, 1, 8)],
                             ['CREATE', 'payload1', 4, (1, 9, 1, 20)],
                             ['CREATE', 'payload2', 2, (1, 5, 1, 20)],
                             ['CREATE', 'payload3', 2, (1, 2, -1, -1)],
                         ])
        # and then apply the operations to the input list and make sure that is
        # what's written in the doc
        for op, payload in operations:
            if op == "CLOSE":
                # adjust the end of the iov
                i = result.index(payload)
                result[i].iov = result[i].iov[:2] + payload.iov[2:]
            else:
                # or just add the full payload
                result.append(payload)
        result.sort()
        self.assertEqual(expected, result)

    def test_doc_example_full(self):
        """Extract the example from the `b2conditionsdb tag runningupdate` docstring and run it"""
        running, staging, expected = self.create_payloads_from_text(command_tag_runningupdate.__doc__)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (1, 2), Mode.FULL_REPLACEMENT)
        operations = updater.calculate_update()
        # check that the update does what we actually wrote in the doc. This will
        # fail if the docs are updated.
        self.assertEqual(self.parse_operations(operations),
                         [
                             ['CLOSE',  'payload1', 2, (1, 1, 1, 1)],
                             ['CLOSE',  'payload2', 1, (0, 1, 1, 4)],
                             ['CLOSE',  'payload4', 1, (0, 1, 1, 20)],
                             ['CLOSE',  'payload5', 1, (0, 1, 1, 1)],
                             ['CREATE', 'payload1', 3, (1, 2, 1, 8)],
                             ['CREATE', 'payload1', 4, (1, 9, 1, 20)],
                             ['CREATE', 'payload2', 2, (1, 5, 1, 20)],
                             ['CREATE', 'payload3', 2, (1, 2, -1, -1)],
                         ])

    def test_doc_example_fixclosed(self):
        """Extract the example from the `b2conditionsdb tag runningupdate` docstring and run it"""
        running, staging, expected = self.create_payloads_from_text(command_tag_runningupdate.__doc__)
        db = self.make_mock_db(running_payloads=running, staging_payloads=staging)
        updater = RunningTagUpdater(db, "running", "staging", (1, 2), Mode.FIX_CLOSED)
        operations = updater.calculate_update()
        # check that the update does what we actually wrote in the doc. This will
        # fail if the docs are updated.
        self.assertEqual(self.parse_operations(operations),
                         [
                             ['CLOSE',  'payload1', 2, (1, 1, 1, 1)],
                             ['CLOSE',  'payload2', 1, (0, 1, 1, 4)],
                             ['CREATE', 'payload1', 3, (1, 2, 1, 8)],
                             ['CREATE', 'payload1', 4, (1, 9, -1, -1)],
                             ['CREATE', 'payload2', 2, (1, 5, -1, -1)],
                             ['CREATE', 'payload3', 2, (1, 2, -1, -1)],
                         ])


if __name__ == "__main__":
    # test everything
    unittest.main()
