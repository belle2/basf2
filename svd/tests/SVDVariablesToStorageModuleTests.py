#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
import b2test_utils
import b2test_utils_analysis
import modularAnalysis as ma
from basf2 import create_path, set_random_seed


class SVDVariablesToStorageModuleTests(unittest.TestCase):
    """The unit test for SVDVariablesToStorageModule"""

    def test(self):
        """ Test of SVDVariablesToStorageModule """

        set_random_seed("something important")

        b2test_utils.configure_logging_for_tests()

        main = create_path()

        input_file = b2test_utils.require_file('SVDClusters.root', 'validation')
        ma.inputMdst(input_file, path=main)

        ma.fillParticleList("pi+:all", "", path=main)

        variablesToNtuple = ["SVDClusterCharge", "SVDClusterSNR", "SVDClusterSize", "SVDClusterTime",
                             "SVDTrackPrime", "SVDResidual", "SVDLayer", "SVDLadder", "SVDSensor", "SVDSide"]

        main.add_module("SVDVariablesToStorage",
                        outputFileName="SVDVariablesToStorageModuleTests.root",
                        containerName="SVDClusterVariables",
                        particleListName="pi+:all",
                        variablesToNtuple=variablesToNtuple)
        b2test_utils.safe_process(main)
        b2test_utils_analysis.scanTTree("SVDVariablesToStorageModuleTests.root")
        print("Test passed. Cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
