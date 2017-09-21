#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

numEvents = 5000
initialValue = 1

usePXD = True
useEvtGen = False
usePGun = True

useSimpleClusterizer = False
useSameEdepThreshold = False  # does only trigger if useEDeposit == False && useSimpleClusterizer == True
# If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml, where you have to activate see neutrons = true
useEDeposit = True

useCustomDecFile = False
doMultiCore = True
doLostTCcheck = True


decFilePath = '/home/jkl/FW/optRel/12.2014_analysisGeoDependentEfficicency/DECAYnoKshortDaughters.DEC'
# momDistribution = 'uniformPt' # uniform in transverse momentum
momDistribution = 'uniform'  # uniform in momentum

# flags for the pGun
numTracks = 10
# transverseMomentum:
momentumMin = 4.  # GeV/c
momentumMax = 4.  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 15.0  # degrees
thetaMax = 152.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees

pdgType = 13  # muons
# pdgType = 11  # electrons
# pdgType = 2212 # protons
# pdgType = 211 # pions
# pdgType = 321 # kaons

charges = [pdgType, -pdgType]

# normal
xVertex = [-0.01, 0.01]
yVertex = [-0.01, 0.01]
zVertex = [-0.5, 0.5]

# fixIP
# xVertex = [-0., 0.]
# yVertex = [-0., 0.]
# zVertex = [-0., 0.]


if len(argv) > 1:
    numEvents = int(argv[1])
    print('1st argument given, new value for numEvents: ' + str(numEvents))
if len(argv) > 2:
    momentumMin = float(argv[2])
    print('2nd argument given, new value for momentumMin: ' + str(momentumMin))
    print('pGun is now activated! Value for evtGen: ' + str(useEvtGen))
    usePGun = True
if len(argv) > 3:
    momentumMax = float(argv[3])
    print('3rd argument given, new value for momentumMax: ' + str(momentumMax))
if len(argv) > 4:
    numTracks = int(argv[4])
    print('4th argument given, new value for numTracks: ' + str(numTracks))
if len(argv) > 5:
    thetaMin = float(argv[5])
    print('5th argument given, new value for thetaMin: ' + str(thetaMin))
if len(argv) > 6:
    thetaMax = float(argv[6])
    print('6th argument given, new value for thetaMax: ' + str(thetaMax))
if len(argv) > 7:
    pdgType = int(argv[7])
    print('7th argument given, new value for particleType: ' + str(pdgType))
    charges = [pdgType, -pdgType]

tuneValue = 0.06
# old geometry for SVD:
# secSetup = ['secMapEvtGenOnR10933June2014SVDStd-moreThan500MeV_SVD',
# 'secMapEvtGenOnR10933June2014SVDStd-125to500MeV_SVD',
# 'secMapEvtGenOnR10933June2014SVDStd-30to125MeV_SVD']
# new 2.2 geometry for SVD:
secSetup = [
    'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-moreThan500MeV_SVD',
    'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-125to500MeV_SVD',
    'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-30to125MeV_SVD']

if usePXD:
        # new 2.2 geometry for SVD:
    secSetup = \
        ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD',
         'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD',
         'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'
         ]
    # old geometry for SVD:
    # secSetup = ['secMapEvtGenOnR10933June2014VXDStd-moreThan500MeV_PXDSVD',
    # 'secMapEvtGenOnR10933June2014VXDStd-125to500MeV_PXDSVD',
    # 'secMapEvtGenOnR10933June2014VXDStd-30to125MeV_PXDSVD']
    tuneValue = 0.22
print('running {events:} events, Seed {theSeed:} - evtGen No BG'.format(events=numEvents,
                                                                        theSeed=initialValue))

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

pxdDigitizer = register_module('PXDDigitizer')
pxdDigitizer.param('ADC', False)
svdDigitizer = register_module('SVDDigitizer')
svdDigitizer.param('ADC', False)
pxdClusterizer = register_module('PXDClusterizer')
pxdClusterizer.param('useADC', False)
svdClusterizer = register_module('SVDClusterizer')
svdClusterizer.param('ADC', False)

simpleClusterizer = register_module('VXDSimpleClusterizer')
simpleClusterizer.logging.log_level = LogLevel.INFO
simpleClusterizer.logging.debug_level = 1
# simpleClusterizer.param('setMeasSigma', 0)
simpleClusterizer.param('onlyPrimaries', True)
if useEDeposit is False:
    simpleClusterizer.param('energyThresholdU', -0.0001)  # eDep in GeV
    simpleClusterizer.param('energyThresholdV', -0.0001)  # eDep in GeV
    simpleClusterizer.param('energyThreshold', -0.0001)  # eDep in GeV
elif useSameEdepThreshold:
    simpleClusterizer.param('energyThresholdU', 7E-6)  # eDep in GeV
    simpleClusterizer.param('energyThresholdV', 7E-6)  # eDep in GeV
    simpleClusterizer.param('energyThreshold', 7E-6)  # eDep in GeV
    print('energyThreshold is now set at the same value for everyone!')

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING
if useCustomDecFile:
    evtgeninput.param('DECFile', decFilePath)

