#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
##############################################################################

import os
from basf2 import *
from beamparameters import add_beamparameters
from simulation import add_simulation

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [1])
eventinfoprinter = register_module('EventInfoPrinter')

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

# find MCTracks
track_finder_mc_truth = register_module('TrackFinderMCTruth')
param_track_finder_mc_truth = {
    'EnergyCut': 0,
    'UseCDCHits': False,
    'UseSVDHits': True,
    'UsePXDHits': True,
    'WhichParticles': ['PXD', 'SVD'],
    'UseClusters': True,  # MUST BE TRUE
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

# MCTrackCandClassifier
MCTrackCandClassifier = register_module('MCTrackCandClassifier')
MCTrackCandClassifier.logging.log_level = LogLevel.INFO
MCTrackCandClassifier.param('rootFileName', "test_MCTrackCandClassifier.root")
MCTrackCandClassifier.param('usePXD', True)
MCTrackCandClassifier.param('nSigma_dR', 3)
MCTrackCandClassifier.param('lapFraction', 0.9)
MCTrackCandClassifier.param('isInAnnulusCriterium', True)
MCTrackCandClassifier.param('isInSemiplaneCriterium', True)
MCTrackCandClassifier.param('isInFirstLapCriterium', True)
MCTrackCandClassifier.param('mustHaveCluster', True)
MCTrackCandClassifier.param('minNhits', 5)

# display
display = register_module('Display')
display.param('showCharged', True)
display.param('showAllPrimaries', False)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'MCTrackCandClassifierOutput_RootOutput.root')
output.logging.log_level = LogLevel.INFO

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
add_beamparameters(main, "Y4S")
main.add_module(evtgeninput)
add_simulation(main)
main.add_module(track_finder_mc_truth)
main.add_module(MCTrackCandClassifier)
# main.add_module(display)
# main.add_module(output)

# Process events
process(main)
print(statistics)
