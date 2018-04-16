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
slotId = args.slotID
pmtId = args.PMTID
calChannel = args.calChannel
threshold = args.threshold
isGlobalDAQ = False
isGlobalDAQForced = args.globalDAQ
isPocketDAQForced = args.pocketDAQ
isOfflineFEDisabled = args.noOfflineFE
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

inputBase = inputFiles[0] if not skipFirst else interimRoot
dotPos = inputBase.rfind('.')
outputBase = inputBase[0:dotPos] if (dotPos > 0) else inputBase

if re.search(r"run[0-9]+_slot[0-1][0-9]", inputFiles[0]):
    outputBase = re.search(r"run[0-9]+_slot[0-1][0-9]", inputFiles[0]).group()
elif re.search(r"(top|cosmic|cdc|ecl|klm|test)\.[0-9]+\.[0-9]+", inputFiles[0]):
    isGlobalDAQ = True
    outputBase = re.search(r"(top|cosmic|cdc|ecl|klm|test)\.[0-9]+\.[0-9]+", inputFiles[0]).group()

if interimRoot is "NoInterimRootFile":
    interimRoot = outputBase + "_gain_histo.root"
if outputRoot is "NoOutputRootFile":
    outputRoot = outputBase + "_gain_" + pmtStr + ".root"
if outputPDF is "NoOutputPDFFile":
    outputPDF = outputBase + "_gain_" + pmtStr + ".pdf"

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

    # Unpacking (format auto detection works now)
    unpack = register_module('TOPUnpacker')
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
    converter.param('calpulseHeightMin', 450)  # in [ADC counts]
    converter.param('calpulseHeightMax', 900)  # in [ADC counts]
    converter.param('calpulseWidthMin', 2.0)  # in [ns]
    converter.param('calpulseWidthMax', 6.0)  # in [ns]
    first.add_module(converter)

    laserHitSelector = register_module('TOPLaserHitSelector')
    laserHitSelector.param('useDoublePulse', (not useSingleCalPulse))
    laserHitSelector.param('minHeightFirstCalPulse', 600)  # in [ADC counts]
    laserHitSelector.param('minHeightSecondCalPulse', 450)  # in [ADC counts]
    laserHitSelector.param('nominalDeltaT', 21.85)  # in [ns]
    laserHitSelector.param('nominalDeltaTRange', 2)  # in [ns]
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
    analysis.param('threshold', threshold)
    second.add_module(analysis)

    process(second)

# Print statistics
print(statistics)
