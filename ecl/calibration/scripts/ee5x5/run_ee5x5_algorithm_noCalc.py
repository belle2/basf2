#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Christopher Hearty
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

import sys
import glob
from ROOT import Belle2

# Usage: basf2 run_ee5x5_algorithm_noCalc.py "inputfiles*.root"

# Run just the algorithm part of the eclee5x5 calibration, which calibrates the single crystal
# energy response using Bhabha events. Runs on the output produced by the collector stage.

# This scripts merges the collector files and writes the hisograms to the specified output file.
# Other scripts are available to obtain payloads "ECLCrystalEnergy5x5", "ECLExpee5x5E",
# "ECLeedPhiData", or "ECLeedPhiMC".

algo = Belle2.ECL.eclee5x5Algorithm()
fileNames = ['eclee5x5CollectorOutput.root']
narg = len(sys.argv)
if(narg == 2):
    fileNames = glob.glob(sys.argv[1])
print("input fileNames = ", fileNames)
algo.setInputFileNames(fileNames)
algo.setOutputName("eclee5x5_algorithm_noCalc.root")
algo.setMinEntries(150)
algo.setPayloadName("None")
algo.setStoreConst(False)


# Run algorithm after merging all input files, as opposed to one run at a time
alg_result = algo.execute()
print("result of eclee5x5Algorithm was", alg_result)

# Commits a successful list of dbobjects to localdb/
algo.commit()
