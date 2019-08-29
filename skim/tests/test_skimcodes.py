#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from skim.registry import skim_registry
import skimExpertFunctions


class TestSkimCodes(unittest.TestCase):
    """Test case for skim codes"""

    def test_code_format(self):
        """check the codes are the correct format (8 digits)"""
        # https://confluence.desy.de/x/URdYBQ
        for code, _ in skim_registry:
            self.assertEqual(len(code), 8, "Incorrect length skim code")
            self.assertTrue(code.isdigit(), "Must consist of digits")

    def test_unique_codes(self):
        """check that there aren't two skims registered with the same code"""
        codes = [code for code, _ in skim_registry]
        self.assertEqual(len(codes), len(set(codes)), "Duplicated skim code")

    def test_unique_names(self):
        """check that there aren't two skims registered with the same name"""
        codes = [name for _, name in skim_registry]
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
