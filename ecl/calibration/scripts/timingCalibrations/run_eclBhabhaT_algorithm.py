#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2019 Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Ewan Hill
#
# This software is provided "as is" without any warranty.
#
# This script executes the bhabha timing calibration algorithm,
# which is used for both the crystal time calibration and the
# crate time calibration.  It has to run on the results of
# the collector code.
# This script is run directly with basf2.
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
import array as arr

from basf2 import conditions as b2conditions

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

# Don't need to use this unless I want to change the reference crystals
# algo.refCrysPerCrate = arr.array('i', [-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
#                                        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
#                                        -1,-1,-1])   # 'i' is a signed integer

# Below is equivalent to the first payload version
# algo.refCrysPerCrate = arr.array('i', [2306, 2309, 2313, 2317, 2321,
# 2326, 2329, 2334, 2337, 2343, 2348, 2349, 2356, 2357, 2361, 2365, 2372,
# 2373, 2377, 2381, 2388, 2391, 2393, 2399, 2401, 2407, 2409, 2413, 2417,
# 2421, 2426, 2429, 2433, 2440, 2585, 2446, 671, 583, 595, 607, 619, 631,
# 643, 655, 8252, 8177, 8185, 8192, 8206, 8224, 8228, 8244])
# 'i' is a signed integer

set_debug_level(35)
set_log_level(LogLevel.INFO)   # LogLevel.INFO or LogLevel.DEBUG


exprun_vector = algo.getRunListFromAllData()

baseName = "eclBhabhaTAlgorithm"
basePathAndName = basePath + baseName
algo.debugFilenameBase = basePathAndName


# == Configure database
b2conditions.reset()
b2conditions.override_globaltags()

B2INFO("Adding Local Database {} to head of chain of local databases.")
b2conditions.prepend_testing_payloads("localdb/database.txt")
B2INFO("Using Global Tag {}")
b2conditions.prepend_globaltag("ECL_testingNewPayload_RefCrystalPerCrate")
b2conditions.prepend_globaltag("master_2020-05-13")
b2conditions.prepend_globaltag("online_proc11")
b2conditions.prepend_globaltag("data_reprocessing_proc11")
b2conditions.prepend_globaltag("Reco_master_patch_rel5")


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
