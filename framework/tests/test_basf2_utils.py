#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import unittest
import unittest.mock
import basf2.utils as ut
import os


class TestBasf2Utils(unittest.TestCase):
    """Test the class to bsaf2.utils module"""

    def test_pretty_print_module(self):
        """Check that pretty_print_module finishes without an error"""
        os.environ.update({'PAGER': "cat"})
        ut.pretty_print_module(ut, "Test")
        os.environ.pop("PAGER")


if __name__ == "__main__":
    # test everything
    unittest.main()
