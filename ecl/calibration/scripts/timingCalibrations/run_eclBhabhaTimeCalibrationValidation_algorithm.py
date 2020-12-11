#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2020 Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Ewan Hill
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

# Usage: basf2 run_eclBhabhaTimeCalibrationValidation_algorithm.py [input_file]
# Glob expression can be passed as an input file.
# Example: basf2 run_eclBhabhaTimeCalibrationValidation_algorithm.py "exp_0003_run_*.root"

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
algo = Belle2.ECL.eclTValidationAlgorithm("eclBhabhaTimeCalibrationValidationCollector")


print("Python arguments:")
counting = 0
for arg in sys.argv:
    print(counting, arg)
    counting = counting + 1

fileNames = ['eclBhabhaTimeCalibrationValidationCollector.root']
narg = len(sys.argv)
if(narg >= 2):
    fileNames = glob(sys.argv[1])
algo.setInputFileNames(fileNames)

basePath = ""
if(narg >= 3):
    basePath = sys.argv[2] + "/"
    print("basePath = ", basePath)


# algo.cellIDLo = 3
# algo.cellIDHi = 2
algo.meanCleanRebinFactor = 3   # Rebin factor
# 0 means no cut.  # 1 means keep only bins from max rebinned bin.
algo.meanCleanCutMinFactor = 0.4

set_debug_level(35)
set_log_level(LogLevel.INFO)   # LogLevel.INFO or LogLevel.DEBUG


exprun_vector = algo.getRunListFromAllData()

baseName = "eclBhabhaTValidationAlgorithm"
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
