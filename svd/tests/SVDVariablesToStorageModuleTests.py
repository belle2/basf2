#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
import tempfile
import b2test_utils
import modularAnalysis as ma
from basf2 import create_path, find_file
from ROOT import TFile


class SVDVariablesToStorageModuleTests(unittest.TestCase):
    """The unit test for SVDVariablesToStorageModule"""

    def test(self):
        """ Test of SVDVariablesToStorageModule """

        testFile = tempfile.NamedTemporaryFile()

        main = create_path()

        input_file = find_file('svd/tests/SVDVariablesToStorage_test_data.root')
        ma.inputMdst(input_file, path=main)

        ma.fillParticleList("pi+:all", "", path=main)

        main.add_module("SVDVariablesToStorage",
                        outputFileName=testFile.name,
                        containerName="SVDClusters",
                        particleListName="pi+:all",
                        variablesToNtuple=["SVDClusterCharge", "SVDClusterSNR", "SVDClusterSize", "SVDClusterTime",
                                           "SVDTrackPrime", "SVDResidual", "SVDLayer", "SVDLadder", "SVDSensor", "SVDSide"])
        b2test_utils.safe_process(main)
        output_file = TFile(testFile.name)
        ntuple = output_file.Get("SVDClusters")
        self.assertFalse(ntuple.GetEntries() == 0, "Ntuple is empty")

        print("Test passed. Cleaning up.")


if __name__ == '__main__':
    with b2test_utils.clean_working_directory():
        unittest.main()
