#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import Belle2
from caf.utils import runs_from_vector, find_run_lists_from_boundaries

algo = Belle2.TestCalibrationAlgorithm()

# Can use a Python list of input files/wildcards. It will resolve the existing files
inputFileNames = ["CollectorOutput.root"]
algo.setInputFileNames(inputFileNames)
all_runs = algo.getRunListFromAllData()
boundaries = runs_from_vector(algo.findPayloadBoundaries(all_runs))

print("Boundaries were:")
for exprun in boundaries:
    print(exprun)

print("Run lists associated to boundary IoVs")

for iov, runs in find_run_lists_from_boundaries(boundaries, runs_from_vector(all_runs)).items():
    print(iov, runs)
