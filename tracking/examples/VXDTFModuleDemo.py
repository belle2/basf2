#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from time import time
from basf2 import *
from beamparameters import add_beamparameters

numEvents = 200
initialValue = 1

usePXD = True
useEvtGen = True
usePGun = True
wantDisplay = False

# flags for the pGun
numTracks = 5
# transverseMomentum:
momentumMin = 0.5  # GeV/c
momentumMax = 0.5  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 15.0  # degrees
thetaMax = 152.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees

if len(argv) > 1:
    numEvents = int(argv[1])
    print('1st argument given, new value for numEvents: ' + str(numEvents))
if len(argv) > 2:
    thetaMin = int(argv[2])
    print('2nd argument given, new value for thetaMin: ' + str(thetaMin))
if len(argv) > 3:
    thetaMax = int(argv[3])
    print('3rd argument given, new value for thetaMax: ' + str(thetaMax))
if len(argv) > 4:
    numTracks = int(argv[4])
    print('4th argument given, new value for numTracks: ' + str(numTracks))

tuneValue = 0.06

# standard secMap until r24443:
# secSetup = [
# 'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-moreThan500MeV_SVD',
# 'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-125to500MeV_SVD',
# 'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-30to125MeV_SVD']
# test geometry for comparison with L3-shifted one:
# secSetup = [
# 'secMapsDec2015SVDStd-moreThan400MeV_SVD',
# 'secMapsDec2015SVDStd-100to400MeV_SVD',
# 'secMapsDec2015SVDStd-25to100MeV_SVD']
# secMap for new shifted geometry:
secSetup = [
    'shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
    'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
    'shiftedL3IssueTestSVDStd-25to100MeV_SVD']
if usePXD:
    # standard secMap until r24443:
    # secSetup = \
    # ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
    # 'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
    # 'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'
    # ]
    # test geometry for comparison with L3-shifted one:
    # secSetup = \
    # ['secMapsDec2015VXDStd-moreThan400MeV_PXDSVD',
    # 'secMapsDec2015VXDStd-100to400MeV_PXDSVD',
    # 'secMapsDec2015VXDStd-25to100MeV_PXDSVD'
    # ]
    # secMap for new shifted geometry:
    secSetup = \
        ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
         'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
         'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
         ]
    tuneValue = 0.22

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')

pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

particlegun = register_module('ParticleGun')
param_pGun = {  # 13: muons, 211: charged pions
                # fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
    'pdgCodes': [13, -13],
    'nTracks': numTracks,
    'momentumGeneration': 'uniformPt',
    'momentumParams': [momentumMin, momentumMax],
    'thetaGeneration': 'uniform',
    'thetaParams': [thetaMin, thetaMax],
    'phiGeneration': 'uniform',
    'phiParams': [phiMin, phiMax],
    'vertexGeneration': 'uniform',
    'xVertexParams': [-0.01, 0.01],
    'yVertexParams': [-0.01, 0.01],
    'zVertexParams': [-0.5, 0.5],
}
particlegun.param(param_pGun)

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               'GFTrackCandidatesColName': 'caTracks',
               'tuneCutoffs': tuneValue,
               'displayCollector': 2
               # 'calcQIType': 'kalman'
               }

vxdtf.param(param_vxdtf)


doPXD = 0
if usePXD:
    doPXD = 1
track_finder_mc_truth = register_module('TrackFinderMCTruth')
track_finder_mc_truth.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
param_track_finder_mc_truth = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': doPXD,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

setupGenfit = register_module('SetupGenfitExtrapolation')
trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 11
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)


if wantDisplay:
    display = register_module('Display')
    display.param('options', 'HTMS')  # default
    # display.param('assignHitsToPrimaries', 0)
    display.param('showAllPrimaries', False)
    # display.param('showCharged', False)
    display.param('showTrackLevelObjects', True)
    display.param('automatic', False)
    display.param('useClusters', True)
    display.param('fullGeometry', True)
    display.param('showTrackCandidates', True)
    # display.param('GFTrackCandidatesColName', 'caTracksJKL')

log_to_file('VXDTFModuleDemoOutput.txt', append=False)

# Create paths
main = create_path()


# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)


# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
if useEvtGen:
    # following modules only for evtGen:
    main.add_module(evtgeninput)
    if usePGun:
        main.add_module(particlegun)
else:
    # following modules only for pGun:
    main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(eventCounter)
main.add_module(vxdtf)
main.add_module(track_finder_mc_truth)
main.add_module(analyzer)
main.add_module(trackfitter)
if wantDisplay:
    main.add_module(display)
# Process events
process(main)

print('Event Statistics :')
print(statistics)
