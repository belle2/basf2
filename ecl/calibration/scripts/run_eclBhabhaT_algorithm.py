#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Usage: basf2 run_eclBhabhaT_algorithm.py [input_file]
# Glob expression can be passed as an input file.
# Example: basf2 run_eclBhabhaT_algorithm.py "exp_0003_run_*.root"

import sys
import ROOT
import basf2 as b2
from ROOT import Belle2
from glob import glob

env = Belle2.Environment.Instance()

# Setting just with steering file

# For whether to execute the algorithm over each run invidually or to combine them together in a single plot
#   often it is easier just to only pass a single file to the algorithm code instead of to use this feature
combineRuns = True


# Set up and execute calibration
algo = Belle2.ECL.eclBhabhaTAlgorithm()


print("Python arguments:")
counting = 0
for arg in sys.argv:
    print(counting, arg)
    counting = counting + 1

fileNames = ['eclBhabhaTCollector.root']
narg = len(sys.argv)
if(narg >= 2):
    fileNames = glob(sys.argv[1])
algo.setInputFileNames(fileNames)

basePath = ""
if(narg >= 3):
    basePath = sys.argv[2] + "/"
    print("basePath = ", basePath)


algo.cellIDLo = 3
algo.cellIDHi = 2
algo.debugOutput = True
algo.meanCleanRebinFactor = 3   # Rebin factor
# 0 means no cut.  # 1 means keep only bins from max rebinned bin.  # Note that with low stats, 0.2 is still too large.
algo.meanCleanCutMinFactor = 0.3
# algo.crateIDLo = 10
# algo.crateIDHi = 9
algo.debugFilenameBase = "eclBhabhaTAlgorithm"

b2.set_debug_level(35)
b2.set_log_level(b2.LogLevel.INFO)   # LogLevel.INFO or LogLevel.DEBUG


exprun_vector = algo.getRunListFromAllData()

baseName = "eclBhabhaTAlgorithm"
basePathAndName = basePath + baseName
algo.debugFilenameBase = basePathAndName


reset_database()
use_database_chain()
# use_central_database("data_reprocessing_proc10")
use_central_database("data_reprocessing_proc9")
use_local_database("localdb/database.txt")


if (combineRuns):
    print("Combining all runs' histograms for a single calibration")
    print("path = ", basePathAndName)
    alg_result = algo.execute()
    print("Calibration completion status", alg_result)
    if (alg_result == 0):
        algo.commit()
else:
    print("Calibrating each run individually")
    runsWithoutEnoughData = []
    for exprun in exprun_vector:
        iov_to_execute = ROOT.vector("std::pair<int,int>")()
        iov_to_execute.push_back(exprun)
        print("path = ", basePathAndName)
        alg_result = algo.execute(iov_to_execute, 0)
        print("Calibration success-result was", alg_result, "   (0=ok, 2=needs more data)")
        if (alg_result == 0):
            algo.commit()


print("Summary of possible calibration completion status values:")
print("      c_OK             = Finished successfuly             = 0 in Python")
print("      c_Iterate,       = Needs iteration                  = 1 in Python")
print("      c_NotEnoughData, = Needs more data                  = 2 in Python")
print("      c_Failure,       = Failed                           = 3 in Python")
print("      c_Undefined      = Not yet known (before execution) = 4 in Python")
