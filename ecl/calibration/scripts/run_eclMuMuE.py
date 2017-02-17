#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Energy calibration of ecl barrel plus one theta ring of each endcap using muon pairs.
# Runs within the CAF framework.
from basf2 import *
set_log_level(LogLevel.INFO)

import ROOT
from ROOT.Belle2 import eclMuMuEAlgorithm
from caf.framework import Calibration, CAF

# Specify the input
import glob
inputFileNames = glob.glob('/nfs/dust/belle2/user/ferber/data/kkmc_mumu/kkmc-mumu-1485213008/*.root')
print(inputFileNames)

# The collector module
eclMuMuE = register_module('eclMuMuECollector')
eclMuMuE.param('minPairMass', 9.0)
eclMuMuE.param('minTrackLength', 30.)
eclMuMuE.param('MaxNeighborAmp', 200.)
# 3 axial SL = [24,134];  4 axial SL = [30,126]
eclMuMuE.param('thetaLabMinDeg', 24.)
eclMuMuE.param('thetaLabMaxDeg', 134.)
eclMuMuE.param('useTrueEnergy', False)

# Set up the modules needed for geometry and track extrapolation by the collector module
pre_path = create_path()
gearbox = register_module('Gearbox')
pre_path.add_module(gearbox)
geometry = register_module('Geometry')
pre_path.add_module(geometry)
pre_path.add_module('SetupGenfitExtrapolation')
ext = register_module('Ext')
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
