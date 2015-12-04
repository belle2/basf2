#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import metaoptions
import ROOT
import random
import math

call_iteration = 0


class ValidationTest(unittest.TestCase):

    """
    Various test cases for the validation module
    """

    def test_meta_option_parser(self):
        """
        Test if the meta options parsers behaves nice
        """
        p = metaoptions.MetaOptionParser(["expert", "pvalue-warn=0.9", "pvalue-error=0.4"])

        self.assertEqual(0.9, p.pvalue_warn())
        self.assertEqual(0.4, p.pvalue_error())
        self.assertTrue(p.has_option("expert"))
        self.assertFalse(p.has_option("not is list"))

        p = metaoptions.MetaOptionParser(["expert", "pvalue-warn="])

        self.assertEqual(None, p.pvalue_warn())
        self.assertEqual(None, p.pvalue_error())

if __name__ == "__main__":
    unittest.main()
