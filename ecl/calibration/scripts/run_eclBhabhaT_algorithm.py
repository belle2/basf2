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

algo = Belle2.ECL.eclBhabhaTAlgorithm()
algo.setInputFileNames(['eclBhabhaTCollector.root'])
algo.cellIDLo = 0
algo.cellIDHi = 8735
algo.maxIterations = 10
algo.debugOutput = True
algo.debugFilename = "eclBhabhaTAlgorithm.root"

exprun_vector = algo.getRunListFromAllData()
for exprun in exprun_vector:
    iov_to_execute = [(exprun.first, exprun.second)]
    alg_result = algo.execute(iov_to_execute, 0)
    print("result was", alg_result)

    if (alg_result == 0):
        # Commits a successful list of dbobjects to localdb/
        algo.commit()
