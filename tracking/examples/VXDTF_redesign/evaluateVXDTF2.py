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
# VXDTF2 Example Scripts - Step 4 - Evaluation of the VXDTF2
#
# Evaluation of VXDTF2 which applies a trained Sector Map on the provided
# data. The root file containing the SecMap can be passed via the
# none-basf2 commandline argument --secmap.
#
# Usage: basf2 evaluateVXDTF2.py -i <inputFileName> [-- --secmap <secmapFile>]
#
#
# Contributors: Jonas Wagner
#####################################################################


import basf2 as b2
import sys
import argparse
# Import custom module chain for VXDTF2
from setup_modules import setup_VXDTF2
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule

# ---------------------------------------------------------------------------------------
# Argument parser for input of trained Sector Map.
arg_parser = argparse.ArgumentParser(description='VXDTF2 evaluation:\
                                     Applies VXDTF2 to selected dataset \n\
                                     Usage: basf2 testVXDTF2.py -i <inputFileName> -- --secmap <secmapFile>')

arg_parser.add_argument('--secmap', '-s', type=str,
                        help='Inclusion of the root file containing the trained SecMap for the application of the VXDTF2.')

arguments = arg_parser.parse_args(sys.argv[1:])
sec_map_file = arguments.secmap


# ---------------------------------------------------------------------------------------
# Settings
usePXD = False

# these are the "default" settings
setup_name = 'SVDOnlyDefault'
if usePXD:
    setup_name = 'SVDPXDDefault'

performFit = False

# Logging and Debug Levels
b2.set_log_level(b2.LogLevel.ERROR)
b2.log_to_file('logVXDTF2Evaluation.log', append=False)


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

# puts gearbox and geometry into the path
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
             sec_map_file=sec_map_file,
             setup_name=setup_name,
             overlap_filter='hopfield',
             quality_estimator='circleFit')

if performFit:
    genFitExtrapolation = b2.register_module('SetupGenfitExtrapolation')
    path.add_module(genFitExtrapolation)

    fitter = b2.register_module('DAFRecoFitter')
    path.add_module(fitter)
    path.add_module('TrackCreator', pdgCodes=[211, 13, 321, 2212])

# Matching
mcTrackMatcherModule = b2.register_module('MCRecoTracksMatcher')
mcTrackMatcherModule.param({
    'UseCDCHits': False,
    'UseSVDHits': True,
    'UsePXDHits': usePXD,
    'mcRecoTracksStoreArrayName': 'MCRecoTracks',
    'MinimalPurity': .66,
})
path.add_module(mcTrackMatcherModule)

# Evaluation of matching
trackingValidationModule = CombinedTrackingValidationModule(
    "",
    contact="",
    output_file_name="VXDTF2Validation.root",
    expert_level=2)
path.add_module(trackingValidationModule)

path.add_module('Progress')
b2.process(path)
print(b2.statistics)
