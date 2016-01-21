#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 25
initialValue = 1

usePGun = True

# flags for the pGun
numTracks = 2
# transverseMomentum:
momentumMin = 0.130  # GeV/c
momentumMax = 0.260  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 20.0  # degrees
thetaMax = 145.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees

# flags for GFTC2SPTC
checkTH = False

# flags for SPTCNetworkProducer
# debugLevel = LogLevel.DEBUG
debugLevel1 = LogLevel.INFO
debugLevel2 = LogLevel.INFO
dLevel1 = 1
dLevel2 = 101
checkSPs = False

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


secSetup = ['shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD']
tuneValue = 0.22

set_log_level(LogLevel.WARNING)
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
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD', 'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)


vxdtf = register_module('TFRedesign')
vxdtf.logging.log_level = LogLevel.INFO
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               'GFTrackCandidatesColName': 'caTracks',
               'tuneCutoffs': tuneValue,
               'filterOverlappingTCs': 'none',  # shall provide overlapping TCs
               'useTimeSeedAsQI': True  # hack for storing QIs in TimeSeed-Variable for genfit::TrackCand
               }

vxdtf.param(param_vxdtf)

refTrackFinder = register_module('TrackFinderMCTruth')
refTrackFinder.logging.log_level = LogLevel.INFO
# select which detectors you would like to use
param_refTrackFinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
refTrackFinder.param(param_refTrackFinder)

# double clusters
spCreatorSVD = register_module('SpacePointCreatorSVD')
spCreatorSVD.logging.log_level = LogLevel.INFO
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)


spCreatorPXD = register_module('SpacePointCreatorPXD')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
spCreatorPXD.param('SpacePoints', 'pxdOnly')

sp2thConnector = register_module('SpacePoint2TrueHitConnector')
sp2thConnector.logging.log_level = LogLevel.WARNING
param_sp2thConnector = {
    'DetectorTypes': ['PXD', 'SVD'],
    'TrueHitNames': ['', ''],
    'ClusterNames': ['', ''],
    'SpacePointNames': ['pxdOnly', 'nosingleSP'],
    'outputSuffix': '_relTH',
    'storeSeperate': False,
    'registerAll': True,
    'maxGlobalPosDiff': 0.05,
    'maxPosSigma': 5,
    'minWeight': 0,
    'requirePrimary': True,
    'positionAnalysis': False,
}
sp2thConnector.param(param_sp2thConnector)

# TCConverter, genfit -> SPTC
trackCandConverter = register_module('GFTC2SPTCConverter')
trackCandConverter.logging.log_level = LogLevel.INFO
param_trackCandConverter = {
    'genfitTCName': 'caTracks',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'PXDClusterSP': 'pxdOnly',
    'checkTrueHits': checkTH,
    'useSingleClusterSP': False,
    'checkNoSingleSVDSP': True,
}
trackCandConverter.param(param_trackCandConverter)


trackCandConverterReference = register_module('GFTC2SPTCConverter')
trackCandConverterReference.logging.log_level = LogLevel.INFO
param_trackCandConverterReference = {
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracksReference',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'PXDClusterSP': 'pxdOnly',
    'checkTrueHits': checkTH,
    'useSingleClusterSP': False,
    'checkNoSingleSVDSP': True,
}
trackCandConverterReference.param(param_trackCandConverterReference)


qualiEstimatorRandom = register_module('QualityEstimatorVXDRandom')
qualiEstimatorRandom.logging.log_level = LogLevel.INFO
qualiEstimatorRandom.param('tcArrayName', 'SPTracks')
qualiEstimatorRandom.param('useTimeSeedAsQI', True)  # hack for storing QIs in TimeSeed-Variable for genfit::TrackCand

tcNetworkProducer = register_module('SPTCNetworkProducer')
tcNetworkProducer.logging.log_level = debugLevel1
tcNetworkProducer.logging.debug_level = dLevel1
tcNetworkProducer.param('tcArrayName', 'SPTracks')
tcNetworkProducer.param('tcNetworkName', 'tcNetwork')
tcNetworkProducer.param('checkSPsInsteadOfClusters', checkSPs)

trackSetEval = register_module('TrackSetEvaluatorHopfieldNN')
trackSetEval.logging.log_level = debugLevel1
trackSetEval.logging.debug_level = dLevel1
trackSetEval.param('tcArrayName', 'SPTracks')
trackSetEval.param('tcNetworkName', 'tcNetwork')


analizer = register_module('TrackFinderVXDAnalizer')
analizer.logging.log_level = debugLevel2
analizer.logging.debug_level = dLevel2
analizer.param('testTCname', 'SPTracks')
analizer.param('referenceTCname', 'SPTracksReference')
analizer.param('writeToRoot', True)

# analizer.param('trackedParametersDouble', [
#    [["Perfect"], ["AnalyzingAlgorithmResidualP", "AnalyzingAlgorithmResidualPosition", "AnalyzingAlgorithmValuePT"]]
#    ])


log_to_file('sptcNetworkDemoOutput.txt', append=False)

# Create paths
main = create_path()
# Add modules to paths

main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(particlegun)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(eventCounter)
main.add_module(vxdtf)
main.add_module(refTrackFinder)

main.add_module(spCreatorPXD)
main.add_module(spCreatorSVD)
main.add_module(sp2thConnector)
main.add_module(trackCandConverter)
main.add_module(trackCandConverterReference)
main.add_module(qualiEstimatorRandom)
main.add_module(tcNetworkProducer)
main.add_module(trackSetEval)
main.add_module(analizer)

# Process events
process(main)

print('Event Statistics :')
print(statistics)
