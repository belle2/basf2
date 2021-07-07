#!/usr/bin/env python3
# -- analysis/tests/mva_examples.py

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the mva examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_mva_examples(self):
        """
        Test supported mva examples.
        """
        # list of examples that cannot be tested
        broken_mva_egs = ['B2A712-DeepContinuumSuppression_MVATrain.py',  # training takes too long and cannot be shortened
                          'B2A713-DeepContinuumSuppression_MVAExpert.py',  # requires weight file created in previous example
                          'B2A714-DeepContinuumSuppression_MVAModel.py'  # MVA model description and not standalone example script
                          ]

        self._test_examples_dir('analysis/examples/mva/', broken_mva_egs)


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
