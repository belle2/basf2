#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# TOP MCP-PMT channel-by-channel gain/efficiency anslysis using laser data
# Usage: basf2 analyzeGainEff.py <input_file.sroot> --arg --slot (slotNum) --arg --PMT (PMTNum)
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys
import os
import argparse
import re

parser = argparse.ArgumentParser(description="analyze gain and efficiency for laser run data")
parser.add_argument("inputFile", nargs='*', default=["NoInputFile"],
                    help="Input sroot files. Mulitple files can be given.")
parser.add_argument("--interimRootFile", default="NoInterimRootFile",
                    help="Interim root file name to store timing-height 2D histograms.")
parser.add_argument("--outputRootFile", default="NoOutputRootFile",
                    help="Output root file name to save TTree containing fit results.")
parser.add_argument("--outputPDFFile", default="NoOutputPDFFile",
                    help="Output PDF file name to save fitting results for each channel.")
parser.add_argument("--interimFW", action="store_true", default=False,
                    help="use when analyzing interimFW data")
parser.add_argument("--slotID", type=int, default=0,
                    help="slot number [1-16]")
parser.add_argument("--PMTID", type=int, default=0,
                    help="slot number [1-32]")
parser.add_argument("--calChannel", type=int, default=0,
                    help="asic channel number where calibration signals are injected [0-7]")
parser.add_argument("--threshold", type=float, default=100.,
                    help="threshold in pulse height fitting")
parser.add_argument("--globalDAQ", action="store_true", default=False,
                    help="Force to assume global DAQ data.")
parser.add_argument("--pocketDAQ", action="store_true", default=False,
                    help="Force to assume global DAQ data.")
parser.add_argument("--noOfflineFE", action="store_true", default=False,
                    help="Use only online FE hits, and do not use waveform data.")
parser.add_argument("--useSingleCalPulse", action="store_true", default=False,
                    help="Do not require double calibration pulses, but require only the first one.")
parser.add_argument("--windowSelection", type=int, default=0,
                    help="Select window number (All=0,Odd=2,Even=1)")
parser.add_argument("--includePrimaryChargeShare", action="store_true", default=False,
                    help="don't want to use primary charge share cut,type true.")
parser.add_argument("--includeAllChargeShare", action="store_true", default=False,
                    help="don't want to use all charge share cut,type true.")
parser.add_argument("--sumChargeShare", action="store_true", default=False,
                    help="When you want to use summation of charge share for gain distribution,type true.")
parser.add_argument("--fitoption", action="store", default='L',
                    help="fitting with chisquare=R, loglikelihood=L")
parser.add_argument("--timeCut", type=float, default=1,
                    help="cut of hittiming for chargeshare")
parser.add_argument("--HVHigherSetting", type=int, default=0,
                    help="HV positive difference from nominal value")
parser.add_argument("--HVLowerSetting", type=int, default=0,
                    help="HV negative difference from nominal value")
args = parser.parse_args()

if (args.inputFile[0] == "NoInputFile") and (args.interimRootFile == "NoInterimRootFile"):
    print("Steering file to study gain/efficiency analysis from laser data.")
    print("In the first step, 2D histograms (hit time vs pulse height) were created for all the available channels, "
          "and saved in an interim root file.")
    print("Then, in the second process, 1D pulse height distribution is extracted "
          "from the 2D histogram and fitted to evaluate gain and efficiency for each channel.")
    print("The second process is done only for one give PMT as it takes time.")
    print("usage:")
    print("basf2 analyzeGainEff.py [input_filename1.sroot, input_filename2.sroot, ...]")
    print("                        [--arg --interimRootFile interim_histo_output.root]")
    print("                        [--arg --outputRootFile summary_tree_output.root]")
    print("                        [--arg --outputPDFFile summary_plot.pdf]")
    print("                        [--arg --slotID slotNum(1-16)] [--arg --PMTID PMTID(1-32)]")
    print("                        [--arg --calChannel asicCh(0-7)] [--arg --threshold threshold]")
    print("                        [--arg --globalDAQ] [--arg --pocketDAQ]")
    print("                        [--arg --noOfflineFE] [--arg --useSingleCalPulse]")
    print("                        [--arg --timeCut] [--arg --fitoption]")
    print("                        [--arg --windowSelection] [--arg --sumChargeShare]")
    print("                        [--arg --includePrimaryChargeShare] [--arg --includeAllChargeShare]")
    print("                        [--arg --HVHigherSetting] [--arg --HVLowerSetting]")
    print("*When input sroot files are missing but interim root file is given,"
          " skip the first process to create 2D histogram and start fitting.")
    print("*Both the slot and PMT numbers are mandatory to proceed to the second processes.")
    print("*default calibration asic channel is " + str(args.calChannel))
    print("*default threshold is " + str(args.threshold))
    print()
    sys.exit()

