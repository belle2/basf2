#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Test the classes to handle iovs

import math
import random
import unittest
from conditions_db.iov import IntervalOfValidity, IoVSet


class TestIntervalOfValidity(unittest.TestCase):
    def iovify(self, iov):
        """Helper function to turn a tuple/None/list(tuple) into iov instances"""
        if isinstance(iov, list):
            return tuple(IntervalOfValidity(i) for i in iov)
        elif iov is not None:
            return IntervalOfValidity(iov)
        else:
            return None

    def test_create(self):
        """Test that we get errors when creating invalid iovs"""
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(0)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(-1, 0, 0, 0)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(0, -1, 0, 0)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(-1, -1, 0, 0)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(1, 0, 0, 0)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(0, 1, 0, 0)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(10, 0, 7, 4)
        with self.assertRaises(ValueError):
            iov = IntervalOfValidity(0, 0, -1, 4)
        iov = IntervalOfValidity(0, 0, -1, -1)
        self.assertEqual(iov.final, (math.inf, math.inf))
        iov = IntervalOfValidity(0, 0, 1, -1)
        self.assertEqual(iov.final, (1, math.inf))
        self.assertEqual(IntervalOfValidity(0, 1, 2, 3), IntervalOfValidity((0, 1, 2, 3)))

    def test_intersection(self):
        """Test that we can intersect iovs"""
        iov = IntervalOfValidity
        for a, b, c in [
            [(0, 0, 10, 0), (11, 10, 11, 12), None],
            [(0, 0, 10, 0), (8, 0, 10, 12), (8, 0, 10, 0)],
            [(0, 0, 0, 0), (0, 0, -1, -1), (0, 0, 0, 0)],
            [(0, 0, -1, -1), (20, 12, 21, 234), (20, 12, 21, 234)],
            [(0, 0, 2, -1), (1, 1, 2, 8), (1, 1, 2, 8)],
            [(1, 2, 2, -1), (0, 0, 2, 8), (1, 2, 2, 8)],
            [(1, 0, 3, -1), (0, 0, 2, -1), (1, 0, 2, -1)],
            [(0, 0, 0, 0), (0, 0, 0, 0), (0, 0, 0, 0)],
        ]:
            self.assertEqual(iov(a) & iov(b), self.iovify(c), f'{a} & {b} = {c}')
            self.assertEqual(iov(b) & iov(a), self.iovify(c), f'{a} & {b} = {c}')

    def test_union(self):
        """Test that we can calculate the union of iovs"""
        iov = IntervalOfValidity
        for a, b, c in [
            [(0, 0, 10, 0), (11, 10, 11, 12), None],
            [(0, 0, 10, 0), (8, 0, 10, 12), (0, 0, 10, 12)],
            [(0, 0, 0, 0), (0, 0, -1, -1), (0, 0, -1, -1)],
            [(0, 0, -1, -1), (20, 12, 21, 234), (0, 0, -1, -1)],
            [(0, 0, 2, -1), (1, 1, 2, 8), (0, 0, 2, -1)],
            [(1, 2, 2, -1), (0, 0, 2, 8), (0, 0, 2, -1)],
            [(1, 0, 3, -1), (0, 0, 2, -1), (0, 0, 3, -1)],
            [(0, 0, 0, 0), (0, 0, 0, 0), (0, 0, 0, 0)],
            [(0, 0, 1, 1), (1, 2, 1, 3), (0, 0, 1, 3)],
            [(0, 0, 1, 1), (1, 3, 1, 4), None],
            [(0, 0, 1, -1), (2, 0, 2, 1), (0, 0, 2, 1)],
            [(0, 0, 1, -1), (2, 0, 2, 1), (0, 0, 2, 1)],
            [(0, 0, 1, -1), (2, 1, 2, 1), None],
            [(0, 0, 1, -1), (1, 9999, 2, 1), (0, 0, 2, 1)],
        ]:
            self.assertEqual(iov(a) | iov(b), self.iovify(c), f'{a} | {b} = {c}')
            self.assertEqual(iov(b) | iov(a), self.iovify(c), f'{a} | {b} = {c}')

    def test_union_startone(self):
        """Test that we can caluclate the union starting at run 1 as well"""
        iov = IntervalOfValidity
        self.assertEqual(iov(0, 0, 1, -1).union(iov(2, 1, 2, -1)), None)
        self.assertEqual(iov(0, 0, 1, -1).union(iov(2, 1, 2, -1), True), iov(0, 0, 2, -1))

    def test_subtract(self):
        """Test subtracting iovs from another"""
        iov = IntervalOfValidity
        for a, b, c, d in [
            [(0, 0, 10, 0), (11, 10, 11, 12), (0, 0, 10, 0), (11, 10, 11, 12)],
            [(0, 0, 10, 0), (8, 0, 10, 12), (0, 0, 7, -1), (10, 1, 10, 12)],
            [(0, 0, 0, 0), (0, 0, -1, -1), None, (0, 1, -1, -1)],
            [(0, 0, -1, -1), (20, 12, 21, 234), [(0, 0, 20, 11), (21, 235, -1, -1)], None],
            [(0, 0, 2, -1), (1, 1, 2, 8), [(0, 0, 1, 0), (2, 9, 2, -1)], None],
            [(1, 2, 2, -1), (0, 0, 2, 8), (2, 9, 2, -1), (0, 0, 1, 1)],
            [(1, 0, 3, -1), (0, 0, 2, -1), (3, 0, 3, -1), (0, 0, 0, -1)],
            [(0, 0, -1, -1), (0, 0, -1, -1), None, None],
            [(0, 0, 1, -1), (0, 0, 1, -1), None, None],
            [(0, 0, 0, 0), (0, 0, 0, 0), None, None],
            [(0, 0, 1, -1), (2, 0, 2, -1), (0, 0, 1, -1), (2, 0, 2, -1)]
        ]:
            self.assertEqual(iov(a) - iov(b), self.iovify(c), f'{a} - {b} = {c}')
            self.assertEqual(iov(b) - iov(a), self.iovify(d), f'{b} - {a} = {d}')


class TestIoVSet(unittest.TestCase):
    def test_add(self):
        """Test adding iovs to a set"""
        inputs = [
            [(0, 0, 0, -1), (1, 0, 1, -1), (2, 1, 2, -1), (3, 1, 3, 1), (3, 2, 3, 2), (3, 3, 3, 6), (3, 8, 3, 8)],
            [(0, i, 0, i) for i in range(0, 100)],
            [(0, i, 0, i) for i in range(0, 100, 2)],
        ]
        results = [
            [(0, 0, 1, -1), (2, 1, 2, -1), (3, 1, 3, 6), (3, 8, 3, 8)],
            [(0, 0, 0, 99)],
            [(0, i, 0, i) for i in range(0, 100, 2)],
        ]

        for _ in range(10):
            for input, output in zip(inputs, results):
                s = IoVSet()
                for iov in random.sample(input, len(input)):
                    s.add(iov)
                output = {IntervalOfValidity(e) for e in output}
                self.assertEqual(s.iovs, output)

    def test_remove(self):
        """Test removing iovs from a set"""
        s = IoVSet()
        s.add((0, 0, 0, 100))
        to_remove = [(0, i, 0, i) for i in range(0, 100, 2)]
        result = {IntervalOfValidity(0, i, 0, i) for i in range(1, 99, 2)}
        result.add(IntervalOfValidity(0, 99, 0, 100))
        for _ in range(10):
            for iov in random.sample(to_remove, len(to_remove)):
                s.remove(iov)
            self.assertEqual(s.iovs, result)

if __name__ == "__main__":
    unittest.main()
