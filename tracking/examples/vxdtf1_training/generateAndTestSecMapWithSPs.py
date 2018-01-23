#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from beamparameters import add_beamparameters
from subprocess import call
from sys import argv

# #### initial information:
# This file is a "oneForAll"-approach carrying as much of the secMap-creation and testing process in one file.
# The total cyle of creating secMaps needs creating of raw stuff (this file, with 'generateSecMap' = true),
# distilling the actual secMap out of the collected samples (this file when 'doExportXML' is true or onlyExportMap.py
# copying the stuff into tracking/data, adding their names to the VXDTFindex.xml in the same directory and running "scons -D data"
# testing the new sectorMap with this (with 'generateSecMap' = false) or any other File you want to use
# (don't forget to set the correct names into the 'secSetup' variable!
#####


# ################

generateSecMap = True  # <----------- here you can switch between generating sectorMaps and testing with the VXDTF!
useSimpleClusterizer = False
useEvtGen = True
usePGun = True
# useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
# where you have to activate see neutrons = true:
useEDeposit = True
# usePXD: useful only for generateSecMap = false, activates secMaps for PXD+SVD and changes settings for the
# TrackFinderMCTruth too.
usePXD = False
# ################
if generateSecMap:
    # usePXD: the MCTF provides full TCs all the time for secMapGen. Filtering of PXDSPs is done within the filterCalculators
    usePXD = True

# Important parameters:
numEvents = 5000  # can be overridden by the parameters given via terminal
initialValue = 3  # want random events, if set to 0


# flags for GFTC2SPTC
checkTH = True


# flags for the pGun
numTracks = 5
# transverseMomentum:
momentumMin = 0.03  # GeV/c
momentumMax = 0.3  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 12.0  # degrees
thetaMax = 155.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees


# flags for the 2nd pGun
numTracks2 = 5
# transverseMomentum:
momentumMin2 = 0.3  # GeV/c
momentumMax2 = 3.5  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin2 = 12.0  # degrees
thetaMax2 = 155.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin2 = 0.  # degrees
phiMax2 = 360.  # degrees

# parameters for the secMap-calculation (pT in GeV/c):
pTcuts = [0.025, 0.1, 0.4]

# choose a meaningful name for the sectorMap:
setupFileName = 'TESTSecMaps'


# set the relative coordinates for the secMapCreation:
secConfigUStd = [0., 0.33, 0.67, 1.0]
secConfigVStd = [0., 0.33, 0.67, 1.0]


MyLogLevel = LogLevel.INFO
MyDebugLevel = 10

# set for the curling splitter module and GFTC2SPTC
MyOtherLogLevel = LogLevel.ERROR
MyOtherDebugLevel = 10

posAnalysisRootFileName = 'bla' + 'TH_' + 'allTH_' + '_' + str(int(numEvents / 1000)) + 'k'


# if you want to test your newly generated sectorMaps, copy them to tracking/data add them to the VXDTFIndex.xml,
# and copy their names in the container "secSetup"
# (please take care that your entries are of the same format as used in secSetup)

if generateSecMap is False:
    secSetupSVD = [setupFileName + 'SVDStd-moreThan400MeV_SVD',
                   setupFileName + 'SVDStd-100to400MeV_SVD',
                   setupFileName + 'SVDStd-25to100MeV_SVD']
    secSetupVXD = [setupFileName + 'VXDStd-moreThan400MeV_PXDSVD',
                   setupFileName + 'VXDStd-100to400MeV_PXDSVD',
                   setupFileName + 'VXDStd-25to100MeV_PXDSVD']

# some other settings for the VXDTF
qiType = 'circleFit'
filterOverlaps = 'hopfield'


if len(argv) is 1:
    print('no arguments given, using standard values')
elif len(argv) is 2:
    initialValue = int(argv[1])
    print('1 argument given, new value for seed: ' + str(initialValue))
elif len(argv) is 3:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    print('2 arguments given, new value for seed: ' + str(initialValue) +
          ' and for numEvents: ' + str(numEvents))


clusterType = 'fullClusterizer'
if useSimpleClusterizer:
    clusterType = 'simpleClusterizer'
particleGenType = 'pGun'
if useEvtGen:
    particleGenType = 'evtGen'
eDepType = 'eDepNo'
if useEDeposit:
    eDepType = 'eDepYes'


set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)


setupFileNamesvdNew = setupFileName + 'SVD' + 'Std'
setupFileNamevxdNew = setupFileName + 'VXD' + 'Std'


