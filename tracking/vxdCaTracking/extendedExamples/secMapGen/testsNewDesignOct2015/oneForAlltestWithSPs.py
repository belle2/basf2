#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv
from beamparameters import add_beamparameters

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
useEvtGen = False
usePGun = True
# useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
# where you have to activate see neutrons = true:
useEDeposit = True
# doExportXML: can be used here directly or when using the other file in this directory, called "onlyExportMap.py".
# executing that in separate files gives you the possibility to repeat the oneForAll-File several times to increase
# the sample-size used for secMapGen:
doExportXML = False
# usePXD: useful only for generateSecMap = false, activates secMaps for PXD+SVD and changes settings for the
# TrackFinderMCTruth too.
usePXD = False

newTrain = False
# ################
if generateSecMap:
    # usePXD: the MCTF provides full TCs all the time for secMapGen. Filtering of PXDSPs is done within the filterCalculators
    usePXD = True

# Important parameters:
numEvents = 5000  # can be overridden by the parameters given via terminal
initialValue = 0  # want random events, if set to 0


# flags for GFTC2SPTC
checkTH = True


# flags for the pGun
numTracks = 5
# transverseMomentum:
momentumMin = 0.05  # GeV/c
momentumMax = 0.3  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 75.0  # degrees
thetaMax = 85.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 350.  # degrees
phiMax = 360.  # degrees
pdgCODES = [13]


# flags for the 2nd pGun
numTracks2 = 5
# transverseMomentum:
momentumMin2 = 0.3  # GeV/c
momentumMax2 = 3.5  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin2 = 75.0  # degrees
thetaMax2 = 85.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin2 = 350.  # degrees
phiMax2 = 360.  # degrees
pdgCODES2 = [13]

# parameters for the secMap-calculation (pT in GeV/c):
pTcuts = [0.025, 0.1, 0.4]

# choose a meaningful name for the sectorMap:
setupFileName = 'evtGenPGunR19630Jul2015Old'
setupFileName2 = 'evtGenPGunR19630Jul2015New'


# set the relative coordinates for the secMapCreation:
secConfigUStd = [0., 0.33, 0.67, 1.0]
secConfigVStd = [0., 0.33, 0.67, 1.0]

trainerVXDTFLogLevel = LogLevel.DEBUG
trainerVXDTFDebugLevel = 10

MyLogLevel = LogLevel.INFO
MyDebugLevel = 10

# set for the curling splitter module and GFTC2SPTC
MyOtherLogLevel = LogLevel.ERROR
MyOtherDebugLevel = 10

posAnalysisRootFileName = 'bla' + 'TH_' + 'allTH_' + '_' + str(int(numEvents / 1000)) + 'k'


# if you want to test your newly generated sectorMaps, copy them to tracking/data add them to the VXDTFIndex.xml,
# and copy their names in the container "secSetup"
# (please take care that your entries are of the same format as used in secSetup)

if usePXD:
    secSetup = [setupFileName + 'VXDStd-moreThan400MeV_PXDSVD',
                setupFileName + 'VXDStd-100to400MeV_PXDSVD', setupFileName + 'VXDStd-25to100MeV_PXDSVD']
    secSetup2 = [setupFileName2 + 'VXDStd-moreThan400MeV_PXDSVD',
                 setupFileName2 + 'VXDStd-100to400MeV_PXDSVD', setupFileName2 + 'VXDStd-25to100MeV_PXDSVD']

else:
    secSetup = [setupFileName + 'SVDStd-moreThan400MeV_SVD',
                setupFileName + 'SVDStd-100to400MeV_SVD', setupFileName + 'SVDStd-25to100MeV_SVD']
    secSetup2 = [setupFileName2 + 'SVDStd-moreThan400MeV_SVD',
                 setupFileName2 + 'SVDStd-100to400MeV_SVD', setupFileName2 + 'SVDStd-25to100MeV_SVD']

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
elif len(argv) is 4:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    eventAdd = int(argv[3]) * numEvents
    print('3 arguments given, new value for seed: ' + str(initialValue) +
          ', for numEvents: ' + str(numEvents) + ', for eventAdd: ' + str(eventAdd))
elif len(argv) is 5:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    eventAdd = int(argv[3]) * numEvents
    tempX = int(argv[4])
    if tempX is 0:
        doExportXML = False
    else:
        doExportXML = True
    print('4 arguments given, new value for seed: ' + str(initialValue) +
          ', for numEvents: ' + str(numEvents) + ', for eventAdd: ' +
          str(eventAdd) + ', for doExportXML: ' + str(doExportXML))


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

setupFileNamesvdNew = setupFileName2 + 'SVD' + 'Std'
setupFileNamevxdNew = setupFileName2 + 'VXD' + 'Std'

