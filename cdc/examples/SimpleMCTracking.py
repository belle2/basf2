#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file creates the Belle II detector geometry, and perfoms the
# simulation and MC based track finding and fitting.
#
# EventInfoSetter and EventInfoPrinter generates and shows event meta data (see example
# in the framework package). Gearbox and Geometry are used to create the Belle2
# detector geometry. The generator used in this example is geant4 particle gun
# (see example in the simulation or generator package). FullSim performs the
# full simulation.
#
# CDCDigitizer creates the detector response in the CDC for the simulated Hits.
# For the PXD and SVD currently the TrueHits are used (created directly by the
# sensitive detector), will be replaced by realistic clusters later on.
#
# MCTrackFinder creates relations between MCParticles and CDCHits/PXDTrueHits
# /SVDTrueHits produced by it. GenFitter fits the found MCTracks and created
# two track collections: GFTracks (Genfit class) and Tracks (class with helix
# parametrization)
#
# For details about module parameters just type > basf2 -m .
#
##############################################################################
##

import os
from basf2 import *

set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
set_random_seed(1028307)

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [1])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate CDC only; B-field outside CDC = 0
geometry.param('components', ['MagneticFieldConstant4LimitedRCDC', 'CDC'])

# particle gun to shoot particles in the detector
pGun = register_module('ParticleGun')

# choose the particles you want to simulate
param_pGun = {
    'pdgCodes': [211],
    'nTracks': 1,
    'varyNTracks': 0,
    'momentumGeneration': 'uniform',
    'momentumParams': [1.0, 1.0],
    'thetaGeneration': 'uniform',
    #    'thetaGeneration': 'uniformCos',
    'thetaParams': [90., 90.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)

# simulation
g4sim = register_module('FullSim')
# this is needed for the MCTrackFinder to work correctly <- obsolete ?
g4sim.param('StoreAllSecondaries', True)
# g4sim.param('SecondariesEnergyCut', 0.0)
g4sim.param('UICommands', ['/control/execute interactions.mac'])
# suppress delta-rays
g4sim.param('ProductionCut', 1000000.)

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MCTracks
# mctrackfinder = register_module('MCTrackFinder')
# mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder = register_module('TrackFinderMCTruthRecoTracks')

# select which detectors you would like to use
param_mctrackfinder = {  # select which particles to use: primary particles
    'UseCDCHits': 1,
    'UseSVDHits': 0,
    'UsePXDHits': 0,
    #    'UseClusters': False,
    'WhichParticles': ['primary'],
    #    'WhichParticles': ['CDC'],
}
mctrackfinder.param(param_mctrackfinder)

# setupgf
setupgf = register_module('SetupGenfitExtrapolation')
# param_setupgf = {}
# setupgf.param(param_setupgf)

# fitting
cdcfitting = register_module('DAFRecoFitter')
# param_cdcfitting = {}
# cdcfitting.param(param_cdcfitting)

# build track
buildtrack = register_module("TrackCreator")

# output
output = register_module('RootOutput')
output.param('outputFileName', 'SimpleMCTrackingOutput.root')

# create path
main = create_path()

# add modules to path
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)

main.add_module(mctrackfinder)
main.add_module(setupgf)
main.add_module(cdcfitting)
main.add_module(buildtrack)

main.add_module(output)

# Process events
process(main)
print(statistics)
