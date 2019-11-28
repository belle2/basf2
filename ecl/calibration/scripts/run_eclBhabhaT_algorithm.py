#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018 Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Mikhail Remnev
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

# Usage: basf2 run_eclBhabhaT_algorithm.py [input_file]
# Glob expression can be passed as an input file.
# Example: basf2 run_eclBhabhaT_algorithm.py "exp_0003_run_*.root"

import os
import sys
import ROOT
from basf2 import *
from ROOT import Belle2
from glob import glob

env = Belle2.Environment.Instance()

# Setting just with steering file
combineRuns = True   # For whether to run over each run invidually or to combine them together in a single plot
# combineRuns = False ;   # For whether to run over each run invidually or
# to combine them together in a single plot    ---- for crates run
# sequentially - small amouonts log size too big


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

set_debug_level(35)
set_log_level(LogLevel.INFO)   # or LogLevel.DEBUG


exprun_vector = algo.getRunListFromAllData()


if (combineRuns):
    print("Combining all runs' histograms for a single calibration")
    baseName = "eclBhabhaTAlgorithm"
    basePathAndName = basePath + baseName
    print("path = ", basePathAndName)
    algo.debugFilenameBase = basePathAndName
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
        baseName = "eclBhabhaTAlgorithm"
        basePathAndName = basePath + baseName
        print("path = ", basePathAndName)
        algo.debugFilenameBase = basePathAndName
        alg_result = algo.execute(iov_to_execute, 0)
        print("Calibration success-result was", alg_result, "   (0=ok, 2=needs more data)")
        if (alg_result == 0):
            algo.commit()
        # if ( alg_result == 2 ):  # Not enough data
        #   runsWithoutEnoughData.append(exprun)


print("Summary of possible calibration completion status values:")
print("      c_OK             = Finished successfuly             = 0 in Python")
print("      c_Iterate,       = Needs iteration                  = 1 in Python")
print("      c_NotEnoughData, = Needs more data                  = 2 in Python")
print("      c_Failure,       = Failed                           = 3 in Python")
print("      c_Undefined      = Not yet known (before execution) = 4 in Python")
