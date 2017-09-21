#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# steering script for generating data to investigate the phase space of genfit::TrackCands
import os
from basf2 import *
from sys import argv

numEvents = 250

# some flags for easier access up here, than searching them below
useEvtGen = True
usePGun = False

# flag for creating two Cluster and single Cluster SVD SpacePoints
create2ClusterSP = True
createSingleClusterSP = False

# flags for GFTC2SPTC
checkTH = True
useSingle = False
checkNoSingle = True

# some sanity checks on the flags
if not createSingleClusterSP:
    # makes no sense to check for StoreArray of single Cluster SVD SpacePoints if they are not created!!
    useSingle = False
if not create2ClusterSP:
    # makes no sense to check for StoreArray if they do not get created
    checkNoSingle = False

# setting debuglevels for more than one module at once (currently set for the PhaseSpaceAnalysis module)
MyLogLevel = LogLevel.INFO
MyDebugLevel = 500

# params for the pGun
numTracks = 5
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

initialValue = 0  # 0 for random events

if len(argv) is 1:
    print('no arguments given, using standard values')
if len(argv) is 2:
    initialValue = int(argv[1])
    print('using input value ' + str(initialValue) + ' as initialValue')

# set up basic root file name from set parameters
rootBasicName = ''
if useEvtGen:
    rootBasicName = 'EvtGen'
if usePGun:
    rootBasicName = rootBasicName + 'PGun_theta_' + str(int(thetaMin)) + '_' \
        + str(int(thetaMax))

rootBasicName = rootBasicName + '_TH_' + str(checkTH) + '_'

if create2ClusterSP and createSingleClusterSP:
    rootBasicName = rootBasicName + 'All'
elif create2ClusterSP:
    rootBasicName = rootBasicName + 'NoSingle'
elif createSingleClusterSP:
    rootBasicName = rootBasicName + 'OnlySingle'

# rootBasicName = rootBasicName + '_small' # for small sample

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
trackCandConverter.logging.log_level = LogLevel.INFO
# trackCandConverter.logging.debug_level = MyOtherDebugLevel
param_trackCandConverter = {
    # 'PXDClusters': 'myPXDClusters',
    # 'SVDClusters': 'mySVDClusters',
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'SingleClusterSVDSP': 'singleSP',
    'PXDClusterSP': 'pxdOnly',
    # set checkTrueHits to true if TrueHits should be checked (CAUTION: no analysis on MisMatch TrueHits in CurlingTrackCandSplitter
    # possible if set to true)
    'checkTrueHits': checkTH,
    # set useSingleClusterSP to false if exception should be thrown when no doubleCluster SpacePoint can be found for SVD Clusters
    'useSingleClusterSP': useSingle,
    'checkNoSingleSVDSP': checkNoSingle,
}
trackCandConverter.param(param_trackCandConverter)

# TCConverter, SPTC -> genfit
btrackCandConverter = register_module('SPTC2GFTCConverter')
btrackCandConverter.logging.log_level = LogLevel.INFO
# btrackCandConverter.logging.debug_level = MyDebugLevel
param_btrackCandConverter = {'SpacePointTCName': 'SPTracks',
                             'genfitTCName': 'myMCTracks'}
btrackCandConverter.param(param_btrackCandConverter)

rootPhaseSpace = 'PhaseSpaceAnalysis_' + rootBasicName

# phase space analysis of TrackCands after conversion and back conversion to/from SPTC
phaseSpaceAnalysis = register_module('PhaseSpaceAnalysis')
phaseSpaceAnalysis.logging.log_level = MyLogLevel
phaseSpaceAnalysis.logging.debug_level = MyDebugLevel
param_phaseSpaceAnalysis = {'GFTCname': ['myMCTracks', ''],
                            'rootFileName': [rootPhaseSpace + '_converted',
                                             'RECREATE']}
phaseSpaceAnalysis.param(param_phaseSpaceAnalysis)

mcPhaseSpaceAnalysis = register_module('PhaseSpaceAnalysis')
mcPhaseSpaceAnalysis.logging.log_level = MyLogLevel
mcPhaseSpaceAnalysis.logging.debug_level = MyDebugLevel
param_mcPhaseSpaceAnalysis = {'GFTCname': ['mcTracks', ''],
                              'rootFileName': [rootPhaseSpace + '_mc',
                                               'RECREATE']}
mcPhaseSpaceAnalysis.param(param_mcPhaseSpaceAnalysis)

# only missing TCs
diffPhaseSpaceAnalysis = register_module('PhaseSpaceAnalysis')
diffPhaseSpaceAnalysis.logging.log_level = MyLogLevel
diffPhaseSpaceAnalysis.logging.debug_level = MyDebugLevel
param_diffPhaseSpaceAnalysis = {'GFTCname': ['mcTracks', 'myMCTracks'],
                                'rootFileName': [rootPhaseSpace + '_diff',
                                                 'RECREATE']}
diffPhaseSpaceAnalysis.param(param_diffPhaseSpaceAnalysis)

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
main.add_module(mcTrackFinder)

main.add_module(spCreatorPXD)
if create2ClusterSP:
    # only create these if flag is set
    main.add_module(spCreatorSVD)
if createSingleClusterSP:
    # only create these if flag is set
    main.add_module(spCreatorSVDsingle)

main.add_module(trackCandConverter)
main.add_module(btrackCandConverter)

main.add_module(phaseSpaceAnalysis)
main.add_module(mcPhaseSpaceAnalysis)
main.add_module(diffPhaseSpaceAnalysis)

process(main)

print(statistics)
