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
from basf2 import find_file
from b2test_utils import clean_working_directory


class TestEmbedding(unittest.TestCase):
    """Test to run the embedding example."""

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    def testEmbedding(self):
        """
        Test the embedding example.
        """
        embedding_example = find_file('analysis/examples/embedding/embedding.py')

        result = subprocess.run(['basf2', embedding_example], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            # failure running tutorial so let's print the output
            # on stderr so it's not split from output of unittest
            # done like this since we don't want to decode/encode utf8
            sys.stdout.buffer.write(result.stdout)

        self.assertEqual(result.returncode, 0)


if __name__ == '__main__':
    with clean_working_directory():
        unittest.main()
