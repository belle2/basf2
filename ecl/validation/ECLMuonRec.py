#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

set_log_level(LogLevel.ERROR)

# Register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

# Create geometry
# Geometry parameter loader
gearbox = register_module('Gearbox')

# Geometry builder
geometry = register_module('Geometry')

# Simulation
g4sim = register_module('FullSim')

# Set the number of events to be processed (1000 events)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

import random
intseed = random.randint(1, 10000000)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [13],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [1., 1.],
    'thetaGeneration': 'uniform',
    'thetaParams': [50., 130.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)

eclDigi = register_module('ECLDigitizer')
eclRecShower = register_module('ECLReconstructor')
makeGamma = register_module('ECLGammaReconstructor')
makePi0 = register_module('ECLPi0Reconstructor')
makeMatch = register_module('ECLMCMatching')
param_Gamma = {
    'gammaEnergyCut': 0.02,
    'gammaE9o25Cut': 0.75,
    'gammaWidthCut': 6.0,
    'gammaNhitsCut': 0,
    }

makeGamma.param(param_Gamma)

cdcDigitizer = register_module('CDCDigitizer')
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

ext = register_module('Ext')

genfit = register_module('GenFitter')
mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    }
mctrackfinder.param(param_mctrackfinder)

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('NIterations', 3)

trackfitchecker = register_module('TrackFitChecker')
trackfitchecker.logging.log_level = LogLevel.INFO
trackfitchecker.param('testSi', True)
trackfitchecker.param('testCdc', False)
trackfitchecker.param('writeToTextFile', True)

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
# main.add_module(trackfitchecker)
main.add_module(ext)
main.add_module(eclDigi)
main.add_module(eclRecShower)
main.add_module(makeGamma)
main.add_module(makePi0)
main.add_module(makeMatch)
simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', '../ECLMuonOutput.root')
main.add_module(simpleoutput)

process(main)
print statistics
