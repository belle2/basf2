#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test all the steering files used in the online_book lessons.
Proudly based on analysis/test/examples.py.
"""

# std
import unittest
import subprocess
from pathlib import Path

# basf2
from b2test_utils import clean_working_directory


this_dir = Path(__file__).resolve().parent
statistics_script = this_dir.parent / "online_book_statistics.py"


class Test(unittest.TestCase):
    def test(self):
        subprocess.call(["python3", str(statistics_script.resolve())])


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main()
