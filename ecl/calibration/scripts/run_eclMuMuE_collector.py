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

# Usage: basf2 run_eclMuMuE_collector.py
# Run just the collector part of the eclMuMuE calibration, which calibrates the single crystal
# energy response using muon pairs.
# Input file should be MC e+e- --> mu+mu- including ecl digits and track extrapolation.
# Output histograms are written to specified output file.
# run_eclMuMuE_algorithm.py is then used to perform calibration using these histograms, or to simply copy
# them to an output file.
# run_eclMuMuE.py performs both stages of the calibration in a single job.

import os
import sys
from basf2 import *
from ROOT import Belle2

main = create_path()
main.add_module('RootInput', inputFileNames=['/nfs/dust/belle2/user/ferber/data/kkmc_mumu/kkmc-mumu-1485213008/out-*.root'])
main.add_module("HistoManager", histoFileName="eclMuMuECollectorOutput.root")

# Genfit and track extrapolation
gearbox = register_module('Gearbox')
main.add_module(gearbox)
geometry = register_module('Geometry')
main.add_module(geometry)
main.add_module("SetupGenfitExtrapolation")
ext = register_module('Ext')

# extrapolate using muon hypothesis only
pdgcodes = [13]
ext.param('pdgCodes', pdgcodes)

main.add_module(ext)

eclMuMuE = register_module('eclMuMuECollector')
eclMuMuE.param('minPairMass', 9.0)
eclMuMuE.param('minTrackLength', 30.)
eclMuMuE.param('MaxNeighborAmp', 200.)
# 3 axial SL = [24,134];  4 axial SL = [30,126]
eclMuMuE.param('thetaLabMinDeg', 30.)
eclMuMuE.param('thetaLabMaxDeg', 126.)
# Can fill histograms with MC true deposited energy if eclCalDigits have been stored
eclMuMuE.param('useTrueEnergy', False)
main.add_module(eclMuMuE)

main.add_module('Progress')

set_log_level(LogLevel.INFO)

process(main)

print(statistics)
