#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
import unittest
import glob
import b2test_utils


def test_one_tutorial(tutorial_name):
    """Tests one tutorial. Will fail if the tutorial doesn't exist."""
    loc = basf2.find_file('analysis/examples/tutorials/' + tutorial_name)
    return os.system('basf2 ' + loc + ' -n 1')


class TutorialsTest(unittest.TestCase):
    """Test to run all tutorials. Will fail if no tutorial directory is found."""

    #: list of supported tutorials (everything in the tutorials directory)
    supported_tutorials = [x.split("/")[-1]
                           for x in glob.glob(basf2.find_file('analysis/examples/tutorials/') + "*.py")]

    #: list of the broken tutorials (to be removed when they are individually fixed)
    broken_tutorials = ['B2A701-ContinuumSuppression_Input.py',  # BII-4246
                        'B2A702-ContinuumSuppression_MVATrain.py',  # BII-4246
                        'B2A703-ContinuumSuppression_MVAExpert.py',  # BII-4246
                        'B2A503-ReadDecayHash.py',  # BII-4254
                        ]

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    @unittest.skipIf(not os.getenv('BELLE2_VALIDATION_DATA_DIR'),
                     "$BELLE2_VALIDATION_DATA_DIR not found.")
    def test_tutorials(self):
        """
        Test supported tutorials.
        """
        for t in self.supported_tutorials:
            with self.subTest(t):
                if t in self.broken_tutorials:
                    pass
                else:
                    self.assertEqual(test_one_tutorial(t), 0)


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
