#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys
import subprocess
import unittest
import glob
from basf2 import find_file
from b2test_utils import clean_working_directory, configure_logging_for_tests
from b2test_utils_analysis import scanTTree


class TutorialsTest(unittest.TestCase):
    """Test to run all B2A7XX tutorials. Will fail if no tutorial directory is found."""

    #: list of the broken tutorials (to be removed when they are individually fixed)
    broken_tutorials = []

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    @unittest.skipIf(not os.getenv('BELLE2_VALIDATION_DATA_DIR'),
                     "$BELLE2_VALIDATION_DATA_DIR not found.")
    def test_tutorials(self):
        """
        Test supported tutorials.
        """
        configure_logging_for_tests()
        all_tutorials = sorted(glob.glob(find_file('analysis/examples/tutorials/') + "/B2A7*.py"))
        for tutorial in all_tutorials:
            filename = os.path.basename(tutorial)
            if filename not in self.broken_tutorials:
                with self.subTest(msg=filename):
                    outputfilename = filename.replace('.py', '.root')
                    result = subprocess.run(['basf2', '-n100', tutorial, '-o', outputfilename],
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                    if result.returncode != 0:
                        # failure running tutorial so let's print the output
                        # on stderr so it's not split from output of unittest
                        # done like this since we don't want to decode/encode utf8
                        sys.stdout.buffer.write(result.stdout)
                    self.assertEqual(result.returncode, 0)

                    if os.path.exists(outputfilename):
                        scanTTree(outputfilename)


if __name__ == '__main__':
    with clean_working_directory():
        unittest.main()
