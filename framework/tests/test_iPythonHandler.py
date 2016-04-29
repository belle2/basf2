#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from ipython_tools import handler
import basf2

# @cond internal_test


class TestIPythonHandler(unittest.TestCase):

    def create_path(self):
        path = basf2.create_path()
        info_setter = basf2.register_module("EventInfoSetter")
        info_setter.param("evtNumList", [100])
        path.add_module(info_setter)
        path.add_module("Gearbox")
        path.add_module("PrintCollections")

        return path

    def test_process_no_start(self):
        path = self.create_path()

        calculation = handler.process(path)
        # status should be set correctly
        self.assertFalse(calculation.is_running())
        self.assertRaises(AssertionError, calculation.has_failed)
        self.assertFalse(calculation.is_finished())
        self.assertEqual(calculation.get_status(), "not started")

        # All the output should lead to nothing
        self.assertRaises(AssertionError, calculation.get_statistics)
        self.assertRaises(AssertionError, calculation.get_log)

        self.assertEqual(len(calculation.get_keys()), 0)

        # The modules
        modules = calculation.get_modules()
        self.assertEqual(len(modules), 6)

    def test_process_with_start(self):
        path = self.create_path()

        calculation = handler.process(path)
        calculation.start()

        self.assertTrue(calculation.is_running())
        self.assertEqual(calculation.get_status(), "running")

        calculation.wait_for_end(display_bar=False)

        self.assertFalse(calculation.is_running())
        self.assertEqual(calculation.get_status(), "finished")

        self.assertFalse(calculation.has_failed())
        self.assertTrue(calculation.is_finished())

        # All the output should lead to something
        self.assertGreater(len(str(calculation.get_statistics())), 0)
        self.assertGreater(len(calculation.get_log()), 0)

        self.assertEqual(len(calculation.get_keys()), 2)

        self.assertEqual(calculation.get("basf2.statistics"), calculation.get_statistics())
        sc = [{'number': i, 'store_content': [['EventMetaData', 0]]} for i in range(0, 100, 10)]
        self.assertEqual(calculation.get("basf2.store_content"), sc)

        # The modules
        modules = calculation.get_modules()
        self.assertEqual(len(modules), 6)

if __name__ == "__main__":
    unittest.main()

# @endcond
