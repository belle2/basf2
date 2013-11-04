#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will simulate several testbeam events, reconstruct
# and fit tracks, and display each event.
import os
from basf2 import *
from subprocess import call
import datetime

### setup of the most important parts for the VXDTF ###
secSetup = ['testBeamFINE_VXD']  # use 'testBeamFINE_SVD' for svd-only
# or 'testBeamFINE_VXD' for full vxd reco.
# and don't forget to set the clusters for the detector type you want in the
# mcTrackFinder down below!
qiType = 'circleFit'
filterOverlaps = 'hopfield'
seed = 1
numEvents = 250


set_log_level(LogLevel.ERROR)
set_random_seed(seed)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName', 'testbeam/vxd/VXD.xml')

geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes the example much faster
geometry.param('Components', ['MagneticField', 'TB'])

# ParticleGun
particlegun = register_module('ParticleGun')

# number of primaries per event
particlegun.param('nTracks', 1)

# DESY electrons:
particlegun.param('pdgCodes', [-11])
# momentum magnitude 2 - 6 GeV/c
# Beam divergence 2mrad not covered yet (we need some starting point location)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [2.0, 0.])
# momentum direction
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [90.0, 0.])
particlegun.param('phiGeneration', 'fixed')
particlegun.param('phiParams', [0., 0.])
# gun displacement
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [-90.0, 0.])
particlegun.param('yVertexParams', [1.0, 0.])
particlegun.param('zVertexParams', [0., 0.])
particlegun.param('independentVertices', True)

g4sim = register_module('FullSim')
# this is needed for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', True)
SVDDIGI = register_module('SVDDigitizer')
SVDDIGI.param('PoissonSmearing', True)
SVDDIGI.param('ElectronicEffects', True)

SVDCLUST = register_module('SVDClusterizer')

PXDDIGI = register_module('PXDDigitizer')
PXDDIGI.param('SimpleDriftModel', False)
PXDDIGI.param('PoissonSmearing', True)
PXDDIGI.param('ElectronicEffects', True)

PXDCLUST = register_module('PXDClusterizer')

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
# calcQIType:
# Supports 'kalman', 'circleFit' or 'trackLength.
# 'circleFit' has best performance at the moment

# filterOverlappingTCs:
# Supports 'hopfield', 'greedy' or 'none'.
# 'hopfield' has best performance at the moment
param_vxdtf = {
    # normally we don't know the particleID, but in the case of the testbeam,
    # we can expect (anti-?)electrons...
    'activateBaselineTF': 1,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'standardPdgCode': -11,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
    'activateDistanceXY': [False],
    'activateAngles3DHioC': [False],
    'activateAnglesXYHioC': [False],
    'activateDeltaSlopeRZHioC': [False],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [False],
    'activateHelixFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateAnglesXY': [False],
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [False],
    'activateDistance2IP': [False],
    'activatePT': [False],
    'activateHelixFit': [False],
    'activateZigZagXY': [False],
    'activateDeltaPt': [False],
    'activateCircleFit': [True],
    'tuneCircleFit': [0.00001],
    }
vxdtf.param(param_vxdtf)


analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 11
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)

mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 5,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
mctrackfinder.param(param_mctrackfinder)

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'acceptedVXDTFTracks')
trackfitter.param('UseClusters', True)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)


# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(SVDDIGI)
main.add_module(SVDCLUST)
main.add_module(vxdtf)
main.add_module(mctrackfinder)
main.add_module(analyzer)
main.add_module(trackfitter)
main.add_module(eventCounter)

# Process events
process(main)

print statistics
