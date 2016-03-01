#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

from VXDTF.setup_modules import (setup_gfTCtoSPTCConverters,
                                 setup_spCreatorPXD,
                                 setup_spCreatorSVD,
                                 setup_sp2thConnector,
                                 setup_qualityEstimators,
                                 setup_trackSetEvaluators)

from VXDTF.setup_modules_ml import *

# ################
# rootInputFileName = "seed4nEv100000pGun1_1T.root"
# rootInputFileName = "evtGenseed6nEv100000.root" #evtGenSinglePassTest-TrainSample. skipCluster = False
# rootInputFileName = "evtGenseed5nEv10000.root" #testSample. skipCluster = False
rootInputFileName = "evtGenseed7nEv10000.root"  # testSample. skipCluster = True
# rootInputFileName = "evtGenseed8nEv200000.root" # trainSample. skipCluster = True
# rootInputFileName = "evtGenseed8nEv100000.root" #evtGenSinglePassTest-TrainSample. skipCluster = True (TODO)

# file name into which the segNetAnalize stores its stuff
segNetAnaRFN = 'SegNetAnalyzer_SM_train.root'
fbdtSamplesFN = 'FBDTClassifier_samples_train_10k.dat'
fbdtFN = 'FBDTClassifier_1000_3.dat'


usePXD = False
useDisplay = False
newTrain = False  # if true, rawSecMap-Data is collected. IF false, new TF will be executed
printNetworks = False  # creates graphs for each network for each event if true
useOldTFinstead = False  # if true, the old vxdtf is used instead of the new one.
ignoreDeadTCs = True  # if true, the TrackFinderVXDAnalizer will not add dead TCs to the efficiencies

trainFBDT = False  # with the current settings: collects samples but does not train a FastBDT!
useFBDT = False  # use the ML Filter for creating the SegmentNetwork instead of the SectorMap filters
activateSegNetAnalizer = False  # only needed when studying FastBDT-behavior

if useFBDT:
    cNetworks = int(2)
else:
    cNetworks = int(3)

# Important parameters:

tempStringList = rootInputFileName.split('nEv', 1)
stringInitialValue = tempStringList[0].split("seed", 1)
print("found seed: " + stringInitialValue[1])
numEvents = 10000  # WARNING has to be identical with the value named in rootInputFileName!
# initialValue = int(stringInitialValue[1])
initialValue = 7

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

trainerVXDTFLogLevel = LogLevel.INFO
trainerVXDTFDebugLevel = 10

TFlogLevel = LogLevel.INFO
TFDebugLevel = 1

CAlogLevel = LogLevel.DEBUG
CADebugLevel = 1

AnalizerlogLevel = LogLevel.INFO
AnalizerDebugLevel = 1