param_secMapDataCollectVXD = {
    'detectorType': ['SVD', 'PXD'],
    'maxXYvertexDistance': 1.,
    'pTcuts': pTcuts,
    'pTCutSmearer': 5.,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUStd,
    'sectorConfigV': secConfigVStd,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileNamevxdNew, 'UPDATE'],
    'sectorSetupFileName': setupFileNamevxdNew,
    'spTCarrayName': 'checkedSPTCs',
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
}

param_secMapDataCollectSVD = {
    'detectorType': ['SVD'],
    'maxXYvertexDistance': 1.,
    'pTcuts': pTcuts,
    'pTCutSmearer': 5.,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUStd,
    'sectorConfigV': secConfigVStd,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileNamesvdNew, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvdNew,
    'spTCarrayName': 'checkedSPTCs',
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
}

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])

# 13: muons, 211: charged pions
# fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
particlegun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [13, -13],
    'nTracks': numTracks,
    'momentumGeneration': 'uniformPt',
    'momentumParams': [momentumMin, momentumMax],
    'thetaGeneration': 'uniform',
    'thetaParams': [thetaMin, thetaMax],
    'phiGeneration': 'uniform',
    'phiParams': [phiMin, phiMax],
    'vertexGeneration': 'uniform',
    'xVertexParams': [-0.1, 0.1],
    'yVertexParams': [-0.1, 0.1],
    'zVertexParams': [-0.5, 0.5],
}
particlegun.param(param_pGun)

particlegun2 = register_module('ParticleGun')
param_pGun2 = {
    'pdgCodes': [13, -13],
    'nTracks': numTracks2,
    'momentumGeneration': 'uniformPt',
    'momentumParams': [momentumMin2, momentumMax2],
    'thetaGeneration': 'uniform',
    'thetaParams': [thetaMin2, thetaMax2],
    'phiGeneration': 'uniform',
    'phiParams': [phiMin2, phiMax2],
    'vertexGeneration': 'uniform',
    'xVertexParams': [-0.1, 0.1],
    'yVertexParams': [-0.1, 0.1],
    'zVertexParams': [-0.5, 0.5],
}
particlegun2.param(param_pGun2)

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING


progress = register_module('Progress')

print('')
print('')


secMapDataCollectVXD = register_module('SecMapTrainerWithSpacePoints')
secMapDataCollectVXD.logging.log_level = LogLevel.INFO
secMapDataCollectVXD.logging.debug_level = 35
secMapDataCollectVXD.param(param_secMapDataCollectVXD)


secMapDataCollectSVD = register_module('SecMapTrainerWithSpacePoints')
secMapDataCollectSVD.logging.log_level = LogLevel.INFO
secMapDataCollectSVD.logging.debug_level = 35
secMapDataCollectSVD.param(param_secMapDataCollectSVD)


pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')

simpleClusterizer = register_module('VXDSimpleClusterizer')
simpleClusterizer.logging.log_level = LogLevel.INFO
simpleClusterizer.logging.debug_level = 1
simpleClusterizer.param('setMeasSigma', 0)
simpleClusterizer.param('onlyPrimaries', True)
if useEDeposit is False:
    simpleClusterizer.param('energyThresholdU', -0.0001)
    simpleClusterizer.param('energyThresholdV', -0.0001)
    simpleClusterizer.param('energyThreshold', -0.0001)

spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.logging.log_level = LogLevel.INFO
spCreatorSVD.logging.debug_level = 5
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)

spCreatorPXD = register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.logging.debug_level = 5
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
spCreatorPXD.param('SpacePoints', 'pxdOnly')


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 250)

if generateSecMap is False:
    vxdtf = register_module('VXDTF')
    vxdtf.logging.log_level = LogLevel.DEBUG
    vxdtf.logging.debug_level = 2
    param_vxdtf = {'sectorSetup': secSetupSVD,
                   'nameOfInstance': 'VXDTFtestSVD',
                   'InfoBoardName': 'testSVD',
                   'GFTrackCandidatesColName': 'caTracksSVD',
                   'writeToRoot': True,
                   'tuneCutoffs': 0.05}
    vxdtf.param(param_vxdtf)

    vxdtf2 = register_module('VXDTF')
    vxdtf2.logging.log_level = LogLevel.DEBUG
    vxdtf2.logging.debug_level = 2
    param_vxdtf2 = {'sectorSetup': secSetupVXD,
                    'nameOfInstance': 'VXDTFtestVXD',
                    'InfoBoardName': 'testVXD',
                    'GFTrackCandidatesColName': 'caTracksVXD',
                    'writeToRoot': True,
                    'tuneCutoffs': 0.15}
    vxdtf2.param(param_vxdtf2)

    analyzer = register_module('TFAnalizer')
    analyzer.logging.log_level = LogLevel.INFO
    analyzer.logging.debug_level = 1
    param_analyzer = {
        'printExtentialAnalysisData': False,
        'mcTCname': 'mcTracksSVD',
        'caTCname': 'caTracksSVD',
        'acceptedTCname': 'acceptedSVDTracks',
        'lostTCname': 'lostSVD',
        'InfoBoardName': 'testSVD',
        'rootFileName': [
            'TFANALIZERRESULTSsvdNormalSecMap',
            'RECREATE']}
    analyzer.param(param_analyzer)

    analyzer2 = register_module('TFAnalizer')
    analyzer2.logging.log_level = LogLevel.INFO
    analyzer2.logging.debug_level = 1
    param_analyzer2 = {
        'printExtentialAnalysisData': False,
        'mcTCname': 'mcTracksVXD',
        'caTCname': 'caTracksVXD',
        'acceptedTCname': 'acceptedVXDTracks',
        'lostTCname': 'lostVXDD',
        'InfoBoardName': 'testVXD',
        'rootFileName': [
            'TFANALIZERRESULTSvxdNormalSecMap',
            'RECREATE']}
    analyzer2.param(param_analyzer2)


