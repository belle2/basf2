#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to run over the KS selector examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_ksselector_examples(self):
        """
        Test supported KS selector examples.
        """

        self._test_examples_dir('analysis/examples/V0Selector/', filepattern="*Ks", nevents=20)


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
