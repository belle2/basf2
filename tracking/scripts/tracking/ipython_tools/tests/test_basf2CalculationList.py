from unittest import TestCase

from tracking.ipython_tools.calculation import Basf2CalculationList
import basf2


class TestBasf2CalculationList(TestCase):

    def path_creator_no_queue(self, value1, value2):
        return value1, value2

    def path_creator_queue(self, value1, value2, queue):
        return value1, value2, queue

    def test_create_all_paths_with_queue(self):
        calculation_list = Basf2CalculationList(self.path_creator_queue, [1, 2, 3], ["a", "b", "c"])

        results, queues = calculation_list.create_all_paths()

        self.assertEqual(len(queues), len(results))
        self.assertEqual(len(queues), 9)

        self.assertEqual(results[0][0], 1)
        self.assertEqual(results[0][1], "a")
        self.assertEqual(results[0][2], queues[0])

        self.assertEqual(results[8][0], 3)
        self.assertEqual(results[8][1], "c")
        self.assertEqual(results[8][2], queues[8])

    def test_create_all_paths_without_queue(self):
        calculation_list = Basf2CalculationList(self.path_creator_no_queue, [1, 2, 3], ["a", "b", "c"])

        results, queues = calculation_list.create_all_paths()

        self.assertEqual(len(queues), len(results))
        self.assertEqual(len(queues), 9)

        self.assertEqual(results[0][0], 1)
        self.assertEqual(results[0][1], "a")

        self.assertEqual(results[8][0], 3)
        self.assertEqual(results[8][1], "c")
