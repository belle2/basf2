#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to merge PXD corrections for cluster shape.
# Shape correction is realize in few steps, see examples:
#   pxd/examples/PXDClasterShape_PrepareSources.py
#   pxd/examples/PXDClasterShape_CalculateCorrections.py
#   pxd/examples/PXDClasterShape_MergeCorrections.py      <--
#
# It use data stored in a root files from step CalculateCorrections.
#
# There is possibility to call this macro with parameters:
#    basf2 path/PXDClasterShape_MergeCorrections.py -- --CalFileOutputName='TestOutput.root'
#                                                      --CalFileBasicName='TestBasicInput.root'
#
#    Example for help:
#    basf2 pxd/examples/PXDClasterShape_MergeCorrections.py -- --h
#
# Author: The Belle II Collaboration
# Contributors: Peter.Kodys@mff.cuni.cz
#
##############################################################################
import os
import sys

from basf2 import *
from ROOT import Belle2

# following lines diconnect central DataBase:
reset_database()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)
# end of lines diconnect central DataBase

import argparse
parser = argparse.ArgumentParser(description="PXD Cluster Shape: merge corrections.")

# parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument('--CalFileBasicName', dest='CalFileBasicName', action='store',
                    default='pxdClShCal_RealData0_Pixel-1.root', type=str,
                    help='Name of file contain basic calibration, default="pxdClShCal_RealData0_Pixel-1')
parser.add_argument('--CalFilePK0Name', dest='CalFilePK0Name', action='store',
                    default='pxdClShCal_RealData0_Pixel0.root', type=str,
                    help='Name of file contain calibration for pixel kind 0 (PitchV=55um), default=pxdClShCal_RealData0_Pixel0')
parser.add_argument('--CalFilePK1Name', dest='CalFilePK1Name', action='store',
                    default='pxdClShCal_RealData0_Pixel1.root', type=str,
                    help='Name of file contain calibration for pixel kind 1 (PitchV=60um), default=pxdClShCal_RealData0_Pixel1')
parser.add_argument('--CalFilePK2Name', dest='CalFilePK2Name', action='store',
                    default='pxdClShCal_RealData0_Pixel2.root', type=str,
                    help='Name of file contain calibration for pixel kind 2 (PitchV=70um), default=pxdClShCal_RealData0_Pixel2')
parser.add_argument('--CalFilePK3Name', dest='CalFilePK3Name', action='store',
                    default='pxdClShCal_RealData0_Pixel3.root', type=str,
                    help='Name of file contain calibration for pixel kind 3 (PitchV=85um), default=pxdClShCal_RealData0_Pixel3')

parser.add_argument('--CalFileOutputName', dest='CalFileOutputName', action='store',
                    default='PXD-ClasterShapeCorrections.root', type=str,
                    help='Name of file contain output merged calibration, default=PXD-ClasterShapeCorrections')

parser.add_argument('--DifferenceBias', dest='DifferenceBias', action='store', default=1.0, type=float,
                    help='Only over this limit is bias correction accepted, default = 1.0 um, in [cm]')
parser.add_argument('--DifferenceBiasClose', dest='DifferenceBiasClose', action='store', default=3.0, type=float,
                    help='Only under this limit is real bias correction compare to simulation accepted, default = 3.0 um, in [cm]')
parser.add_argument('--DifferenceErrEst', dest='DifferenceErrEst', action='store', default=1.0, type=float,
                    help='Only under this limit is real error estimation correction compare accepted, default = 0.1 um, in [cm]')

args = parser.parse_args()

# show warnings during processing
# set_log_level(LogLevel.WARNING)
set_log_level(LogLevel.DEBUG)
# to see many details:
set_debug_level(300)
set_debug_level(30)

print("Final setting of arguments: ")
print("               CalFileBasicName: ", args.CalFileBasicName)
print("                 CalFilePK0Name: ", args.CalFilePK0Name)
print("                 CalFilePK1Name: ", args.CalFilePK1Name)
print("                 CalFilePK2Name: ", args.CalFilePK2Name)
print("                 CalFilePK3Name: ", args.CalFilePK3Name)
print("              CalFileOutputName: ", args.CalFileOutputName)

pxdMergeClusterShapeCorrections = register_module('pxdMergeClusterShapeCorrections')
pxdMergeClusterShapeCorrections.param('CalFileBasicName', args.CalFileBasicName)
pxdMergeClusterShapeCorrections.param('CalFilePK0Name', args.CalFilePK0Name)
pxdMergeClusterShapeCorrections.param('CalFilePK1Name', args.CalFilePK1Name)
pxdMergeClusterShapeCorrections.param('CalFilePK2Name', args.CalFilePK2Name)
pxdMergeClusterShapeCorrections.param('CalFilePK3Name', args.CalFilePK3Name)
pxdMergeClusterShapeCorrections.param('CalFileOutputName', args.CalFileOutputName)

main = create_path()
main.add_module('EventInfoSetter')
main.add_module(pxdMergeClusterShapeCorrections)

process(main)
