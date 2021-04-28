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

# Usage: basf2 run_eclMuMuE.py
# Energy calibration of (part of) the ecl using muon pairs.
# Runs within the CAF framework.
# Calls both collector and algorithm stages of the calibration;
# run_eclMuMuE_collector.py and run_eclMuMuE_algorithm.py run the two stages as separate jobs for
# debugging purposes.
# Input file should be e+e- --> mu+mu- including ecl digits and track extrapolation.
# Production version (but not this version) should call required reconstruction modules.
# Primary output is a histogram of calibration constant vs cellID0;
# output file RootOutput.root contains many additional diagnostic histograms.

import basf2 as b2

from ROOT.Belle2 import eclMuMuEAlgorithm
from caf.framework import Calibration, CAF

# Specify the input
import glob
inputFileNames = glob.glob('/nfs/dust/belle2/user/ferber/data/kkmc_mumu/kkmc-mumu-1485213008/*.root')
print(inputFileNames)
b2.set_log_level(b2.LogLevel.INFO)

# The collector module
eclMuMuE = b2.register_module('eclMuMuECollector')
eclMuMuE.param('minPairMass', 9.0)
eclMuMuE.param('minTrackLength', 30.)
eclMuMuE.param('MaxNeighborAmp', 200.)
# 3 axial SL = [24,134];  4 axial SL = [30,126]
eclMuMuE.param('thetaLabMinDeg', 24.)
eclMuMuE.param('thetaLabMaxDeg', 134.)
# Can fill histograms with MC true deposited energy if eclCalDigits have been stored
eclMuMuE.param('useTrueEnergy', False)

# Set up the modules needed for geometry and track extrapolation by the collector module
pre_path = b2.create_path()
gearbox = b2.register_module('Gearbox')
pre_path.add_module(gearbox)
geometry = b2.register_module('Geometry')
pre_path.add_module(geometry)
pre_path.add_module('SetupGenfitExtrapolation')
ext = b2.register_module('Ext')
pdgcodes = [13]
ext.param('pdgCodes', pdgcodes)
pre_path.add_module(ext)

# Specify the algorithm.  barrel is cells [1152,7775]; barrel plus one endcap ring is [1008,7919]
algorithm = eclMuMuEAlgorithm()
algorithm.cellIDLo = 1008
algorithm.cellIDHi = 7919
algorithm.minEntries = 150
algorithm.maxIterations = 10
algorithm.tRatioMin = 0.2
algorithm.tRatioMax = 0.25
algorithm.performFits = True

# Create a single calibration from a collector module name + algorithm + input files
cal = Calibration(name='eclMuMuECalibration', collector=eclMuMuE, algorithms=algorithm, input_files=inputFileNames)
cal.pre_collector_path = pre_path

# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = 'fullBG'
cal_fw.run()
