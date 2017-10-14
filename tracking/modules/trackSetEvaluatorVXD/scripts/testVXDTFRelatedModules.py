#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv
import time

from VXDTF.setup_modules import (setup_gfTCtoSPTCConverters,
                                 setup_spCreatorPXD,
                                 setup_spCreatorSVD,
                                 setup_sp2thConnector,
                                 setup_qualityEstimators,
                                 setup_trackSetEvaluators)

from VXDTF.setup_modules_ml import *

# ################
# rootInputFileName = "seed4nEv100000pGun1_1T.root"
# rootInputFileName = "seed14nEv100000pGun1_1T.root" # phi 0-90, theta 60-85, pT 100-145 MeV/c, PDG 13
# rootInputFileName = "seed13nEv100000pGun1_2T.root" # phi 0-90, theta 60-85, pT 100-145 MeV/c, PDG 13
# rootInputFileName = "evtGenseed6nEv100000.root" #evtGenSinglePassTest-TrainSample. skipCluster = False
# rootInputFileName = "evtGenseed5nEv10000.root" #testSample. skipCluster = False
# rootInputFileName = "evtGenseed7nEv10000.root"  # testSample. skipCluster = True
# rootInputFileName = "evtGenseed8nEv200000.root" # trainSample. skipCluster = True
# rootInputFileName = "evtGenseed8nEv100000.root" #evtGenSinglePassTest-TrainSample. skipCluster = True (TODO)

# rootInputFileName = "seed11nEv100pGun1_1T.root"  # test- and TrainSample
# rootInputFileName = "seed12nEv200pGun1_2T.root"  # test- and TrainSample 0-90° phi, 60-85° Theta
rootInputFileName = "seed12345nEv1000pGun1_20T.root"  # test- and TrainSample 0-90° phi, 60-85° Theta
rootInputFileName = "MyRootFile.root"  # test- and TrainSample 0-90° phi, 60-85° Theta
# rootInputFileName = "TestFile.root"

# file name into which the segNetAnalize stores its stuff
fitType = 'circleFit'  # currently supported: 'random' and 'circleFit'
# fitType = 'random'  # currently supported: 'random' and 'circleFit'
setFilterType = 'hopfield'  # currently supported: 'greedy' and 'hopfield'

usePXD = False
useDisplay = False
newTrain = False  # if true, rawSecMap-Data is collected. IF false, new TF will be executed
printNetworks = False  # creates graphs for each network for each event if true
useOldTFinstead = False  # if true, the old vxdtf is used instead of the new one.
oldTFNoSubsetSelection = True  # if true, the old vxdtf does not its hopfield-part, which allows using new modules for that
ignoreDeadTCs = True  # if true, the TrackFinderVXDAnalizer will not add dead TCs to the efficiencies
bypassCA = False  # if true, no CA will be used but the BasicPathFinder instead...

activateSegNetAnalizer = False  # only needed when studying FastBDT-behavior

doStrictSeeds = False  # if true, a smaller amount of TCs are created from the same segment-tree
doNewSubsetSelection = True  # if true, then NewSubsetSelection will be executed
doVirtualIPRemovalb4Fit = True  # if false, the vIP willbe removed after the fit
doEventSummary = True  # if true TFVXDAnalizer will produce event-wise results
switchFiltersOff = False  # if true, SegNetProducer does not apply any filters of the sectorMap.

trainFBDT = False  # with the current settings: collects samples but does not train a FastBDT!
useFBDT = False  # use the ML Filter for creating the SegmentNetwork instead of the SectorMap filters

segNetAnaRFN = 'SegNetAnalyzer_SM_train.root'
fbdtSamplesFN = 'FBDTClassifier_samples_train_10k.dat'
fbdtFN = 'FBDTClassifier_1000_3.dat'
if useFBDT:
    cNetworks = int(2)
else:
    cNetworks = int(3)

# Important parameters:

# tempStringList = rootInputFileName.split('nEv', 1)
# stringInitialValue = tempStringList[0].split("seed", 1)
# print("found seed: " + stringInitialValue[1])
# numEvents = 20  # WARNING has to be identical with the value named in rootInputFileName!
# comment Martin: this doesn't seem to be true. With
# initialValue = int(stringInitialValue[1])
initialValue = 0

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
elif (initialValue == 0):
    print("chosen initialvalue 0! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign.root']  # 23
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
    print("chosen initialvalue 5! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1120112796.root']  # lowTestRedesign_1120112796.root
elif (initialValue == 6):
    print("chosen initialvalue 6! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1120112796.root']
    # acceptedRawSecMapFiles = ['lowTestRedesign_1667035383.root'] # 26 - single track, single event  raw data
elif (initialValue == 7):
    print("chosen initialvalue 7!  (skipCluster-setting=True) " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1332084337.root']  # 27 - single track, single event raw data
elif (initialValue == 8):
    print("chosen initialvalue 8! (skipCluster-setting=True): 200k evtGen events " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1332084337.root']  # 28 - single track, single event raw data
elif (initialValue == 11):
    print("chosen initialvalue 11! (skipCluster-setting=True): 100 pGun events " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1017144726.root']  # 28 - single track, single event raw data
elif (initialValue == 12):
    print("chosen initialvalue 12! (skipCluster-setting=True): 200 pGun events " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1196763558.root']  # 28 - single track, single event raw data
elif (initialValue == 13):
    print("chosen initialvalue 13! (skipCluster-setting=True): 100k pGun events " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1874442389.root']  # 28 - single track, single event raw data
# elif (initialValue == 14):
#    print("chosen initialvalue 14! (skipCluster-setting=True): 100k pGun events " + rootInputFileName)
#    acceptedRawSecMapFiles = ['lowTestRedesign_1054912153.root']  # 28 - single track, single event raw data
elif (initialValue == 57):
    print("chosen initialvalue 57! setup remark: train: 10k events, 10 tracks per event, theta 60-85°, phi 0-360°, pT 100-145MeV.")
    acceptedRawSecMapFiles = ['lowTestRedesign_779994078.root']  # 55 - single track, single event raw data
elif (initialValue == 12345):
    print("chosen initialvalue 12345! some dummy setup!")
    acceptedRawSecMapFiles = ['lowTestRedesign_349397772.root']
else:
    print("ERROR! no valid initialvalue chosen!")
    acceptedRawSecMapFiles = [""]
    time.sleep(60)

print('')
time.sleep(5)  # sleep for 5 seconds
print('')

rootInputM = register_module('RootInput')
rootInputM.param('inputFileName', rootInputFileName)

# rootInputM.param('skipNEvents', int(10000))

eventinfoprinter = register_module('EventInfoPrinter')


gearbox = register_module('Gearbox')

secMapBootStrap = register_module('SectorMapBootstrap')
secMapBootStrap.param('ReadSectorMap', False)
secMapBootStrap.param('WriteSectorMap', True)

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
    vxdtf.param('sectorSetup', secSetup)
    vxdtf.param('GFTrackCandidatesColName', 'caTracks')
    vxdtf.param('tuneCutoffs', tuneValue)
    vxdtf.param('displayCollector', 2)
    if oldTFNoSubsetSelection:
        vxdtf.param('filterOverlappingTCs', 'none')  # shall provide overlapping TCs
        # vxdtf.param('useTimeSeedAsQI', True)  # hack for storing QIs in TimeSeed-Variable for genfit::TrackCand

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
    segNetProducer.param('allFiltersOff', switchFiltersOff)
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

    if bypassCA:
        cellOmat = register_module('TrackFinderVXDBasicPathFinder')
    else:
        cellOmat = register_module('TrackFinderVXDCellOMat')
    cellOmat.param('printNetworks', printNetworks)
    cellOmat.param('SpacePointTrackCandArrayName', 'caSPTCs')
    cellOmat.param('NetworkName', 'test2Hits')
    cellOmat.param('removeVirtualIP', False)
    cellOmat.param('strictSeeding', doStrictSeeds)
    cellOmat.logging.log_level = CAlogLevel
    cellOmat.logging.debug_level = CADebugLevel


print("spot 10")

vxdAnal = register_module('TrackFinderVXDAnalizer')
vxdAnal.param('referenceTCname', 'SPTracks')
vxdAnal.param('testTCname', 'caSPTCs')
vxdAnal.param('purityThreshold', 0.7)
vxdAnal.param('ignoreDeadTCs', ignoreDeadTCs)
vxdAnal.param('doEventSummary', doEventSummary)
vxdAnal.logging.log_level = AnalizerlogLevel
vxdAnal.logging.debug_level = AnalizerDebugLevel

print("spot 11")
if newTrain:
    log_to_file('testRedesign' + str(initialValue) + '.log', append=False)
else:
    log_to_file('testsegNetExecute' + str(initialValue) + '.log', append=False)
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

vIPRemover = register_module('SPTCvirtualIPRemover')
vIPRemover.param('maxTCLengthForVIPKeeping', 0)  # want to remove virtualIP for any track length
vIPRemover.param('tcArrayName', 'caSPTCs')

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

    if doVirtualIPRemovalb4Fit:
        main.add_module(vIPRemover)

    setup_qualityEstimators(main, fitType, 'caSPTCs', LogLevel.INFO, 1)
    # setup_qualityEstimators(main, fitType, 'SPTracks', LogLevel.DEBUG, 1)

    if doVirtualIPRemovalb4Fit is False:
        main.add_module(vIPRemover)

    if doNewSubsetSelection:

        tcNetworkProducer = register_module('SPTCNetworkProducer')
        tcNetworkProducer.param('tcArrayName', 'caSPTCs')
        tcNetworkProducer.param('tcNetworkName', 'tcNetwork')
        main.add_module(tcNetworkProducer)

        tsEvaluator = register_module('TrackSetEvaluatorHopfieldNN')
        tsEvaluator.logging.log_level = LogLevel.DEBUG
        tsEvaluator.logging.debug_level = 3
        tsEvaluator.param('tcArrayName', 'caSPTCs')
        tsEvaluator.param('tcNetworkName', 'tcNetwork')
        main.add_module(tsEvaluator)

        svdOverlapResolver = register_module('SVDOverlapResolver')
        svdOverlapResolver.param('NameSpacePointTrackCands', 'caSPTCs')
        svdOverlapResolver.param('resolveMethod', 'greedy')
        # svdOverlapResolver.param('resolveMethod', 'hopfield')
        svdOverlapResolver.logging.log_level = LogLevel.DEBUG

    main.add_module(vxdAnal)

if useDisplay:
    display = register_module('Display')
    display.param('showAllPrimaries', True)
    main.add_module(display)

# Process events
process(main)

print(statistics)
