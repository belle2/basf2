#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will simulate several testbeam events, reconstruct
# and fit tracks, and display each event.
import os
from basf2 import *
from subprocess import call
import datetime

### setup of the most important parts for the VXDTF ###
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
filterOverlaps = 'hopfield'
seed = 1  # 1, 5, 6
numEvents = 250
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  ## degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

set_log_level(LogLevel.ERROR)
set_random_seed(seed)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])
eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName',
              'testbeam/vxd/FullTelescopeVXDTB_v2_NOTAlignedAtAll.xml')

geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes the example much faster
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
    # secSetup = \
        # 'TB3GeVFullMagnetNoAlignedSource2014May22SVD-moreThan1500MeV_SVD'
    secSetup = 'testBeamMini6GeVJune08MagnetOnSVD-moreThan1500MeV_SVD'
    qiType = 'circleFit'  # circleFit
else:
  # To turn off magnetic field:
    geometry.param('components', ['TB'])
    # secSetup = 'TB4GeVNoMagnetNoAlignedSource2014May21SVD-moreThan1500MeV_SVD'
    secSetup = 'testBeamMini6GeVJune08MagnetOffSVD-moreThan1500MeV_SVD'
    qiType = 'straightLine'  # straightLine

particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 1)
# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)

g4sim = register_module('FullSim')
# this is needed for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', True)

SVDDIGI = register_module('SVDDigitizer')
SVDDIGI.param('PoissonSmearing', True)
SVDDIGI.param('ElectronicEffects', True)

SVDCLUST = register_module('SVDClusterizer')
if fieldOn:
    SVDCLUST.param('TanLorentz_holes', 0.)  # 0.052
    SVDCLUST.param('TanLorentz_electrons', 0.)
else:
    SVDCLUST.param('TanLorentz_holes', 0.)  # value scaled from 0.08 for 1.5T to 0.975T
    SVDCLUST.param('TanLorentz_electrons', 0.)

PXDDIGI = register_module('PXDDigitizer')
PXDDIGI.param('SimpleDriftModel', False)
PXDDIGI.param('PoissonSmearing', True)
PXDDIGI.param('ElectronicEffects', True)
if fieldOn:
    PXDDIGI.param('tanLorentz', 0.1625)
else:
    PXDDIGI.param('tanLorentz', 0.)  # value scaled from 0.25 for 1.5T to 0.975T

PXDCLUST = register_module('PXDClusterizer')
if fieldOn:
    PXDCLUST.param('TanLorentz', 0.1625)
else:
    PXDCLUST.param('TanLorentz', 0.)  # value scaled from 0.25 for 1.5T to 0.975T

tf = register_module('BaseLineTF')
tf.logging.log_level = LogLevel.DEBUG
tf.logging.debug_level = 2
# calcQIType:
# Supports 'kalman', 'circleFit' or 'trackLength.
# 'circleFit' has best performance at the moment

# filterOverlappingTCs:
# Supports 'hopfield', 'greedy' or 'none'.
# 'hopfield' has best performance at the moment
param_tf = {  # normally we don't know the particleID, but in the case of the testbeam,
              # we can expect (anti-?)electrons...
    'standardPdgCode': -11,
    'artificialMomentum': 3,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
    'tuneCutoffs': 0.51,
    'activateZigZagXY': False,
    'activateDeltaPt': False,
    'activateCircleFit': False,
    }
tf.param(param_tf)

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')
vxdtf_dqm.param('GFTrackCandidatesColName', 'caTracks')

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 1
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)

mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 5,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
mctrackfinder.param(param_mctrackfinder)

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.FATAL
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('StoreFailedTracks', True)
# trackfitter.param('FilterId', 'simpleKalman')
trackfitter.param('UseClusters', True)

trackfit_dqm = register_module('TrackfitDQM')
trackfit_dqm.param('GFTrackCandidatesColName', 'caTracks')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

# Create paths
main = create_path()
histo = register_module('HistoManager')
histo.param('histoFileName', 'vxdtfTBdemoHist.root')  # File to save histograms
main.add_module(histo)
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
main.add_module(tf)
main.add_module(mctrackfinder)
main.add_module(analyzer)
main.add_module(trackfitter)
main.add_module(eventCounter)
main.add_module(vxdtf_dqm)
main.add_module(trackfit_dqm)
# Process events
process(main)

print statistics
