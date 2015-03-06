#!/usr/bin/env python
# -*- coding: utf-8 -*-

# example file showing how to use the different modules to convert genfit::TrackCands from the TrackFinderMCTruth to SpacePointTrackCands
# also shown in this example file: how to use some other modules (e.g. the SPTCReferee) to get 'clean' (i.e. fullfill some defined criteria) SpacePointTrackCands
# NOTE: you might want to twist some of these pre-defined parameters to fit your purposes!
# NOTE 2: the modules used here are currently under some redesign process, this example file is likely to change frequently
# WARNING: something is apparently wrong with the CurlingTrackCandSplitter -> will not be fixed as it will be merged into the SPTCReferee!

import os
from basf2 import *
from sys import argv

numEvents = 100

# want EvtGen or ParticleGun? (or both)
useEvtGen = True
usePGun = False

# flags for the pGun
numTracks = 2
# transverseMomentum:
momentumMin = .01  # GeV/c
momentumMax = .2  #
# theta: starting angle of particle direction in r-z-plane
thetaMin = 15.0  # degrees
# thetaMax = 60.  # degrees
# thetaMax = 30.  # only slanted parts
thetaMax = 153.  # Full detector from 150 to 17 (officially), to have some reserves exceeding these two values slightly
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees

# change the debug level for more than one module (currently set for: SpacePoint2TrueHitConnector, GFTC2SPTCConverter )
MyLogLevel = LogLevel.INFO
MyDebugLevel = 2  # some of the modules print some additional information in terminate if debug level is set to 1 or 2

# possibility of using another initial Value from the command line
initialValue = 0  # 0 for random events
if len(argv) is 1:
    print 'no arguments given, using standard values'
if len(argv) is 2:
    initialValue = int(argv[1])
    print 'using input value ' + str(initialValue) + ' as initialValue'

########################################################################################################################################################
# module registration
set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

# modules needed for the simulation (without much comments)
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

progress = register_module('Progress')
progress.param('maxN', 2)

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRCDC',
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
    'xVertexParams': [-.01, .01],
    'yVertexParams': [-.01, .01],
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

# SpacePoint Creation (create only SpacePoints with a U & a V cluster)
spCreatorSVD = register_module('SpacePointCreatorSVD')
spCreatorSVD.logging.log_level = LogLevel.INFO
spCreatorSVD.param('OnlySingleClusterSpacePoints', False)
spCreatorSVD.param('NameOfInstance', 'nosingleSP')
spCreatorSVD.param('SpacePoints', 'nosingleSP')
# spCreatorSVD.param('SVDClusters', 'mySVDClusters')

# SpacePoint Creation (singleCluster SpacePoints)
singleSpCreatorSVD = register_module('SpacePointCreatorSVD')
singleSpCreatorSVD.logging.log_level = LogLevel.INFO
singleSpCreatorSVD.param('OnlySingleClusterSpacePoints', True)
singleSpCreatorSVD.param('NameOfInstance', 'singleSP')
singleSpCreatorSVD.param('SpacePoints', 'singleSP')
# spCreatorSVD.param('SVDClusters', 'mySVDClusters')

spCreatorPXD = register_module('SpacePointCreatorPXD')
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
    'MinimalNDF': 1,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    'TrueHitMustExist': True,
    }
mcTrackFinder.param(param_mctrackfinder)

##################################################################### Converting and Referee ##############################################
# module to create relations between SpacePoints and TrueHits -> some of the following modules will be utilizing these relations!
sp2thConnector = register_module('SpacePoint2TrueHitConnector')
# sp2thConnector.logging.log_level = LogLevel.WARNING
sp2thConnector.logging.log_level = MyLogLevel
sp2thConnector.logging.debug_level = MyDebugLevel
param_sp2thConnector = {  # still have to find the appropriate value
                          # 'maxLocalPosDiff': 0.01,
    'DetectorTypes': ['PXD', 'SVD'],
    'TrueHitNames': ['', ''],
    'ClusterNames': ['', ''],
    'SpacePointNames': ['pxdOnly', 'nosingleSP', 'singleSP'],
    'outputSuffix': '_relTH',
    'storeSeperate': True,
    'registerAll': False,
    'maxGlobalPosDiff': 0.05,
    'maxPosSigma': 5,
    }
sp2thConnector.param(param_sp2thConnector)

# TCConverter, genfit -> SPTC
trackCandConverter = register_module('GFTC2SPTCConverter')
# trackCandConverter.logging.log_level = LogLevel.INFO
trackCandConverter.logging.log_level = MyLogLevel
trackCandConverter.logging.debug_level = MyDebugLevel
param_trackCandConverter = {  #    'PXDClusters': 'myPXDClusters',
                              #    'SVDClusters': 'mySVDClusters',
    'NoSingleClusterSVDSP': 'nosingleSP_relTH',
    'SingleClusterSVDSP': 'singleSP_relTH',
    'PXDClusterSP': 'pxdOnly_relTH',
    'checkNoSingleSVDSP': True,
    'checkTrueHits': True,
    'useSingleClusterSP': False,
    'skipCluster': False,
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
    'minNDF': int(1),
    }
trackCandConverter.param(param_trackCandConverter)

# SpacePointTrackCand referee
sptcReferee = register_module('SPTCReferee')
# sptcReferee.logging.log_level = MyLogLevel
sptcReferee.logging.log_level = LogLevel.INFO
sptcReferee.logging.debug_level = MyDebugLevel
param_sptcReferee = {
    'sptcName': 'SPTracks',
    'newArrayName': 'checkedSPTCs',
    'curlingSuffix': '_curlParts',
    'storeNewArray': True,
    'setOrigin': [0., 0., 0.],
    'checkCurling': True,
    'splitCurlers': True,
    'keepOnlyFirstPart': True,
    'kickSpacePoint': False,
    'checkMinDistance': True,
    'minDistance': 0.05,
    'checkSameSensor': True,
    'useMCInfo': True,
    }
sptcReferee.param(param_sptcReferee)

# back conversion and testing commented out -> not needed at the moment
# TCConverter, SPTC -> genfit
# btrackCandConverter = register_module('SPTC2GFTCConverter')
# btrackCandConverter.logging.log_level = LogLevel.INFO
# btrackCandConverter.logging.debug_level = 100
# param_btrackCandConverter = {
    # 'SpacePointTCName': 'SPTracks',
    # 'genfitTCName': 'myMCTracks',
# }
# btrackCandConverter.param(param_btrackCandConverter)

## TCConverterTest
# tcConverterTest = register_module('TCConvertersTest')
# tcConverterTest.logging.log_level = LogLevel.INFO
# tcConverterTest.logging.debug_level = 100
# param_tcConverterTest = {
    # 'SpacePointTCName': 'SPTracks',
    # 'genfitTCNames': ['mcTracks','myMCTracks'],
    # 'SpacePointArrayNames': 'pxdOnly',
# }
# tcConverterTest.param(param_tcConverterTest)

# Path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)

if useEvtGen:
  ##following modules only for evtGen:
    main.add_module(evtgeninput)
    if usePGun:
        main.add_module(particlegun)
else:
  ## following modules only for pGun:
    main.add_module(particlegun)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(svdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdClusterizer)
main.add_module(spCreatorPXD)
main.add_module(spCreatorSVD)
main.add_module(singleSpCreatorSVD)

main.add_module(mcTrackFinder)

main.add_module(sp2thConnector)
main.add_module(trackCandConverter)
main.add_module(sptcReferee)

# main.add_module(btrackCandConverter)
# main.add_module(tcConverterTest)

process(main)

print statistics
