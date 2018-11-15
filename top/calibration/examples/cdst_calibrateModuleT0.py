#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import TFile, TH1F

# ---------------------------------------------------------------------------------------
# Example of calibrating module T0 on cdst files using new constants from local DB
#
# Note: replace local database name/location before running or comment it out
# ---------------------------------------------------------------------------------------

# Suppress messages during processing:
set_log_level(LogLevel.RESULT)

# Database:
# - replace the name and location of the local DB before running!
# - one can even use several local DB's
# - payloads are searched for in the reverse order of DB's given below; therefore the new
#   calibration, if provided, is taken from the local DB.
use_central_database('development')  # some new stuff not in production tag
use_central_database('data_reprocessing_prod6')  # global tag used in production of cdst
use_local_database('zzTBCdb/localDB/localDB.txt', 'zzTBCdb/localDB/')  # new calibration

# Create path
main = create_path()

# input: cdst file(s) - bhabha skim
roinput = register_module('RootInput')
main.add_module(roinput)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'TOP'])
geometry.param('useDB', False)
main.add_module(geometry)

# Time Recalibrator
recalibrator = register_module('TOPTimeRecalibrator')
recalibrator.param('useSampleTimeCalibration', True)
recalibrator.param('useChannelT0Calibration', True)
recalibrator.param('useModuleT0Calibration', False)
recalibrator.param('useCommonT0Calibration', True)
recalibrator.param('subtractBunchTime', True)
main.add_module(recalibrator)

# Module T0 calibration w/ TOPAligner using bhabha's
# - to be replaced with a dedicated module employing different method
# - TOPAligner needs good choice of initial parameter values to converge
initT0 = [1.16, 0.32, 0.37, 0.0, 0.27, -0.21, -0.74, -1.28,
          -1.04, -1.17, 0.08, -0.17, 0.14, -0.43, -1.17, -1.47]  # Senga-san values
fileNames = {}
for moduleId in range(1, 17):
    fileName = 'moduleT0_slot' + '{:0=2d}'.format(moduleId) + '.root'
    if moduleId not in fileNames:
        fileNames[moduleId] = fileName
    align = register_module('TOPAligner')
    align.param('targetModule', moduleId)
    align.param('sample', 'bhabha')
    align.param('parInit', [0, 0, 0, 0, 0, 0, initT0[moduleId-1]])
    align.param('parFixed', ['x', 'y', 'z', 'alpha', 'beta', 'gamma', 'dn/n'])
    align.param('outFileName', fileName)
    main.add_module(align)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)

# Make summary root file

h_valid = TH1F('valid', 'status', 16, 0.5, 16.5)
h_valid.SetXTitle('slot ID')
h_ntrk = TH1F('ntrk', 'number of tracks', 16, 0.5, 16.5)
h_ntrk.SetXTitle('slot ID')
h_results = TH1F('results', 'module T0', 16, 0.5, 16.5)
h_results.SetXTitle('slot ID')
h_results.SetYTitle('module T0 [ns]')

for moduleId in fileNames:
    fileName = fileNames[moduleId]
    file = TFile.Open(fileName)
    if not file:
        B2ERROR("can't open file " + fileName)
        continue

    valid = TH1F(file.Get("valid"))
    h_valid.Add(valid)
    ntrk = TH1F(file.Get("ntrk"))
    h_ntrk.Add(ntrk)
    if valid.GetSum() != 0:
        result = TH1F(file.Get("results_slot" + str(moduleId)))
        h_results.SetBinContent(moduleId, result.GetBinContent(7))
        h_results.SetBinError(moduleId, result.GetBinError(7))
    file.Close()

file = TFile.Open('moduleT0-summary.root', 'recreate')
h_valid.Write()
h_ntrk.Write()
h_results.Write()
file.Close()
