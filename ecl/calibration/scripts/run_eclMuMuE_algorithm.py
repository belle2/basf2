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
from basf2 import *
import ROOT
from ROOT import Belle2

# Usage: basf2 run_eclMuMuE_algorithm.py
# Run just the collector part of the eclMuMuE calibration, which calibrates the single crystal
# energy response using muon pairs. Runs on the output produced by run_eclMuMuE_collector.py.
# Standard usage: algo.performFits = True; performs a Novosibirsk fit on specified crystals to
# extract the calibration constant.
# algo.performFits = False just copies the input histograms into the output file for debugging.
# Primary output is a histogram of calibration constant vs cellID0;
# specified output file contains many additional diagnostic histograms.
# run_eclMuMuE.py performs both stages of the calibration in a single job.


algo = Belle2.eclMuMuEAlgorithm()
algo.setInputFiles(['output/CollectorOutput.root'])
# barrel is [1152,7775]; barrel plus one endcap ring is [1008,7919]
algo.cellIDLo = 1008
algo.cellIDHi = 7919
algo.minEntries = 150
algo.maxIterations = 10
algo.tRatioMin = 0.2
algo.tRatioMax = 0.25
algo.performFits = True

exprun_vector = algo.getRunListFromAllData()
for exprun in exprun_vector:
    iov_to_execute = [(exprun.first, exprun.second)]
    alg_result = algo.execute(iov_to_execute, 0)
    print("result was", alg_result)
    # Commits a successful list of dbobjects to localdb/
    algo.commit()
