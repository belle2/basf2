#!/usr/bin/env python3
# -- analysis/tests/pidcalibration_examples.py

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the PID calibration examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_base_examples(self):
        """
        Test supported PID calibration examples.
        """

        self._test_examples_dir('analysis/examples/PIDCalibration/')


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