# finds reco tracks instead of genfit tracks
mctrackfinderSVD = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinderSVD.logging.log_level = LogLevel.INFO
param_mctrackfinderSVD = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'Smearing': 0,
    # 'UseClusters': True, ### not present in the new implementation
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'RecoTracksStoreArrayName': 'mcRecoTracksSVD',
}
mctrackfinderSVD.param(param_mctrackfinderSVD)

# convert the recotracks to genfit tracks
converterReco_to_Genfit_SVD = register_module('GenfitTrackCandidatesCreator')
converterReco_to_Genfit_SVD.param('recoTracksStoreArrayName', 'mcRecoTracksSVD')
converterReco_to_Genfit_SVD.param('genfitTrackCandsStoreArrayName', 'mcTracksSVD')


mctrackfinderVXD = register_module('TrackFinderMCTruthRecoTracks')
mctrackfinderVXD.logging.log_level = LogLevel.INFO
param_mctrackfinderVXD = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    # 'UseClusters': True,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'RecoTracksStoreArrayName': 'mcRecoTracksVXD',
}
mctrackfinderVXD.param(param_mctrackfinderVXD)

# convert the recotracks to genfit tracks
converterReco_to_Genfit_VXD = register_module('GenfitTrackCandidatesCreator')
converterReco_to_Genfit_VXD.param('recoTracksStoreArrayName', 'mcRecoTracksVXD')
converterReco_to_Genfit_VXD.param('genfitTrackCandsStoreArrayName', 'mcTracksVXD')


