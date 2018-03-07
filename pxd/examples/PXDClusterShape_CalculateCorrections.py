#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to use PXD shape correction.
# Shape correction is realize in few steps, see examples:
#   pxd/examples/PXDClasterShape_PrepareSources.py
#   pxd/examples/PXDClasterShape_CalculateCorrections.py
#   pxd/examples/PXDClasterShape_MergeCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections.py
#   pxd/examples/PXDClasterShape_ApplyCorrections2.py
#   pxd/examples/PXDClasterShape_CheckCorrections.py
# It use data stored in a root file from step PrepareSources.
#
# "UseRealData" to use real data without simulations or simulations (default)
# "PixelKind"=-1: realistic physics simulations or real data (default)
# "PixelKind"!=-1: special calibration for full range of angles, only simulation!
#    this case is split for every kind of pixel size (4 cases)
#    partical gun is locate close to sensor with giving pixel kind
#    perticle gun is define in PXDClusterShape_ParticleGunSetting.py class
#    there are storing only data for giving pixel kind
#    PixelKind=0: 55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
#    PixelKind=1: 60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
#    PixelKind=2: 70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
#    PixelKind=3: 85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]
#    TODO: Need to check influence of orientation of u & v axes for corections
#
# Parameters are not independent, there are some priorities:
#   Priorities: PixelKind, rest is depend
#   if you set UseRealData=True, rest is set automatically.
#
# There is possibility to call this macro with parameters:
#    basf2 path/PXDClasterShape_CalculateCorrections.py -- --UseRealData --PixelKind=-1
#
#    Example for help:
#    basf2 pxd/examples/PXDClasterShape_CalculateCorrections.py -- --h
#
# Author: The Belle II Collaboration                                     *
# Contributors: Peter.Kodys@mff.cuni.cz                                            *
#
##############################################################################
import os
import sys

from basf2 import *
from ROOT import Belle2

# following lines diconnect central DataBase:
from ROOT import Belle2
reset_database()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)
# end of lines diconnect central DataBase

import argparse
parser = argparse.ArgumentParser(description="PXD Cluster Shape: create sources for corrections preparing.")

# parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument('--PixelKind', dest='PixelKind', action='store', default=-1, type=int,
                    help='Set kind of data preparing: -1: IP source, 0: pitch 55 um, 1: 60 um, 2: 70 um, 3: 85 um, default=-1.')
parser.add_argument('--UseRealData', dest='UseRealData', action='store_const', const=True, default=False,
                    help='(experts only) Use real data for cluster shape corrections, not simulations, default=False.')
parser.add_argument('--InputFileName', dest='inputFileName', action='store',
                    default='', type=str,
                    help='Name of file contain source data, default=pxdClShCalSrc_RealData_PixelKind.root')
parser.add_argument('--StoreEECorrectionType', dest='StoreEECorrectionType', action='store', default=0, type=int,
                    help='Use EE calculate from 0: true EE, 1: EE from Track (real data), 2: EE from Track residuals, default=0.')

args = parser.parse_args()

# show warnings during processing
# set_log_level(LogLevel.WARNING)
# set_log_level(LogLevel.DEBUG)
# to see many details:
# set_debug_level(300)
# set_debug_level(100)

# Crosscheck of sets ond other sets:
if (args.UseRealData is True):
    args.PixelKind = -1

if (args.PixelKind >= 0):
    args.UseRealData = False

print("Final setting of arguments: ")
print("               UseRealData: ", args.UseRealData)
print("                 PixelKind: ", args.PixelKind)
print("     StoreEECorrectionType: ", args.StoreEECorrectionType)

inputFileName = 'pxdClShCalSrc_RealData' + str(args.UseRealData)
inputFileName = inputFileName + '_PixelKind' + str(args.PixelKind)
if (args.PixelKind == -1):
    inputFileName = inputFileName + '_Type' + str(args.StoreEECorrectionType) + '_2mils'
else:
    inputFileName = inputFileName + '_10mils'
inputFileName = inputFileName + '.root'

if (args.inputFileName != ''):
    inputFileName = args.inputFileName

# "/scratch/kodys/tempdataBasf2/pxdClShCalSrc_RealDataFalse_PixelKind-1_005mils6*"
# pxdClShCalSrc_RealDataFalse_PixelKind0_10mils9.root
algo = Belle2.PXDClusterShapeCalibrationAlgorithm()
algo.setInputFiles([inputFileName])
algo.setUseRealData(args.UseRealData)
algo.setPixelKind(args.PixelKind)
algo.setStoreEECorrectionType(args.StoreEECorrectionType)

# algo.setDoExpertHistograms(False)
algo.setDoExpertHistograms(True)

algo.execute()

# This would send created payloads to the database
# algo.commit()
# Register modules
