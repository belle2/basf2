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
from basf2 import *
from ROOT import Belle2
from glob import glob

algo = Belle2.ECL.eclBhabhaTAlgorithm()

fileNames = ['eclBhabhaTCollector.root']
narg = len(sys.argv)
if(narg >= 2):
    fileNames = glob(sys.argv[1])
algo.setInputFileNames(fileNames)

algo.cellIDLo = 1
algo.cellIDHi = 8736
algo.maxIterations = 10
algo.debugOutput = True
algo.debugFilename = "eclBhabhaTAlgorithm.root"

exprun_vector = algo.getRunListFromAllData()
alg_result = algo.execute()
print("result was", alg_result)
algo.commit()
