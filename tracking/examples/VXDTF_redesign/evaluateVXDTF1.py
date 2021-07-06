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
# VXDTF1 Example Scripts
#
# Evaluation of VXDTF1.
#
# Usage: basf2 evaluateVXDTF1.py -i <inputFileName>
#
#
# Contributors: Jonas Wagner
#####################################################################


import basf2 as b2
import tracking
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule


# ---------------------------------------------------------------------------------------
# Settings
usePXD = False

performFit = False

# Logging and Debug Levels
b2.set_log_level(b2.LogLevel.ERROR)
b2.log_to_file('logVXDTF1Evaluation.log', append=False)


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
                              'SVD',
                              'CDC'])
path.add_module(geometry)

# Event counter
eventCounter = b2.register_module('EventCounter')
path.add_module(eventCounter)

genFitExtrapolation = b2.register_module('SetupGenfitExtrapolation')
path.add_module(genFitExtrapolation)

tracking.add_vxd_track_finding(path, "RecoTracks", components=["SVD"])

if performFit:
    fitter = b2.register_module('DAFRecoFitter')
    path.add_module(fitter)

# Matching
mcTrackMatcherModule = b2.register_module('MCRecoTracksMatcher')
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

b2.process(path)
print(b2.statistics)
