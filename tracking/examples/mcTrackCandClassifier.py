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
# evtgeninput.param('userDECFile','tracking/examples/dec_traceable.dec')
# DECFile and pdlFile have sane defaults

pGun = register_module('ParticleGun')
pGun.param('momentumParams', [1.1, 1.11])
pGun.param('phiParams', [210, 210.1])
# pGun.param('phiParams',[0,180])
pGun.param('thetaParams', [90, 90.1])
# pGun.param('thetaParams',[30,160])
# pGun.param('pdgCodes',[-2212]) #proton
pGun.param('pdgCodes', [-211])

# find MCTracks
track_finder_mc_truth = register_module('TrackFinderMCTruth')
param_track_finder_mc_truth = {
    'EnergyCut': 0,
    'UseCDCHits': False,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    #    'TrueHitMustExist': True,
    'UseClusters': False,
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'MCFittingEvtGenOutput.root')
output.logging.log_level = LogLevel.DEBUG

# MCTrackCandClassifier
MCTrackCandClassifier = register_module('MCTrackCandClassifier')
MCTrackCandClassifier.logging.log_level = LogLevel.DEBUG
MCTrackCandClassifier.param('rootFileName', "MCTrackCandClassifier.root")
MCTrackCandClassifier.param('usePXD', True)
MCTrackCandClassifier.param('nSigma_dR', 5)
MCTrackCandClassifier.param('lapFraction', 1)
MCTrackCandClassifier.param('isInAnnulusCriterium', True)  # False reproduce TrackCandMCTruth
MCTrackCandClassifier.param('isInSemiplaneCriterium', True)  # False reproduce TrackCandMCTruth
MCTrackCandClassifier.param('isInFirstLapCriterium', True)  # False reproduce TrackCandMCTruth
MCTrackCandClassifier.param('mustHaveCluster', True)  # False reproduce TrackCandMCTruth
MCTrackCandClassifier.param('minNhits', 5)  # 2 to reproduce TrackCandMCTruth

# display
display = register_module('Display')
display.param('showCharged', True)
display.param('showAllPrimaries', False)

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
add_beamparameters(main, "Y4S")
main.add_module(evtgeninput)
# main.add_module(pGun)
add_simulation(main)
main.add_module(track_finder_mc_truth)
main.add_module(MCTrackCandClassifier)
# main.add_module(display)
# main.add_module(output)

# Process events
process(main)
print(statistics)
