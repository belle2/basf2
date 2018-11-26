#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from ROOT import Belle2
import unittest
import glob


def test_one_tutorial(tutorial_name):
    loc = Belle2.FileSystem.findFile('analysis/examples/tutorials/' + tutorial_name)
    return os.system('basf2 ' + loc + ' -n 1')


class TutorialsTest(unittest.TestCase):
    supported_tutorials = [x.split("/")[-1]
                           for x in glob.glob(Belle2.FileSystem.findFile('analysis/examples/tutorials/') + "*.py")]

    broken_tutorials = ['B2A701-ContinuumSuppression_Input.py',  # BII-4246
                        'B2A702-ContinuumSuppression_MVATrain.py',  # BII-4246
                        'B2A703-ContinuumSuppression_MVAExpert.py',  # BII-4246
                        'B2A801-FlavorTagger.py',  # BII-4247
                        'B2A503-ReadDecayHash.py',  # BII-4254
                        ]

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
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
    unittest.main()
