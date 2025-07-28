#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from b2test_utils import clean_working_directory
import b2test_utils_analysis as b2tua


class TutorialsTest(b2tua.ExamplesTest):
    """Test to run all B2A7XX tutorials."""
    def test_tutorials(self):
        """
        Test supported tutorials.
        """

        self._test_examples_dir('analysis/examples/tutorials/', filepattern="B2A7", nevents=100)


if __name__ == '__main__':
    with clean_working_directory():
        b2tua.unittest.main()