# acceptedRawSecMapFiles = ['lowTestRedesign_202608818.root']
if (initialValue == 2):
    print("chosen initialvalue 2! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1373026662.root']  # 42
elif (initialValue == 3):
    print("chosen initialvalue 3! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_202608818.root']  # 23
elif (initialValue == 4):
    print("chosen initialvalue 4!! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_293660864.root']  # 24
# elif (initialValue == 5):
    # print("chosen initialvalue 5! (with background)" + rootInputFileName)
    # acceptedRawSecMapFiles = ['lowTestRedesign_753986291.root']  # 25
elif (initialValue == 5):
    print("chosen initialvalue 5 (skipCluster-setting=False)! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1120112796.root']
elif (initialValue == 6):
    print("chosen initialvalue 6 (skipCluster-setting=False)! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1120112796.root']
    # acceptedRawSecMapFiles = ['lowTestRedesign_1667035383.root'] # 26 - single track, single event  raw data
elif (initialValue == 7):
    print("chosen initialvalue 7!  (skipCluster-setting=True)")
    acceptedRawSecMapFiles = ['lowTestRedesign_1332084337.root']  # 27 - single track, single event raw data
elif (initialValue == 8):
    print("chosen initialvalue 8! (skipCluster-setting=True): 200k evtGen events")
    acceptedRawSecMapFiles = ['lowTestRedesign_1332084337.root']  # 28 - single track, single event raw data
elif (initialValue == 57):
    print("chosen initialvalue 57! setup remark: train: 10k events, 10 tracks per event, theta 60-85°, phi 0-360°, pT 100-145MeV.")
    thetaMin = 60.0  # degrees
    thetaMax = 85.  # degrees
    # phi: starting angle of particle direction in x-y-plane (r-phi-plane)
    phiMin = 0.  # degrees
    phiMax = 360.  # degrees
    acceptedRawSecMapFiles = ['lowTestRedesign_779994078.root']  # 55 - single track, single event raw data
else:
    print("ERROR! no valid initialvalue chosen!")
    acceptedRawSecMapFiles = [""]

print('')
print('')


rootInputM = register_module('RootInput')
rootInputM.param('inputFileName', rootInputFileName)

# rootInputM.param('skipNEvents', int(10000))

eventinfoprinter = register_module('EventInfoPrinter')


gearbox = register_module('Gearbox')


secMapBootStrap = register_module('SectorMapBootstrap')


secMapBootStrap = register_module('SectorMapBootstrap')

evtStepSize = 1
if newTrain:
    newSecMapTrainerBase = register_module('SecMapTrainerBase')
    newSecMapTrainerBase.logging.log_level = trainerVXDTFLogLevel
    newSecMapTrainerBase.logging.debug_level = trainerVXDTFDebugLevel
    newSecMapTrainerBase.param('spTCarrayName', 'checkedSPTCs')
    newSecMapTrainerBase.param('allowTraining', True)

    evtStepSize = 100
else:
    merger = register_module('RawSecMapMerger')
    merger.logging.log_level = trainerVXDTFLogLevel
    merger.logging.debug_level = trainerVXDTFDebugLevel
    merger.param('rootFileNames', acceptedRawSecMapFiles)
    # merger.param('spTCarrayName', 'checkedSPTCs')

if useOldTFinstead:
    evtStepSize = 100

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', evtStepSize)


if useOldTFinstead:
    tuneValue = 0.06
    secSetup = [
        'shiftedL3IssueTestSVDStd-moreThan400MeV_SVD',
        'shiftedL3IssueTestSVDStd-100to400MeV_SVD',
        'shiftedL3IssueTestSVDStd-25to100MeV_SVD']
    if usePXD:
        secSetup = \
            ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
             'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
             'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
             ]
        tuneValue = 0.22
    vxdtf = register_module('VXDTF')  # VXDTF TFRedesign
    vxdtf.logging.log_level = LogLevel.DEBUG
    vxdtf.logging.debug_level = 1
    param_vxdtf = {'sectorSetup': secSetup,
                   'GFTrackCandidatesColName': 'caTracks',
                   'tuneCutoffs': tuneValue,
                   'displayCollector': 2
                   }
    vxdtf.param(param_vxdtf)

    oldAnalyzer = register_module('TFAnalizer')
    oldAnalyzer.logging.log_level = LogLevel.INFO
    oldAnalyzer.param('printExtentialAnalysisData', False)
    oldAnalyzer.param('caTCname', 'caTracks')
    oldAnalyzer.param('acceptedTCname', 'VXDTFoldAcceptedTCS')
    oldAnalyzer.param('lostTCname', 'VXDTFoldLostTCS')

    # TCConverter, genfit -> SPTC
    trackCandConverter = register_module('GFTC2SPTCConverter')
    trackCandConverter.logging.log_level = LogLevel.WARNING
    trackCandConverter.param('genfitTCName', 'caTracks')
    trackCandConverter.param('SpacePointTCName', 'caSPTCs')
    trackCandConverter.param('NoSingleClusterSVDSP', 'nosingleSP')
    trackCandConverter.param('PXDClusterSP', 'pxdOnly')
    trackCandConverter.param('checkNoSingleSVDSP', True)
    trackCandConverter.param('checkTrueHits', False)
    trackCandConverter.param('useSingleClusterSP', False)
    trackCandConverter.param('skipCluster', True)
else:
    segNetProducer = register_module('SegmentNetworkProducer')
    segNetProducer.param('CreateNeworks', cNetworks)
    segNetProducer.param('NetworkOutputName', 'test2Hits')
    segNetProducer.param('printNetworks', printNetworks)
    # segNetProducer.param('SpacePointsArrayNames', ['pxdOnly', 'nosingleSP'])
    segNetProducer.param('SpacePointsArrayNames', ['nosingleSP_relTH'])
    segNetProducer.logging.log_level = TFlogLevel
    segNetProducer.logging.debug_level = TFDebugLevel

    if activateSegNetAnalizer:
        segNetAnalyzer = register_module('SegmentNetworkAnalyzer')
        segNetAnalyzer.param('networkInputName', 'test2Hits')
        segNetAnalyzer.param('rootFileName', segNetAnaRFN)
        segNetAnalyzer.logging.log_level = LogLevel.INFO
        segNetAnalyzer.logging.debug_level = 100

    cellOmat = register_module('TrackFinderVXDCellOMat')
    cellOmat.param('printNetworks', printNetworks)
    cellOmat.param('SpacePointTrackCandArrayName', 'caSPTCs')
    cellOmat.param('NetworkName', 'test2Hits')
    cellOmat.logging.log_level = CAlogLevel
    cellOmat.logging.debug_level = CADebugLevel


vxdAnal = register_module('TrackFinderVXDAnalizer')
vxdAnal.param('referenceTCname', 'SPTracks')
vxdAnal.param('testTCname', 'caSPTCs')
vxdAnal.param('purityThreshold', 0.7)
vxdAnal.param('ignoreDeadTCs', ignoreDeadTCs)
vxdAnal.logging.log_level = AnalizerlogLevel
vxdAnal.logging.debug_level = AnalizerDebugLevel


if newTrain:
    log_to_file('testRedesign' + str(initialValue) + '_' + str(numEvents) + '.log', append=False)
else:
    log_to_file('testsegNetExecute' + str(initialValue) + '_' + str(numEvents) + '.log', append=False)
# Create paths
main = create_path()


main.add_module(rootInputM)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(eventCounter)
main.add_module(secMapBootStrap)

setup_spCreatorSVD(path=main, nameOutput='nosingleSP', createSingleClusterSPs=False, logLevel=LogLevel.INFO)
setup_spCreatorPXD(path=main, nameOutput='pxdOnly', logLevel=LogLevel.INFO)  # needed since 2gftc-converter does not work without it
setup_gfTCtoSPTCConverters(
    path=main,
    pxdSPs='pxdOnly',
    svdSPs='nosingleSP',
    gfTCinput='mcTracks',
    sptcOutput='checkedSPTCs',
    usePXD=usePXD,
    logLevel=LogLevel.WARNING)

# connect all SpacePoints to all possible TrueHits and store them in a new
# StoreArray (to not interfere with the SpacePoints of the reference
# TrackCands)
setup_sp2thConnector(main, 'pxdOnly', 'nosingleSP', '_relTH', True, LogLevel.ERROR, 1)
if newTrain:
    main.add_module(newSecMapTrainerBase)
else:
    if useOldTFinstead:
        main.add_module(register_module('SetupGenfitExtrapolation'))
        main.add_module(vxdtf)
        main.add_module(oldAnalyzer)
        main.add_module(trackCandConverter)
    else:
        main.add_module(merger)
        main.add_module(segNetProducer)
        if trainFBDT:  # collect in this step
            add_fbdtclassifier_training(main, 'test2Hits', 'FBDTClassifier.dat', False, True,
                                        False, fbdtSamplesFN, 100, 3, 0.15, 0.5, LogLevel.DEBUG, 10)
        if useFBDT:  # apply the filters
            add_ml_threehitfilters(main, 'test2Hits', fbdtFN, 0.989351, True)
        if activateSegNetAnalizer:
            main.add_module(segNetAnalyzer)
        main.add_module(cellOmat)
        setup_qualityEstimators(main, 'circleFit', 'caSPTCs', LogLevel.DEBUG, 1)

        tcNetworkProducer = register_module('SPTCNetworkProducer')
        tcNetworkProducer.param('tcArrayName', 'caSPTCs')
        tcNetworkProducer.param('tcNetworkName', 'tcNetwork')
        main.add_module(tcNetworkProducer)

        setup_trackSetEvaluators(main, 'hopfield', 'caSPTCs', 'tcNetwork')
    main.add_module(vxdAnal)

if useDisplay:
    display = register_module('Display')
    display.param('showAllPrimaries', True)
    main.add_module(display)
# Process events
process(main)

print(statistics)
