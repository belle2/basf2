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


class V0SelectorTest(unittest.TestCase):
    """Test to run all scripts in V0Selector. Will fail if no the directory is found."""

    #: list of the broken V0Selectors (to be removed when they are individually fixed)
    broken_V0Selectors = ['TrainLambdaSelector.py']
    #: list of the V0Selectors whose output file name cannot be overwritten
    do_not_overwrite = ['CreateTrainData_Ks.py', 'CreateTrainData_Lambda.py']

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    @unittest.skipIf(not os.getenv('BELLE2_VALIDATION_DATA_DIR'),
                     "$BELLE2_VALIDATION_DATA_DIR not found.")
    def test_V0Selectors(self):
        """
        Test supported V0Selectors.
        """
        configure_logging_for_tests()
        all_V0Selectors = sorted(glob.glob(find_file('analysis/examples/V0Selector/') + "/*.py"))
        for V0Selector in all_V0Selectors:
            filename = os.path.basename(V0Selector)
            if filename not in self.broken_V0Selectors:
                with self.subTest(msg=filename):
                    outputfilename = filename.replace('.py', '.root')
                    if filename not in self.do_not_overwrite:
                        result = subprocess.run(['basf2', '-n100', V0Selector, '-o', outputfilename],
                                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                    else:
                        result = subprocess.run(['basf2', '-n100', V0Selector],
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
