#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#####################################################################
# VXDTF1 Example Scripts
#
# Evaluation of VXDTF1.
#
# Usage: basf2 evaluateVXDTF1.py -i <inputFileName>
#
#
# Contributors: Jonas Wagner
#####################################################################


from basf2 import *
import tracking
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule


# ---------------------------------------------------------------------------------------
# Settings
usePXD = False

performFit = False

# Logging and Debug Levels
set_log_level(LogLevel.ERROR)
log_to_file('logVXDTF1Evaluation.log', append=False)


# ---------------------------------------------------------------------------------------
path = create_path()

# Input
rootInput = register_module('RootInput')
path.add_module(rootInput)

# Event Info Module
eventinfoprinter = register_module('EventInfoPrinter')
path.add_module(eventinfoprinter)

# Gearbox
gearbox = register_module('Gearbox')
path.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe',
                              'MagneticFieldConstant4LimitedRSVD',
                              'PXD',
                              'SVD',
                              'CDC'])
path.add_module(geometry)

# Event counter
eventCounter = register_module('EventCounter')
path.add_module(eventCounter)

genFitExtrapolation = register_module('SetupGenfitExtrapolation')
path.add_module(genFitExtrapolation)

tracking.add_vxd_track_finding(path, "RecoTracks", components=["SVD"])

if performFit:
    fitter = register_module('DAFRecoFitter')
    path.add_module(fitter)

# Matching
mcTrackMatcherModule = register_module('MCRecoTracksMatcher')
mcTrackMatcherModule.param({
    'UseCDCHits': False,
    'UseSVDHits': True,
    'UsePXDHits': False,
    'mcRecoTracksStoreArrayName': 'MCRecoTracks',
    'MinimalPurity': .66,
})
path.add_module(mcTrackMatcherModule)

# Evaluation of matching
trackingValidationModule = CombinedTrackingValidationModule(
    "",
    contact="",
    output_file_name="VXDTF1Validation.root",
    expert_level=2)
path.add_module(trackingValidationModule)

process(path)
print(statistics)
