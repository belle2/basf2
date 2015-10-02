#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest

from ipython_tools.calculation import Basf2CalculationList
import basf2

# @cond internal_test


class TestBasf2CalculationList(unittest.TestCase):

    def path_creator_no_queue(self, value1, value2):
        return value1, value2

    def path_creator_queue(self, value1, value2, queue):
        return value1, value2, queue

    def test_create_all_paths_with_queue(self):
        lists = dict(value1=[1, 2, 3], value2=["a", "b", "c"])
        calculation_list = Basf2CalculationList(self.path_creator_queue, lists)

        results, queues, parameter_combination = calculation_list.create_all_paths()

        self.assertEqual(len(queues), len(results))
        self.assertEqual(len(queues), 9)

        self.assertEqual(results[0][0], 1)
        self.assertEqual(results[0][1], "a")
        self.assertEqual(results[0][2], queues[0])

        self.assertEqual(results[8][0], 3)
        self.assertEqual(results[8][1], "c")
        self.assertEqual(results[8][2], queues[8])

    def test_create_all_paths_without_queue(self):
        lists = dict(value1=[1, 2, 3], value2=["a", "b", "c"])
        calculation_list = Basf2CalculationList(self.path_creator_no_queue, lists)

        results, queues, parameter_combination = calculation_list.create_all_paths()

        self.assertEqual(len(queues), len(results))
        self.assertEqual(len(queues), 9)

        self.assertEqual(results[0][0], 1)
        self.assertEqual(results[0][1], "a")

        self.assertEqual(results[8][0], 3)
        self.assertEqual(results[8][1], "c")


if __name__ == "__main__":
    unittest.main()

# @endcond
