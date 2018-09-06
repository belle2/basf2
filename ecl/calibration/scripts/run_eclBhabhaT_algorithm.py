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

# Usage: basf2 run_eclBhabhaT_algorithm.py

import os
import sys
from basf2 import *
from ROOT import Belle2
from glob import glob

algo = Belle2.ECL.eclBhabhaTAlgorithm()
algo.setInputFileNames(['eclBhabhaTCollector.root'])

algo.cellIDLo = 1
algo.cellIDHi = 8736
algo.maxIterations = 10
algo.debugOutput = True
algo.debugFilename = "eclBhabhaTAlgorithm.root"

exprun_vector = algo.getRunListFromAllData()
alg_result = algo.execute()
print("result was", alg_result)
algo.commit()
