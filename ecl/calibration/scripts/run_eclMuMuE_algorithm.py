#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2

# Run just the collector part of the eclMuMuE calibration, which calibrates the single crystal
# energy response using muon pairs. Runs on the output produced by run_eclMuMuE_collector.py
input = register_module('RootInput')
input.param('inputFileName', 'output/CollectorOutput.root')
input.initialize()

algo = Belle2.eclMuMuEAlgorithm()
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
    iov_to_execute = ROOT.vector("std::pair<int,int>")()
    iov_to_execute.push_back(exprun)
    alg_result = algo.execute(iov_to_execute, 0)
    print("result was", alg_result)

# Commits a successful list of dbobjects to localdb/
algo.commit()
