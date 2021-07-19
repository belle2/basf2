#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# VXDTF2 Example Scripts - Step 3 - Application of the VXDTF2
#
# Test of VXDTF2 which applies a trained Sector Map on the provided
# data. The root file containing the SecMap can be passed via the
# none-basf2 commandline argument --secmap.
#
# Usage: basf2 testVXDTF2.py -i <inputFileName> -o <outputFileName> -- --secmap <secmapFile>
#####################################################################


import basf2 as b2
import argparse
# Import custom module chain for VXDTF2
from setup_modules import setup_VXDTF2
import sys


# ---------------------------------------------------------------------------------------
# Argument parser for input of trained Sector Map.
arg_parser = argparse.ArgumentParser(description='VXDTF2 application:\
                                     Applies trained SecMap on provided data and returns RecoTrackCands.\n\
                                     Usage: basf2 testVXDTF2.py -i <inputFileName> -o <outputFileName> -- --secmap <secmapFile>')

arg_parser.add_argument('--secmap', '-s', type=str,
                        help='Inclusion of the root file containing the trained SecMap for the application of the VXDTF2.')

arguments = arg_parser.parse_args(sys.argv[1:])
secmap_name = arguments.secmap


# ---------------------------------------------------------------------------------------
# Settings
usePXD = False
useDisplay = False

performFit = False
generateTimeSeedAfterFit = False

# Logging and Debug Levels
b2.set_log_level(b2.LogLevel.ERROR)
b2.log_to_file('logVXDTF2Execution.log', append=False)


# ---------------------------------------------------------------------------------------
path = b2.create_path()

# Input
rootInput = b2.register_module('RootInput')
path.add_module(rootInput)

# Event Info Module
eventinfoprinter = b2.register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

# Gearbox
gearbox = b2.register_module('Gearbox')
path.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
geometry.param('components', ['BeamPipe',
                              'MagneticFieldConstant4LimitedRSVD',
                              'PXD',
                              'SVD'])
path.add_module(geometry)


# VXDTF2: Including actual VXDTF2 Modul Chain
setup_VXDTF2(path=path,
             use_pxd=usePXD,
             sec_map_file=secmap_name,
             overlap_filter='hopfield',
             quality_estimator='circleFit')


if performFit:
    # This is required for RecoFitter
    genFitExtrapolation = b2.register_module('SetupGenfitExtrapolation')
    path.add_module(genFitExtrapolation)

    if not generateTimeSeedAfterFit:
        timeSeed = b2.register_module('IPTrackTimeEstimator')
        timeSeed.param('useFittedInformation', False)
        path.add_module(timeSeed)

    fitter = b2.register_module('DAFRecoFitter')
    path.add_module(fitter)

    if generateTimeSeedAfterFit:
        timeSeedAfterFit = b2.register_module('IPTrackTimeEstimator')
        timeSeedAfterFit.param('useFittedInformation', True)
        path.add_module(timeSeedAfterFit)


output = b2.register_module('RootOutput')
path.add_module(output)


if useDisplay:
    display = b2.register_module('Display')
    display.param('showAllPrimaries', True)
    path.add_module(display)

path.add_module('Progress')
b2.process(path)
print(b2.statistics)
