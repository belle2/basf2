#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import unittest
import glob
from basf2 import find_file
from b2test_utils import clean_working_directory


class TutorialsTest(unittest.TestCase):
    """Test to run all tutorials. Will fail if no tutorial directory is found."""

    #: list of the broken tutorials (to be removed when they are individually fixed)
    broken_tutorials = ['B2A702-ContinuumSuppression_MVATrain.py',  # BII-4246
                        'B2A703-ContinuumSuppression_MVAExpert.py',  # BII-4246
                        ]

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    @unittest.skipIf(not os.getenv('BELLE2_VALIDATION_DATA_DIR'),
                     "$BELLE2_VALIDATION_DATA_DIR not found.")
    def test_tutorials(self):
        """
        Test supported tutorials.
        """
        all_tutorials = sorted(glob.glob(find_file('analysis/examples/tutorials/') + "/*.py"))
        for tutorial in all_tutorials:
            filename = os.path.basename(tutorial)
            if filename not in self.broken_tutorials:
                with self.subTest(msg=filename):
                    result = subprocess.run(['basf2', '-n1', tutorial], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                    if result.returncode != 0:
                        # failure running tutorial so let's print the output
                        # on stderr so it's not split from output of unittest
                        # done like this since we don't want to decode/encode utf8
                        sys.stdout.buffer.write(result.stdout)
                    self.assertEqual(result.returncode, 0)


if __name__ == '__main__':
    with clean_working_directory():
        unittest.main()