particlegun = register_module('ParticleGun')
param_pGun = {  # 13: muons, 211: charged pions
                # fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
    'pdgCodes': charges,
    'nTracks': numTracks,
    'momentumGeneration': momDistribution,
    'momentumParams': [momentumMin, momentumMax],
    'thetaGeneration': 'uniform',
    'thetaParams': [thetaMin, thetaMax],
    'phiGeneration': 'uniform',
    'phiParams': [phiMin, phiMax],
    'vertexGeneration': 'uniform',
    'xVertexParams': xVertex,
    'yVertexParams': yVertex,
    'zVertexParams': zVertex,
}
particlegun.param(param_pGun)


geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
g4sim.param('SecondariesEnergyCut', 0.)  # that correct?


vxdtf = register_module('TFRedesign')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               'GFTrackCandidatesColName': 'caTracks',
               'writeToRoot': True,
               # 'calcQIType': 'kalman',
               'tuneCutoffs': tuneValue}

vxdtf.param(param_vxdtf)

# VXDTF DQM module
# vxdtf_dqm = register_module('VXDTFDQM')
# vxdtf_dqm.param('GFTrackCandidatesColName', 'caTracks')

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

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')


# Conv
spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.logging.log_level = LogLevel.INFO
spCreatorSVD.logging.debug_level = 5
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)

spCreatorSVDSingle = register_module('SVDSpacePointCreator')
spCreatorSVDSingle.logging.log_level = LogLevel.INFO
spCreatorSVDSingle.logging.debug_level = 5
param_spCreatorSVDSingle = {'OnlySingleClusterSpacePoints': True,
                            'NameOfInstance': 'singleClusters',
                            'SpacePoints': 'singleSPonly'}
spCreatorSVDSingle.param(param_spCreatorSVDSingle)

spCreatorPXD = register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.logging.debug_level = 5
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
# spCreatorPXD.param('PXDClusters', 'myPXDClusters')
spCreatorPXD.param('SpacePoints', 'pxdOnly')


# module to create relations between SpacePoints and TrueHits -> some of the following modules will be utilizing these relations!
sp2thConnector = register_module('SpacePoint2TrueHitConnector')
sp2thConnector.logging.log_level = LogLevel.INFO
sp2thConnector.logging.debug_level = 10
param_sp2thConnector = {  # still have to find the appropriate value
                          # 'maxLocalPosDiff': 0.01,
    'storeSeperate': False,
    'DetectorTypes': ['PXD', 'SVD'],
    'SpacePointNames': ['pxdOnly', 'nosingleSP', 'singleSPonly'],
    'TrueHitNames': ['', ''],
    'ClusterNames': ['', ''],
    'outputSuffix': '_relTH',
    'registerAll': False,
    'maxGlobalPosDiff': 0.05,
    'maxPosSigma': 4,
}
sp2thConnector.param(param_sp2thConnector)


# TCConverter, genfit -> SPTC
# NOTE: at the moment the parameters are set as such, that this module only uses SpacePoints that are related to TrueHits!
trackCandConverter = register_module('GFTC2SPTCConverter')
trackCandConverter.logging.log_level = LogLevel.INFO
trackCandConverter.logging.debug_level = 10
param_trackCandConverter = {  # 'PXDClusters': 'myPXDClusters',
                              #    'SVDClusters': 'mySVDClusters',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'SingleClusterSVDSP': 'singleSPonly',
    'checkNoSingleSVDSP': True,
    'checkTrueHits': True,
    'useSingleClusterSP': False,  # switch between singlesOnly and all SPs
    'PXDClusterSP': 'pxdOnly',
    'genfitTCName': 'lostMCTracks',
    # 'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
}
trackCandConverter.param(param_trackCandConverter)


sptcReferee = register_module('SPTCReferee')
param_sptcReferee = {
    'sptcName': 'SPTracks',
    'newArrayName': 'checkedSPTCs',
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


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 50)
# if doMultiCore:
#     eventCounter.param('allowMultiThreaded', True)
# else:
eventCounter.param('allowMultiThreaded', False)

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 11
param_analyzer = {'printExtentialAnalysisData': False,
                  'caTCname': 'caTracks',
                  'acceptedTCname': 'acceptedVXDTFTracks',
                  'lostTCname': 'lostMCTracks'}
analyzer.param(param_analyzer)

# Create paths
main = create_path()
# histo = register_module('HistoManager')
# histo.param('histoFileName', 'DQM-VXDTFdemo.root')  # File to save histograms
# main.add_module(histo)
# Add modules to paths

# main.add_module(inputM)
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
main.add_module(g4sim)
if useSimpleClusterizer:
    main.add_module(simpleClusterizer)
else:
    main.add_module(pxdDigitizer)
    main.add_module(pxdClusterizer)
    main.add_module(svdDigitizer)
    main.add_module(svdClusterizer)
main.add_module(track_finder_mc_truth)
main.add_module(eventCounter)
if doLostTCcheck:
    main.add_module(spCreatorPXD)
    main.add_module(spCreatorSVD)
    main.add_module(spCreatorSVDSingle)
    main.add_module(sp2thConnector)
main.add_module(vxdtf)
main.add_module(analyzer)
if doLostTCcheck:
    main.add_module(trackCandConverter)
    main.add_module(sptcReferee)
# main.add_module(trackfitter)
# main.add_module(vxdtf_dqm)
# Process events

if doMultiCore:
    set_nprocesses(6)
else:
    set_nprocesses(0)

process(main)

print('Event Statistics :')
print(statistics)
