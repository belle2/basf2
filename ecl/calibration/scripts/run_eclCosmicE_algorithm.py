#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------
# BASF2 (Belle Analysis Framework 2)
# Copyright(C) 2018  Belle II Collaboration
#
# Author: The Belle II Collaboration
# Contributors: Christopher Hearty, hearty@physics.ubc.ca
#
# This software is provided "as is" without any warranty.
# -----------------------------------------------------------

import os
import sys
import glob
from basf2 import *
import ROOT
from ROOT import Belle2

# Run algorithm part of the eclCosmicE calibration, which calibrates the single crystal
# energy response using cosmic rays. Input files have been produced by eclCosmicECollector,
# using run_eclCosmicE_collector.py

# Usage: basf2 run_eclCosmicE_algorithm.py
# or: bsub -q s -o logfile.txt 'basf2 run_eclCosmicE_algorithm.py "inputfiles*.root"'

# algo.performFits = False: just copy the input histograms into the output file for debugging.
# algo.findExpValues: True: fits are used to find the expected energy deposits
#                     False: fits are used to find the crystal energy calibration constants

# algo.storeConst = 0 fills database payloads ECLExpCosmicESame and ECLExpCosmicEDifferent (findExpValues=True)
#                     or payload ECLCrystalEnergyCosmic (if findExpValues=False)
#                     for all crystals with successful fits and writes to localdb
# algo.storeConst = 1 fills writes the payloads only if every crystal in specified range is successful
# algo.storeConst = -1 do not write to the database

# directory to store produced constants. If not specified, use localdb subdirectory of current working directory.
# use_local_database("/home/belle2/czhearty/CosmicCalib/workdir/localdb/database.txt",directory="/home/belle2/czhearty/CosmicCalib/workdir/localdb")

algo = Belle2.ECL.eclCosmicEAlgorithm()

fileNames = ['eclCosmicECollectorOutput.root']
narg = len(sys.argv)
if(narg == 2):
    fileNames = glob.glob(sys.argv[1])
print("input fileNames = ", fileNames)
algo.setInputFileNames(fileNames)

# barrel is [1153,7776]
algo.cellIDLo = 1
algo.cellIDHi = 8736
algo.minEntries = 150
algo.tRatioMin = 0.2
algo.tRatioMax = 0.25
algo.maxIterations = 10
algo.performFits = True
algo.findExpValues = False
algo.storeConst = 0

# Run algorithm after merging all input files, as opposed to one run at a time
alg_result = algo.execute()
print("result of eclCosmicEAlgorithm was", alg_result)

# Commits a successful list of dbobjects to localdb/
algo.commit()
