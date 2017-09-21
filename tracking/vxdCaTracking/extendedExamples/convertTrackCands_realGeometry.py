#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# example file invoking and testing the ConverterModules that convert genfit::TrackCands to SpacePointTrackCands and vice versa
import os
from basf2 import *
from sys import argv

numEvents = 1

# some flags for easier access up here, than searching them below
useEvtGen = True
usePGun = True

# flag for creating two Cluster and single Cluster SVD SpacePoints
create2ClusterSP = True
createSingleClusterSP = False

# flags for GFTC2SPTC
checkTH = False
useSingle = False
checkNoSingle = True
# flags for CurlingChecker
useNonSingleTH = True

# some sanity checks on the flags
if not createSingleClusterSP:
    # makes no sense to check for StoreArray of single Cluster SVD SpacePoints if they are not created!!
    useSingle = False
if not create2ClusterSP:
    # makes no sense to check for StoreArray if they do not get created
    checkNoSingle = False
if checkTH:
    # makes no sense to use them, as they will no longer be present then !!
    useNonSingleTH = False

# flags for the pGun
numTracks = 1
# transverseMomentum:
momentumMin = 1.  # GeV/c
momentumMax = 1.  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 15.0  # degrees
# thetaMax = 60.  # degrees
# thetaMax = 30.  # only slanted parts
thetaMax = 153.  # Full detector from 150 to 17 (officially), to have some reserves exceeding these two values slightly
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees

# automatically generate root file name from tags above
firstPart = ''
if useEvtGen:
    firstPart = 'EvtGen_'
if usePGun:
    firstPart = firstPart + 'PartGun_' + 'theta_' + str(int(thetaMin)) + '_' \
        + str(int(thetaMax)) + '_'

SPstring = ''
if create2ClusterSP:
    SPstring = '_NoSingles_'
if createSingleClusterSP:
    SPstring = '_OnlySingles_'
if create2ClusterSP and createSingleClusterSP:
    SPstring = '_All_'

posAnalysisRootFileName = firstPart + 'TH_' + str(checkTH) + SPstring \
    + 'allTH_' + str(useNonSingleTH) + '_' + str(int(numEvents / 1000)) + 'k'

# print the root file name
print('Saving to root file-name ' + posAnalysisRootFileName + '.root')

# setting debuglevels for more than one module at once (currently set for the converter modules)
MyLogLevel = LogLevel.INFO
MyDebugLevel = 500

# set for the curling splitter module and GFTC2SPTC
MyOtherLogLevel = LogLevel.INFO
MyOtherDebugLevel = 500

initialValue = 0  # 0 for random events

if len(argv) is 1:
    print('no arguments given, using standard values')
if len(argv) is 2:
    initialValue = int(argv[1])
    print('using input value ' + str(initialValue) + ' as initialValue')

# module registration
set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

progress = register_module('Progress')
progress.param('maxN', 2)

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])  # 'components', ['BeamPipe', 'MagneticFieldConstant4LimitedRCDC',........

# evtgen
evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

# particle gun
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

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # need for MCTrackfinder to work correctly

# digitizers & clusterizers
pxdDigitizer = register_module('PXDDigitizer')

# cant name clusters, because there is no way to pass these names to the TrackFinderMCTruth
pxdClusterizer = register_module('PXDClusterizer')
# pxdClusterizer.param('Clusters','myPXDClusters')

svdDigitizer = register_module('SVDDigitizer')
svdClusterizer = register_module('SVDClusterizer')
# svdClusterizer.param('Clusters','mySVDClusters')

# SpacePoint Creation
# single clusters only
spCreatorSVDsingle = register_module('SVDSpacePointCreator')
spCreatorSVDsingle.logging.log_level = LogLevel.INFO
spCreatorSVDsingle.param('OnlySingleClusterSpacePoints', True)
spCreatorSVDsingle.param('NameOfInstance', 'singleClusters')
spCreatorSVDsingle.param('SpacePoints', 'singleSP')
# spCreatorSVD.param('SVDClusters', 'singleSP')

# double clusters
spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.logging.log_level = LogLevel.INFO
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)