inputFiles = args.inputFile
interimRoot = args.interimRootFile
outputRoot = args.outputRootFile
outputPDF = args.outputPDFFile
isInterimFW = args.interimFW
slotId = args.slotID
pmtId = args.PMTID
calChannel = args.calChannel
threshold = args.threshold
isGlobalDAQ = False
isGlobalDAQForced = args.globalDAQ
isPocketDAQForced = args.pocketDAQ
isOfflineFEDisabled = args.noOfflineFE
windowSelect = args.windowSelection
includePrimaryChargeShare = args.includePrimaryChargeShare
includeAllChargeShare = args.includeAllChargeShare
sumChargeShare = args.sumChargeShare
HVHigherSetting = args.HVHigherSetting
HVLowerSetting = args.HVLowerSetting
fitOption = args.fitoption
timeCut = args.timeCut

useSingleCalPulse = (True if isOfflineFEDisabled else args.useSingleCalPulse)
skipFirst = ((inputFiles[0] == "NoInputFile") and (interimRoot != "NoInterimRootFile"))
skipSecond = ((slotId < 1) or (slotId > 16) or (pmtId < 1) or (pmtId > 32))
pmtStr = "s" + ('%02d' % slotId) + "_PMT" + ('%02d' % pmtId)
if isGlobalDAQForced and isPocketDAQForced:
    print("ERROR : both of --GlobalDAQ or --PocketDAQ can not be given.")
    sys.exit()
if skipFirst and skipSecond:
    print("ERROR : unable to run either of the first and second process:"
          "no input sroot file and no valid slot ID or PMT ID")
    sys.exit()
if (calChannel < 0) or (calChannel > 7):
    print("ERROR : invalid asic channel with calibration pulses : " + str(calChannel))
    sys.exit()

if includePrimaryChargeShare and includeAllChargeShare:
    print("ERROR : both of --includePrimaryChargeShare and --includeAllChargeShare can not be given.")
    sys.exit()

if sumChargeShare and includeAllChargeShare:
    print("ERROR : both of --sumChargeShare and --includeAllChargeShare can not be given."
          "While both of --sumChargeShare and --includePrimaryChargeShare can be given.")
    sys.exit()

if HVHigherSetting and HVLowerSetting:
    print("ERROR : both of --HVHigherSetting and --HVLowerSetting can not be given.")
    sys.exit()

# data base
reset_database()
path_to_db = "/group/belle2/group/detector/TOP/calibration/combined/Combined_TBCrun417x_LocaT0run4855_AfterRelease01/localDB/"
use_local_database(path_to_db + '/localDB.txt', path_to_db)

inputBase = inputFiles[0] if not skipFirst else interimRoot
dotPos = inputBase.rfind('.')
outputBase = inputBase[0:dotPos] if (dotPos > 0) else inputBase

if re.search(r"run[0-9]+_slot[0-1][0-9]", inputFiles[0]):
    outputBase = re.search(r"run[0-9]+_slot[0-1][0-9]", inputFiles[0]).group()
elif re.search(r"(top|cosmic|cdc|ecl|klm|test|debug|beam)\.[0-9]+\.[0-9]+", inputFiles[0]):
    isGlobalDAQ = True
    outputBase = re.search(r"(top|cosmic|cdc|ecl|klm|test|debug|beam)\.[0-9]+\.[0-9]+", inputFiles[0]).group()

if interimRoot is "NoInterimRootFile":
    interimRoot = outputBase + "_gain_histo.root"
if outputRoot is "NoOutputRootFile":
    outputRoot = outputBase + "_gain_" + pmtStr + ".root"
if outputPDF is "NoOutputPDFFile":
    outputPDF = outputBase + "_" + pmtStr

if isGlobalDAQForced and (not isPocketDAQForced):
    isGlobalDAQ = True
elif (not isGlobalDAQForced) and isPocketDAQForced:
    isGlobalDAQ = False

if not skipFirst:
    print("*first process  : " + str(inputFiles) + " --> " + interimRoot)
else:
    print("*first process is skipped...")
