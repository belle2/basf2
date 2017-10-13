#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


from basf2 import *
import argparse
# Import custom module chain for VXDTF2
from setup_modules import setup_VXDTF2
from setup_modules import setup_Geometry
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

performFit = True

# Logging and Debug Levels
set_log_level(LogLevel.ERROR)
log_to_file('logVXDTF2Evaluation.log', append=False)


# ---------------------------------------------------------------------------------------
path = create_path()

# Input
rootInput = register_module('RootInput')
path.add_module(rootInput)

# Event Info Module
eventinfoprinter = register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

# puts gearbox and geometry into the path
setup_Geometry(path)

# Event counter
eventCounter = register_module('EventCounter')
path.add_module(eventCounter)

# VXDTF2: Including actual VXDTF2 Modul Chain
setup_VXDTF2(path=path,
             use_pxd=usePXD,
             sec_map_file=sec_map_file,
             setup_name=setup_name,
             overlap_filter='hopfield',
             quality_estimator='circleFit')

if performFit:
    genFitExtrapolation = register_module('SetupGenfitExtrapolation')
    path.add_module(genFitExtrapolation)

    fitter = register_module('DAFRecoFitter')
    path.add_module(fitter)
    path.add_module('TrackCreator', pdgCodes=[211, 13, 321, 2212])

# Matching
mcTrackMatcherModule = register_module('MCRecoTracksMatcher')
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

process(path)
print(statistics)
