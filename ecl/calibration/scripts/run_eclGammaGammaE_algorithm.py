#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2017  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Christopher Hearty
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

import os
import sys
import glob
from basf2 import *
import ROOT
from ROOT import Belle2

# Usage: basf2 run_eclGammaGammaE_algorithm.py
# or: bsub -q s -o logfile.txt 'basf2 run_eclGammaGammaE_algorithm.py "inputfiles*.root"'

# Run just the algorithm part of the eclGammaGammaE calibration, which calibrates the single crystal
# energy response using e+e- --> gamma gamma events. Runs on the output produced by
# run_eclGammaGammaE_collector.py.
# Standard usage: algo.performFits = True; performs a Novosibirsk fit on specified crystals to
# extract the calibration constant.
# algo.performFits = False just copies the input histograms into the output file for debugging.

# algo.storeConst = 0 fills database payloads ECLCrystalEnergyGammaGamma or ECLExpGammaGammaE
#                     (if findExpValues=True) for all crystals with successful fits and writes to localdb
# algo.storeConst = 1 fills writes the payloads only if every crystal in specified range is successful
# algo.storeConst = -1 do not write to the database

# Specified output file contains many diagnostic histograms.

algo = Belle2.ECL.eclGammaGammaEAlgorithm()
fileNames = ['eclGammaGammaECollectorOutput.root']
narg = len(sys.argv)
if(narg == 2):
    fileNames = glob.glob(sys.argv[1])
print("input fileNames = ", fileNames)
algo.setInputFileNames(fileNames)
algo.outputName = "eclGammaGammaE_algorithm.root"
# barrel is [1153,7776]; range clear of cdc walls is [161,8608]
algo.cellIDLo = 1
algo.cellIDHi = 8736
algo.minEntries = 150
algo.maxIterations = 10
algo.tRatioMin = 0.45
algo.tRatioMax = 0.60
algo.upperEdgeThresh = 0.02
algo.performFits = True
algo.findExpValues = False
algo.storeConst = 0

# Run algorithm after merging all input files, as opposed to one run at a time
alg_result = algo.execute()
print("result of eclGammaGammaEAlgorithm was", alg_result)

# Commits a successful list of dbobjects to localdb/
algo.commit()
