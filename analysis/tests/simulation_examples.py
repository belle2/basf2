#!/usr/bin/env python3
# -- analysis/tests/simulation_examples.py

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the simulation examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_simulation_examples(self):
        """
        Test supported simulation examples.
        """
        b2tu.skip_test_if_light(self)  # simulation doesn't work in light releaes

        # list of the broken examples (to be removed when they are individually fixed)
        broken_sim_egs = ['B2A104-SimulateAndReconstruct-withBeamBkg.py'
                          ]

        self._test_examples_dir('analysis/examples/simulations/', broken_sim_egs)


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
