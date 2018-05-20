#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import skimExpertFunctions


class TestSkimCodes(unittest.TestCase):
    """Test case for skim codes"""

    def test_unique_codes(self):
        """check that there aren't two skims registered with the same code"""
        codes = [pair[0] for pair in skimExpertFunctions._skimNameMatching]
        self.assertEqual(len(codes), len(set(codes)), "Duplicated skim code")

    def test_unique_names(self):
        """check that there aren't two skims registered with the same name"""
        names = [pair[1] for pair in skimExpertFunctions._skimNameMatching]
        self.assertEqual(len(names), len(set(names)), "Duplicated skim name")

    def test_encode(self):
        """check that we raise a LookupError if the skim name doesn't exist"""
        with self.assertRaises(LookupError):
            skimExpertFunctions.encodeSkimName('SomeNonExistentSkimName')

    def test_decode(self):
        """check that we raise a LookupError if the skim code doesn't exist"""
        with self.assertRaises(LookupError):
            skimExpertFunctions.decodeSkimName('1337')


if __name__ == '__main__':
    unittest.main()