if not skipSecond:
    print("*second process : " + interimRoot + " --> " + outputRoot + ", " + outputPDF)
else:
    print("*second process is skipped")
print("*Is global DAQ?   : " + str(isGlobalDAQ))
print("*Offline FE       : " + ("enabled" if not isOfflineFEDisabled else "disabled"))
print("*use double pulse : " + str(not useSingleCalPulse))
print("*cal. asic channel: " + str(calChannel))
print("*threshold        : " + str(threshold))
print()
print("start process...")


if not skipFirst:
    # Create path
    first = create_path()

    srootInput = register_module('SeqRootInput')
    srootInput.param('inputFileNames', inputFiles)
    first.add_module(srootInput)

    # HistoManager
    histoman = register_module('HistoManager')
    histoman.param('histoFileName', interimRoot)
    first.add_module(histoman)

    # conversion from RawCOPPER or RawDataBlock to RawDetector objects
    if not isGlobalDAQ:
        converter = register_module('Convert2RawDet')
        first.add_module(converter)

    # geometry parameters
    gearbox = register_module('Gearbox')
    first.add_module(gearbox)

    # Geometry (only TOP needed)
    geometry = register_module('Geometry')
    geometry.param('components', ['TOP'])
    first.add_module(geometry)

    # Unpacking
    unpack = register_module('TOPUnpacker')
    if isInterimFW:
        unpack.param('swapBytes', True)
        unpack.param('dataFormat', 0x0301)
    first.add_module(unpack)

    # Add multiple hits by running feature extraction offline
    if not isOfflineFEDisabled:
        featureExtractor = register_module('TOPWaveformFeatureExtractor')
        first.add_module(featureExtractor)

    # Convert to TOPDigits
    converter = register_module('TOPRawDigitConverter')
    converter.param('useSampleTimeCalibration', False)
    converter.param('useChannelT0Calibration', False)
    converter.param('useModuleT0Calibration', False)
    converter.param('useCommonT0Calibration', False)
    converter.param('calibrationChannel', calChannel)  # if set, cal pulses will be flagged
    converter.param('calpulseHeightMin', 100)  # in [ADC counts]
    converter.param('calpulseHeightMax', 650)  # in [ADC counts]
    converter.param('calpulseWidthMin', 0.8)  # in [ns]
    converter.param('calpulseWidthMax', 2.4)  # in [ns]
    first.add_module(converter)

    flagSetter = register_module('TOPXTalkChargeShareSetter')
    flagSetter.param('sumChargeShare', sumChargeShare)
    flagSetter.param('timeCut', timeCut)  # in [nsec]
    first.add_module(flagSetter)

    laserHitSelector = register_module('TOPLaserHitSelector')
    laserHitSelector.param('useDoublePulse', (not useSingleCalPulse))
    laserHitSelector.param('minHeightFirstCalPulse', 300)  # in [ADC counts]
    laserHitSelector.param('minHeightSecondCalPulse', 100)  # in [ADC counts]
    laserHitSelector.param('nominalDeltaT', 25.5)  # in [ns]
    laserHitSelector.param('nominalDeltaTRange', 2)  # in [ns]
    laserHitSelector.param('windowSelect', windowSelect)
    laserHitSelector.param('includePrimaryChargeShare', includePrimaryChargeShare)
    laserHitSelector.param('includeAllChargeShare', includeAllChargeShare)
    # laserHitSelector.param('timeHistogramBinning', [100,-150,-50]) # number of bins, lower limit, upper limit
    first.add_module(laserHitSelector)

    # Print progress
    progress = register_module('Progress')
    first.add_module(progress)

    # Process events
    process(first)

if not skipSecond:
    # second process
    print("start the second process...")
    second = create_path()

    eventinfosetter = register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
    second.add_module(eventinfosetter)

    # HistoManager
    histoman2 = register_module('HistoManager')
    histoman2.param('histoFileName', outputRoot)
    second.add_module(histoman2)

    analysis = register_module('TOPGainEfficiencyCalculator')
    analysis.param('inputFile', interimRoot)
    analysis.param('outputPDFFile', outputPDF)
    analysis.param('targetSlotId', slotId)
    analysis.param('targetPmtId', pmtId)
    analysis.param('hvDiff', HVHigherSetting - HVLowerSetting)
    analysis.param('threshold', threshold)
    analysis.param('fitoption', fitOption)
    second.add_module(analysis)

    process(second)

# Print statistics
print(statistics)