# ############################################################### Converting and Referee ###########################################
# module to create relations between SpacePoints and TrueHits -> some of the following modules will be utilizing these relations!
sp2thConnector = register_module('SpacePoint2TrueHitConnector')
sp2thConnector.logging.log_level = LogLevel.WARNING
param_sp2thConnector = {
    'DetectorTypes': ['PXD', 'SVD'],
    'TrueHitNames': ['', ''],
    'ClusterNames': ['', ''],
    'SpacePointNames': ['pxdOnly', 'nosingleSP'],
    'outputSuffix': '_relTH',
    'storeSeperate': False,
    'registerAll': False,
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
param_trackCandConverterReference = {
    'genfitTCName': 'mcTracksVXD',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'PXDClusterSP': 'pxdOnly',
    'checkTrueHits': checkTH,
    'useSingleClusterSP': False,
    'checkNoSingleSVDSP': True,
}
trackCandConverter.param(param_trackCandConverterReference)

# SpacePointTrackCand referee
sptcReferee = register_module('SPTCReferee')
sptcReferee.logging.log_level = LogLevel.INFO
param_sptcReferee = {
    'sptcName': 'SPTracks',
    'newArrayName': 'checkedSPTCs',
    'storeNewArray': True,
    'checkCurling': True,
    'splitCurlers': True,
    'keepOnlyFirstPart': True,
    'kickSpacePoint': True,  # not necessarily essential -> alternatve: check and filter in SecMapTrainerBase
    'checkSameSensor': True,
    'useMCInfo': True,
}
sptcReferee.param(param_sptcReferee)


g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
if useEDeposit is False:
    g4sim.param('UICommands', [
        '/process/list',
        '/process/inactivate                msc',
        '/process/inactivate              hIoni',
        '/process/inactivate            ionIoni',
        '/process/inactivate              eIoni',
        '/process/inactivate              eBrem',
        '/process/inactivate            annihil',
        '/process/inactivate               phot',
        '/process/inactivate              compt',
        '/process/inactivate               conv',
        '/process/inactivate             hBrems',
        '/process/inactivate          hPairProd',
        '/process/inactivate              muMsc',
        '/process/inactivate             muIoni',
        '/process/inactivate            muBrems',
        '/process/inactivate         muPairProd',
        '/process/inactivate        CoulombScat',
        '/process/inactivate    PhotonInelastic',
        '/process/inactivate     ElectroNuclear',
        '/process/inactivate    PositronNuclear',
        '/process/inactivate              Decay',
        '/process/inactivate         hadElastic',
        '/process/inactivate   NeutronInelastic',
        '/process/inactivate           nCapture',
        '/process/inactivate           nFission',
        '/process/inactivate    ProtonInelastic',
        '/process/inactivate  PionPlusInelastic',
        '/process/inactivate PionMinusInelastic',
        '/process/inactivate  KaonPlusInelastic',
        '/process/inactivate KaonMinusInelastic',
        '/process/inactivate KaonZeroLInelastic',
        '/process/inactivate KaonZeroSInelastic',
        '/process/inactivate AntiProtonInelastic',
        '/process/inactivate AntiNeutronInelastic',
        '/process/inactivate    LambdaInelastic',
        '/process/inactivate AntiLambdaInelastic',
        '/process/inactivate SigmaMinusInelastic',
        '/process/inactivate AntiSigmaMinusInelastic',
        '/process/inactivate SigmaPlusInelastic',
        '/process/inactivate AntiSigmaPlusInelastic',
        '/process/inactivate   XiMinusInelastic',
        '/process/inactivate AntiXiMinusInelastic',
        '/process/inactivate    XiZeroInelastic',
        '/process/inactivate AntiXiZeroInelastic',
        '/process/inactivate OmegaMinusInelastic',
        '/process/inactivate AntiOmegaMinusInelastic',
        '/process/inactivate CHIPSNuclearCaptureAtRest',
        '/process/inactivate muMinusCaptureAtRest',
        '/process/inactivate  DeuteronInelastic',
        '/process/inactivate    TritonInelastic',
        '/process/inactivate      ExtEnergyLoss',
        '/process/inactivate       OpAbsorption',
        '/process/inactivate         OpRayleigh',
        '/process/inactivate            OpMieHG',
        '/process/inactivate         OpBoundary',
        '/process/inactivate              OpWLS',
        '/process/inactivate           Cerenkov',
        '/process/inactivate      Scintillation',
    ])
# "/process/inactivate        StepLimiter"

log_to_file('ONEFORALLseed_events_' + str(initialValue) + '_' + str(numEvents) + '.txt', append=False)

# Create paths
main = create_path()

beamparameters = add_beamparameters(main, "Y4S")

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
if useEvtGen:
    # # following modules only for evtGen:
    if usePGun:
        main.add_module(particlegun)
        main.add_module(particlegun2)
    main.add_module(evtgeninput)
else:
    # # following modules only for pGun:
    main.add_module(particlegun)
    main.add_module(particlegun2)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(g4sim)

if generateSecMap:
    if useSimpleClusterizer:
        main.add_module(simpleClusterizer)
    else:
        main.add_module(pxdDigitizer)
        main.add_module(svdDigitizer)
        main.add_module(pxdClusterizer)
        main.add_module(svdClusterizer)
        main.add_module(eventCounter)

    main.add_module(spCreatorPXD)
    main.add_module(spCreatorSVD)
    main.add_module(mctrackfinderVXD)
    main.add_module(converterReco_to_Genfit_VXD)

    main.add_module(sp2thConnector)
    main.add_module(trackCandConverter)
    main.add_module(sptcReferee)
    main.add_module(secMapDataCollectVXD)
    main.add_module(secMapDataCollectSVD)
else:
    # # following modules only for secMapTest:
    if useSimpleClusterizer:
        main.add_module(simpleClusterizer)
    else:
        main.add_module(pxdDigitizer)
        main.add_module(svdDigitizer)
        main.add_module(pxdClusterizer)
        main.add_module(svdClusterizer)
        main.add_module(eventCounter)
    main.add_module('SetupGenfitExtrapolation')
    main.add_module(vxdtf2)
    main.add_module(vxdtf)
    main.add_module(mctrackfinderSVD)
    main.add_module(converterReco_to_Genfit_SVD)
    main.add_module(mctrackfinderVXD)
    main.add_module(converterReco_to_Genfit_VXD)
    main.add_module(analyzer)
    main.add_module(analyzer2)

if generateSecMap:
    set_nprocesses(7)
    # set_nprocesses(0)
else:
    set_nprocesses(0)

# Process events
process(main)

print(statistics)