param_filterCalcVXDStdNew = {
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

param_filterCalcSVDStdNew = {
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

# 13: muons, 211: charged pions
# fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
particlegun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': pdgCODES,
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
    'pdgCodes': pdgCODES2,
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


gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])

progress = register_module('Progress')

print('')
print('')


filterCalcVXDStdNew = register_module('SecMapTrainerWithSpacePoints')
filterCalcVXDStdNew.logging.log_level = LogLevel.INFO
filterCalcVXDStdNew.logging.debug_level = 15
filterCalcVXDStdNew.param(param_filterCalcVXDStdNew)

filterCalcSVDStdNew = register_module('SecMapTrainerWithSpacePoints')
filterCalcSVDStdNew.logging.log_level = LogLevel.INFO
filterCalcSVDStdNew.logging.debug_level = 15
filterCalcSVDStdNew.param(param_filterCalcSVDStdNew)


secMapBootStrap = register_module('SectorMapBootstrap')
secMapBootStrap.param('ReadSectorMap', False)
secMapBootStrap.param('WriteSectorMap', True)


if newTrain:
    newSecMapTrainer = register_module('SecMapTrainerVXDTF')
    newSecMapTrainer.logging.log_level = trainerVXDTFLogLevel
    newSecMapTrainer.logging.debug_level = trainerVXDTFDebugLevel
    newSecMapTrainer.param('spTCarrayName', 'checkedSPTCs')

    newSecMapTrainer2 = register_module('SecMapTrainerBase')
    newSecMapTrainer2.logging.log_level = trainerVXDTFLogLevel
    newSecMapTrainer2.logging.debug_level = trainerVXDTFDebugLevel
    newSecMapTrainer2.param('spTCarrayName', 'checkedSPTCs')
else:
    merger = register_module('RawSecMapMerger')
    merger.logging.log_level = trainerVXDTFLogLevel
    merger.logging.debug_level = trainerVXDTFDebugLevel
    # merger.param('spTCarrayName', 'checkedSPTCs')


# using one export module only
exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.INFO
exportXML.logging.debug_level = 1
exportXML.param('rootFileName', setupFileName)
exportXML.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
exportXML.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
exportXML.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
exportXML.param('smallSampleQuantiles', [0, 1])  # first for min, second for max


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
    param_vxdtf = {'sectorSetup': secSetup,
                   'nameOfInstance': 'VXDTFtestOld',
                   'InfoBoardName': 'testOld',
                   'GFTrackCandidatesColName': 'caTracksOld',
                   'writeToRoot': True,
                   'tuneCutoffs': 0.22}
    vxdtf.param(param_vxdtf)

    vxdtf2 = register_module('VXDTF')
    vxdtf2.logging.log_level = LogLevel.DEBUG
    vxdtf2.logging.debug_level = 2
    param_vxdtf2 = {'sectorSetup': secSetup,
                    'nameOfInstance': 'VXDTFtestNew',
                    'InfoBoardName': 'testNew',
                    'GFTrackCandidatesColName': 'caTracksNew',
                    'writeToRoot': True,
                    'tuneCutoffs': 0.22}
    vxdtf2.param(param_vxdtf2)

    analyzer = register_module('TFAnalizer')
    analyzer.logging.log_level = LogLevel.INFO
    analyzer.logging.debug_level = 1
    param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracksOld', 'InfoBoardName': 'testOld'}
    analyzer.param(param_analyzer)

    analyzer2 = register_module('TFAnalizer')
    analyzer2.logging.log_level = LogLevel.INFO
    analyzer2.logging.debug_level = 1
    param_analyzer2 = {'printExtentialAnalysisData': False, 'caTCname': 'caTracksNew', 'InfoBoardName': 'testNew'}
    analyzer2.param(param_analyzer2)


doPXD = 0
if usePXD:
    doPXD = 1
mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': doPXD,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
mctrackfinder.param(param_mctrackfinder)


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
    'genfitTCName': 'mcTracks',
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
main.add_module(secMapBootStrap)

if generateSecMap:
    # # following modules only for secMapGen:
    if doExportXML:
        main.add_module(exportXML)
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
    main.add_module(mctrackfinder)

    main.add_module(sp2thConnector)
    main.add_module(trackCandConverter)
    main.add_module(sptcReferee)
    # main.add_module(filterCalcVXDStdNew)
    # main.add_module(filterCalcSVDStdNew)

    if newTrain:
        main.add_module(newSecMapTrainer)
        main.add_module(newSecMapTrainer2)
    else:
        main.add_module(merger)
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
    main.add_module(vxdtf2)
    main.add_module(vxdtf)
    main.add_module(mctrackfinder)
    main.add_module(analyzer)
    main.add_module(analyzer2)


# Process events
process(main)

print(statistics)
