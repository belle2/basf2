#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

from VXDTF.setup_modules import (setup_gfTCtoSPTCConverters,
                                 setup_spCreatorPXD,
                                 setup_spCreatorSVD,
                                 setup_sp2thConnector)

from VXDTF.setup_modules_ml import *

# ################
rootInputFileName = "seed2nEv100pGun1_10T.root"

# file name into which the segNetAnalize stores its stuff
segNetAnaRFN = 'SegNetAnalyzer_SM_train.root'
fbdtSamplesFN = 'FBDTClassifier_samples_train_10k.dat'
fbdtFN = 'FBDTClassifier_1000_3.dat'


usePXD = False
useDisplay = False
newTrain = False  # if true, rawSecMap-Data is collected. IF false, new TF will be executed
printNetworks = False  # creates graphs for each network for each event if true
trainFBDT = False  # with the current settings: collects samples but does not train a FastBDT!
useFBDT = False  # use the ML Filter for creating the SegmentNetwork instead of the SectorMap filters

if useFBDT:
    cNetworks = int(2)
else:
    cNetworks = int(3)

# Important parameters:
# seed42nEv1000pGun1_10T.root
tempStringList = rootInputFileName.split('nEv', 1)
stringInitialValue = tempStringList[0].split("seed", 1)
print("found seed: " + stringInitialValue[1])
numEvents = 2  # WARNING has to be identical with the value named in rootInputFileName!
initialValue = int(stringInitialValue[1])

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

trainerVXDTFLogLevel = LogLevel.ERROR
trainerVXDTFDebugLevel = 10

TFlogLevel = LogLevel.ERROR
TFDebugLevel = 1

CAlogLevel = LogLevel.ERROR
CADebugLevel = 1

AnalizerlogLevel = LogLevel.ERROR
AnalizerDebugLevel = 1


if (initialValue == 2):
    print("chosen initialvalue 2! " + rootInputFileName)
    acceptedRawSecMapFiles = ['lowTestRedesign_1373026662.root']  # 42
elif (initialValue == 23):
    print("chosen initialvalue 23!")
    acceptedRawSecMapFiles = ['lowTestRedesign_216235540.root']  # 23
elif (initialValue == 24):
    print("chosen initialvalue 24!")
    acceptedRawSecMapFiles = ['lowTestRedesign_374830533.root']  # 24
elif (initialValue == 25):
    print("chosen initialvalue 25!")
    acceptedRawSecMapFiles = ['lowTestRedesign_753986291.root']  # 25
elif (initialValue == 26):
    print("chosen initialvalue 26! setup remark: single track, single event raw data")
    # acceptedRawSecMapFiles = ['lowTestRedesign_1667035383.root'] # 26 - single track, single event raw data
    acceptedRawSecMapFiles = ['lowTestRedesign_249371033.root']  # 26 - single track, single event raw data
elif (initialValue == 27):
    print("chosen initialvalue 27! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_1865959838.root']  # 27 - single track, single event raw data
elif (initialValue == 28):
    print("chosen initialvalue 28! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_1406543755.root']  # 28 - single track, single event raw data
elif (initialValue == 29):
    print("chosen initialvalue 29! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_985902979.root']  # 29 - single track, single event raw data
elif (initialValue == 30):
    print("chosen initialvalue 30! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_1987088791.root']  # 30 - single track, single event raw data
elif (initialValue == 31):
    print("chosen initialvalue 31! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_713253727.root']  # 31 - single track, single event raw data
elif (initialValue == 32):
    print("chosen initialvalue 32! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_1403360988.root']  # 32 - single track, single event raw data
elif (initialValue == 33):
    print("chosen initialvalue 33! setup remark: single track, single event raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_574263477.root']  # 33 - single track, single event raw data
elif (initialValue == 55):
    print("chosen initialvalue 55! setup remark: single track, 10 events raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_1138928103.root']  # 55 - single track, single event raw data
elif (initialValue == 56):
    print("chosen initialvalue 56! setup remark: single track, 1000 events raw data")
    acceptedRawSecMapFiles = ['lowTestRedesign_922296899.root']  # 55 - single track, single event raw data
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
if newTrain:
    newSecMapTrainerBase = register_module('SecMapTrainerBase')
    newSecMapTrainerBase.logging.log_level = trainerVXDTFLogLevel
    newSecMapTrainerBase.logging.debug_level = trainerVXDTFDebugLevel
    newSecMapTrainerBase.param('spTCarrayName', 'checkedSPTCs')
    newSecMapTrainerBase.param('allowTraining', True)
else:
    merger = register_module('RawSecMapMerger')
    merger.logging.log_level = trainerVXDTFLogLevel
    merger.logging.debug_level = trainerVXDTFDebugLevel
    merger.param('rootFileNames', acceptedRawSecMapFiles)
    # merger.param('spTCarrayName', 'checkedSPTCs')


geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.ERROR
eventCounter.param('stepSize', 1)


segNetProducer = register_module('SegmentNetworkProducer')
segNetProducer.param('CreateNeworks', cNetworks)
segNetProducer.param('NetworkOutputName', 'test2Hits')
segNetProducer.param('printNetworks', printNetworks)
# segNetProducer.param('SpacePointsArrayNames', ['pxdOnly', 'nosingleSP'])
segNetProducer.param('SpacePointsArrayNames', ['nosingleSP_relTH'])
segNetProducer.logging.log_level = TFlogLevel
segNetProducer.logging.debug_level = TFDebugLevel

segNetAnalyzer = register_module('SegmentNetworkAnalyzer')
segNetAnalyzer.param('networkInputName', 'test2Hits')
segNetAnalyzer.param('rootFileName', segNetAnaRFN)
segNetAnalyzer.logging.log_level = LogLevel.INFO
segNetAnalyzer.logging.debug_level = 100

cellOmat = register_module('TrackFinderVXDCellOMat')
cellOmat.param('printNetworks', False)
cellOmat.param('SpacePointTrackCandArrayName', 'caSPTCs')
cellOmat.param('NetworkName', 'test2Hits')
cellOmat.logging.log_level = CAlogLevel
cellOmat.logging.debug_level = CADebugLevel


vxdAnal = register_module('TrackFinderVXDAnalizer')
vxdAnal.param('referenceTCname', 'SPTracks')
vxdAnal.param('testTCname', 'caSPTCs')
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
    main.add_module(merger)
    main.add_module(segNetProducer)
    if trainFBDT:  # collect in this step
        add_fbdtclassifier_training(main, 'test2Hits', 'FBDTClassifier.dat', False, True,
                                    False, fbdtSamplesFN, 100, 3, 0.15, 0.5, LogLevel.DEBUG, 10)
    if useFBDT:  # apply the filters
        add_ml_threehitfilters(main, 'test2Hits', fbdtFN, 0.989351, True)
    main.add_module(segNetAnalyzer)
    main.add_module(cellOmat)
    main.add_module(vxdAnal)

if useDisplay:
    display = register_module('Display')
    display.param('showAllPrimaries', True)
    main.add_module(display)
# Process events
process(main)

print(statistics)
