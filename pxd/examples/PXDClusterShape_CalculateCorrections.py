#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to use PXD shape correction.
# Shape correction is realize in three steps, se exampes:
#   pxd/examples/PXDClasterShape_PrepareSources.py
#   pxd/examples/PXDClasterShape_CalculateCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections2.py
# It use data stored in a root file from step PrepareSources.
#
# "UseTracks" to use track information (default) or simulations
# "UseRealData" to use real data without simulations or simulations (default)
# "CompareTruePointTracks" to compare true point and track position in simulations (default = False)
# "CalibrationKind" to run this task:
# 1) (default) standard calibration based on realistic physics or real data
# 2) special calbration for full range of angles, only for simulation!
#    this case is split for every kind of pixel size (4 cases)
#    partical gun is locate close to sensor with giving pixel kind
#    there are storing only data for giving pixel kind
# "PixelKind" only for CalibrationKind=2 for every kind of pixel
#    PixelKind=0: 55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
#    PixelKind=1: 60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
#    PixelKind=2: 70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
#    PixelKind=3: 85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]
#    TODO: Need to check influence of orientation of u & v axes for corections
#
# Parameters are not independent, there are some priorities:
#   Priorities: UseRealData, UseTracks, rest is independent
#   if you set UseRealData=True, UseTracks is set automatically to True (noe TrueHit data).
#
# There is possibility to call this macro with presets in parameters:
#    basf2 PXDClasterShape_CalculateCorrections.py UseTracks UseRealData CalibrationKind PixelKind
#    Example for defaults:
#    basf2 pxd/examples/PXDClasterShape_CalculateCorrections.py 1 0 1 0
#
##############################################################################
import os
import sys

from basf2 import *
from ROOT import Belle2

reset_database()
use_local_database()

# show warnings during processing
set_log_level(LogLevel.WARNING)
set_log_level(LogLevel.INFO)
set_log_level(LogLevel.DEBUG)
set_debug_level(50)

# Presets (defaults, no need to set if no change):
UseTracks = True
UseRealData = False
CompareTruePointTracks = False
# For special calibration, only for experts:
CalibrationKind = 1
PixelKind = 0

# If exist load from arguments:
argvs = sys.argv
argc = len(argvs)
print("Number of arguments: ", argc - 1)
if argc >= 2:
    if (argvs[1] == '0'):
        UseTracks = False
    if (argvs[1] == '1'):
        UseTracks = True
    print("first argument UseTracks: ", UseTracks)
if argc >= 3:
    if (argvs[2] == '0'):
        UseRealData = False
    if (argvs[2] == '1'):
        UseRealData = True
    print("second argument UseRealData: ", UseRealData)
if argc >= 4:
    CalibrationKind = int(argvs[3])
    print("third argument CalibrationKind: ", CalibrationKind)
if argc >= 5:
    PixelKind = int(argvs[4])
    print("fours argument PixelKind: ", PixelKind)

# Crosscheck of presets:
if UseTracks:
    CompareTruePointTracks = True
    CalibrationKind = 1
if (UseRealData is True):
    UseTracks = True
    CalibrationKind = 1
    CompareTruePointTracks = False
if (CalibrationKind == 2):
    UseTracks = False
    CompareTruePointTracks = False

print("Final setting of arguments: ")
print("                 UseTracks: ", UseTracks)
print("               UseRealData: ", UseRealData)
print("    CompareTruePointTracks: ", CompareTruePointTracks)
print("           CalibrationKind: ", CalibrationKind)
print("                 PixelKind: ", PixelKind)

# inputFileName = 'pxdClShapeCalibrationSource_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
# inputFileName = inputFileName + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKind) + '.root'

inputFileName = 'pxdClShCalSrc_RealData' + str(UseRealData) + '_Track' + str(UseTracks)
inputFileName = inputFileName + '_Calib' + str(CalibrationKind) + '_Pixel' + str(PixelKind) + '.root'

algo = Belle2.PXDClusterShapeCalibrationAlgorithm()
algo.setInputFiles([inputFileName])
algo.setUseTracks(UseTracks)
algo.setUseRealData(UseRealData)
algo.setCompareTruePointTracks(CompareTruePointTracks)
algo.setCalibrationKind(CalibrationKind)
algo.setPixelKind(PixelKind)
# algo.setDoExpertHistograms(False)
algo.setDoExpertHistograms(True)

# algo.set_log_level(LogLevel.INFO)
# algo.set_log_level(LogLevel.DEBUG)
# algo.set_debug_level(100)

algo.execute()

# This would send created payloads to the database
# algo.commit()
# Register modules