spCreatorPXD = register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
# spCreatorPXD.param('PXDClusters', 'myPXDClusters')
spCreatorPXD.param('SpacePoints', 'pxdOnly')

# MCTrackFinder
mcTrackFinder = register_module('TrackFinderMCTruth')
mcTrackFinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 4,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    'TrueHitMustExist': True,
}
mcTrackFinder.param(param_mctrackfinder)

# TCConverter, genfit -> SPTC
trackCandConverter = register_module('GFTC2SPTCConverter')
trackCandConverter.logging.log_level = MyOtherLogLevel
trackCandConverter.logging.debug_level = MyOtherDebugLevel
param_trackCandConverter = {
    # 'PXDClusters': 'myPXDClusters',
    # 'SVDClusters': 'mySVDClusters',
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'SingleClusterSVDSP': 'singleSP',
    'PXDClusterSP': 'pxdOnly',
    # set checkTrueHits to true if TrueHits should be checked (CAUTION: no analysis on MisMatch
    # TrueHits in CurlingTrackCandSplitter possible if set to true)
    'checkTrueHits': checkTH,
    # set useSingleClusterSP to false if exception should be thrown when no doubleCluster SpacePoint can be found for SVD Clusters
    'useSingleClusterSP': useSingle,
    'checkNoSingleSVDSP': checkNoSingle,
}
trackCandConverter.param(param_trackCandConverter)

# TCConverter, SPTC -> genfit
btrackCandConverter = register_module('SPTC2GFTCConverter')
btrackCandConverter.logging.log_level = MyLogLevel
btrackCandConverter.logging.debug_level = MyDebugLevel
param_btrackCandConverter = {'SpacePointTCName': 'SPTracks',
                             'genfitTCName': 'myMCTracks'}
btrackCandConverter.param(param_btrackCandConverter)

# TCConverterTest
tcConverterTest = register_module('TCConvertersTest')
tcConverterTest.logging.log_level = MyLogLevel
tcConverterTest.logging.debug_level = MyDebugLevel
param_tcConverterTest = {'SpacePointTCName': 'SPTracks',
                         'genfitTCNames': ['mcTracks', 'myMCTracks'],
                         'SpacePointArrayNames': 'pxdOnly'}
tcConverterTest.param(param_tcConverterTest)

curlingSplitter = register_module('CurlingTrackCandSplitter')
curlingSplitter.logging.log_level = MyOtherLogLevel
curlingSplitter.logging.debug_level = MyOtherDebugLevel
param_curlingSplitter = {
    'SpacePointTCName': 'SPTracks',
    'curlingFirstOutName': 'firstOutParts',
    'curlingAllInName': 'allInParts',
    'curlingRestOutName': 'restOutParts',
    'completeCurlerName': 'completeCurler',
    # set splitCurlers to false if you do not want to split curling TrackCandidates but simply analyze them for curling behaviour
    'splitCurlers': True,
    'nTrackStubs': int(0),
    'setOrigin': [0., 0., 0.],
    # set positionAnalysis to true if you want to analyze the position of SpacePoints and the TrueHits they are related to
    'positionAnalysis': True,
    # set useNonSingleTHinPA to False always, at the moment only a testing feature!
    'useNonSingleTHinPA': useNonSingleTH,
    'rootFileName': [posAnalysisRootFileName, 'RECREATE'],
}
curlingSplitter.param(param_curlingSplitter)

# Path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)

if useEvtGen:
    # following modules only for evtGen:
    main.add_module(evtgeninput)
    if usePGun:
        main.add_module(particlegun)
else:
    # following modules only for pGun:
    main.add_module(particlegun)

main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(svdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdClusterizer)
main.add_module(spCreatorPXD)
if create2ClusterSP:
    # only create these if flag is set
    main.add_module(spCreatorSVD)
if createSingleClusterSP:
    # only create these if flag is set
    main.add_module(spCreatorSVDsingle)
main.add_module(mcTrackFinder)

main.add_module(trackCandConverter)
main.add_module(curlingSplitter)  # for easier debug reading in this position
main.add_module(btrackCandConverter)
main.add_module(tcConverterTest)

process(main)

print(statistics)
