#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file which shows all usage options for the particle gun module
# in the generators package. The generated particles from the particle gun are
# then fed through a full Geant4 simulation and the output is stored in a root
# file.
#
# The different options for the particle gun are explained below.
# Uncomment/comment different lines to get the wanted settings
#
# Example steering file - 2012 Belle II Collaboration
##############################################################################

##############################################################################

# select if partial or full test
full = False

# select which subdetectors to test
pxd = True
svd = True
cdc = True

# select fitter (simpleKalman, Kalman, DAF)
FilterId = 'Kalman'

# visualize failed tracks?
vis = False

##############################################################################

import inspect
import ROOT
import sys
import shutil
from ROOT import std
from basf2 import *

if not vis:
    ROOT.gROOT.ProcessLine('gROOT.SetBatch()')

# generate filename
print 'use the following detectors: '
rootFileName = '../genfit_'
if pxd:
    rootFileName += 'pxd'
    print 'pxd'
if svd:
    rootFileName += 'svd'
    print 'svd'
if cdc:
    rootFileName += 'cdc'
    print 'cdc'
    rootFileName += '_'
    rootFileName += FilterId
    print 'fit with ' + FilterId
if full:
    rootFileName += '_FULL'
    nOfEvents = 100000
    print 'do FULL test'
else:
    nOfEvents = 1000
    print 'do partial test'

logFileName = rootFileName + '.log'

sys.stdout = open(logFileName, 'w')

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# set fixed random seed to make results comparable between different revisions
set_random_seed(1)

# to run the framework the used modules need to be registered
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'uniformPt')
if full:
    particlegun.param('momentumParams', [0.2, 3.0])
else:
    particlegun.param('momentumParams', [0.9, 0.9])
particlegun.param('thetaGeneration', 'uniformCosinus')
if full:
    particlegun.param('thetaParams', [17, 150])
else:
    particlegun.param('thetaParams', [100, 100])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('independentVertices', False)
particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [0, 0])
particlegun.param('yVertexParams', [0, 0])
particlegun.param('zVertexParams', [-0.01, 0.01])
print_params(particlegun)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
# Show progress of processing
progress = register_module('Progress')
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
componentList = ['BeamPipe']
if pxd:
    componentList.append('PXD')
if svd:
    componentList.append('SVD')
if cdc:
    componentList.append('CDC')
    componentList.append('MagneticField4LimitedRCDC')
else:
    componentList.append('MagneticField4LimitedRSVD')

geometry.param('Components', componentList)

# Run simulation
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', True)

# ---------------------------------------------------------------
# digitizer
if pxd:
    pxdDigitizer = register_module('PXDDigitizer')
    pxdClusterizer = register_module('PXDClusterizer')
if svd:
    svdDigitizer = register_module('SVDDigitizer')
    svdClusterizer = register_module('SVDClusterizer')
if cdc:
    cdcDigitizer = register_module('CDCDigitizer')

mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.param('UsePXDHits', pxd)
mctrackfinder.param('UseSVDHits', svd)
mctrackfinder.param('UseCDCHits', cdc)

# ---------------------------------------------------------------
# fitting
fitter = register_module('GenFitter')
param_fitter = {
    'StoreFailedTracks': True,
    'FilterId': FilterId,
    'NMinIterations': 3,
    'NMaxIterations': 10,
    'ProbCut': 0.001,
    }
    # 'MSCModel': "GEANE",
    # 'noEffects' : False,
    # 'energyLossBetheBloch' : True,
    # 'noiseBetheBloch' : False,
    # 'noiseCoulomb' : False,
    # 'energyLossBrems' : False,
    # 'noiseBrems' : False,
    # 'GFTracksColName': 'GF2Tracks',

fitter.param(param_fitter)

# ---------------------------------------------------------------
# Setting the option for all non particle gun modules:
# want to process 100 MC events
eventinfosetter.param({'evtNumList': [nOfEvents], 'runList': [1]})

trackfitchecker = register_module('TrackFitChecker')
# trackfitchecker.logging.log_level = LogLevel.INFO  # the reults of the statistical tests will only show up at info or debug level
trackfitchecker.param('robustTests', True)
trackfitchecker.param('writeToTextFile', True)
trackfitchecker.param('writeToRootFile', True)
# trackfitchecker.param('testSi', True)
# trackfitchecker.param('inspectTracks', False)
trackfitchecker.param('truthAvailable', True)
trackfitchecker.param('outputFileName', rootFileName)

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
if pxd:
    main.add_module(pxdDigitizer)
    main.add_module(pxdClusterizer)
if svd:
    main.add_module(svdDigitizer)
    main.add_module(svdClusterizer)
if cdc:
    main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(fitter)
main.add_module(trackfitchecker)

if vis:
    visualization = register_module('GenfitVis')
    # visualization.param('onlyBadTracks', 1)
    main.add_module(visualization)

output = register_module('RootOutput')
output.param('outputFileName', 'GF2Tracks.root')
# main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics

