#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Unpack raw data in Interim FE format v2.1 to TOPDigits and gives summary plots
# Usage: basf2 makeInterimFEDataNtuple.py <input_file.sroot>
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys
import argparse
import re
from plotInterimFEDataNtuple import plotInterimFEDataNtupleSummary

parser = argparse.ArgumentParser(description="Create ntuple file and plots from interimFE data")
parser.add_argument("inputFile", nargs='?', default="NoInputFile",
                    help="input sroot file name")
parser.add_argument("--outputFile", default="NoOutputFile",
                    help="output root file name")
parser.add_argument("--calChannel", type=int, default=0,
                    help="asic channel number where calibration signals are injected [0-7]")
parser.add_argument(
    "--lookbackWindows",
    type=int,
    default=0,
    help="lookback windows to redefine rawTime so that t=0 is beginning of search windows."
    " Give 0 (default) not to allow the redefinition.")
parser.add_argument("--noOfflineFE", action="store_true", default=False,
                    help="Use only online FE hits, and do not use waveform data")
parser.add_argument("--saveWaveform", action="store_true", default=False,
                    help="Save waveform data in the output ntuple file")
parser.add_argument("--globalDAQ", action="store_true", default=False,
                    help="Force to assume global DAQ data.")
parser.add_argument("--pocketDAQ", action="store_true", default=False,
                    help="Force to assume global DAQ data.")
parser.add_argument("--skipPlot", action="store_true", default=False,
                    help="Skip making summary plot.")
args = parser.parse_args()

if args.inputFile == "NoInputFile":
    print("Create a flat ntuple file from sroot data file(s) taken with interimFE firmware and plots for quick data quality check.")
    print("usage:")
    print("basf2 makeInterimFEDataNtuple.py (input_filename.sroot) [--arg --outputFile output_ntuple.root]")
    print("                                  [--arg --calChannel asicCh]")
    print("                                  [--arg --lookbackWindows windows]")
    print("                                  [--arg --noOfflineFE] [--arg --saveWaveform]")
    print("                                  [--arg --globalDAQ] [--arg --pocketDAQ]")
    print("                                  [--arg --skipPlot]")
    print("*Switching of local/global run and output file name is automatically given as folows if it is not specified:")
    print("  runXXXXXX_slotYY_ntuple.root (local run with PocketDAQ)")
    print(" or top.XXXXX.YYYYYY_ntuple.root (global run with global DAQ)")
    print("*Deafult asic channel number with calibration signals is 0 (can be changed with \"--calChannel\" option.)")
    print("*Deafult the number of lookback windows is 0, with which timing correction is not applied.")
    print(" (rawTime is corrected so that it is measured with respect to the start of search windows when lookbackWindows>0.)")
    print("*Option \"--noOfflineFE\"  : disable offline FE from waveform data.")
    print("                             Calculation of reference timing is based on hit in calibration channel.")
    print("*       \"--saveWaveform\" : save waveform data in the output ntuple file")
    print("*       \"--globalDAQ\"    : force to assume global DAQ data as an input file")
    print("*       \"--pocketDAQ\"    : force to assume pocketDAQ data as an input file")
    print("*       \"--skipPlot\"     : only processing sroot data file, do not create summary plot")
    sys.exit()

inputFile = args.inputFile
isGlobalDAQ = False
isOfflineFEDisabled = args.noOfflineFE
isGlobalDAQForced = args.globalDAQ
isPocketDAQForced = args.pocketDAQ
calCh = args.calChannel
lookbackWindows = args.lookbackWindows
if calCh < 0 or calCh > 7:
    print("ERROR : invalid calibration asic channel :" + str(calCh))
    print("        (should be [0-7])")
    sys.exit()

if re.search(r"run[0-9]+_slot[0-1][0-9]", inputFile):
    outputRoot = re.search(r"run[0-9]+_slot[0-1][0-9]", inputFile).group() + "_ntuple.root"
elif re.search(r"(top|cosmic|cdc|ecl|klm|test)\.[0-9]+\.[0-9]+", inputFile):
    isGlobalDAQ = True
    outputRoot = re.search(
        r"(top|cosmic|cdc|ecl|klm|test)\.[0-9]+\.[0-9]+",
        inputFile).group() + "_ntuple.root"
else:
    outputRoot = inputFile + "_ntuple.root"

if args.outputFile != "NoOutputFile":
    outputRoot = args.outputFile

if isGlobalDAQForced and (not isPocketDAQForced):
    isGlobalDAQ = True
elif (not isGlobalDAQForced) and isPocketDAQForced:
    isGlobalDAQ = False
elif isGlobalDAQForced and isPocketDAQForced:
    print("ERROR : both of --GlobalDAQ or --PocketDAQ can not be given.")
    sys.exit()

print(inputFile + " --> " + outputRoot)
print("Is global DAQ?        : " + str(isGlobalDAQ))
print("OfflineFE             : " + str(not isOfflineFEDisabled))
print("Save waveform?        : " + str(args.saveWaveform))
print("Cal. asic ch          : " + str(calCh))
print("# of lookback windows : " + str(lookbackWindows))
print()
print("start process...")


# data base
reset_database()
path_to_db = "/group/belle2/group/detector/TOP/calibration/combined/Combined_TBCrun417x_LocaT0run4855/localDB"
use_local_database(path_to_db + '/localDB.txt', path_to_db)

# Create path
main = create_path()

roinput = register_module('SeqRootInput')
roinput.param('inputFileName', inputFile)
main.add_module(roinput)

# HistoManager
histoman = register_module('HistoManager')
histoman.param('histoFileName', outputRoot)
main.add_module(histoman)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if not isGlobalDAQ:
    converter = register_module('Convert2RawDet')
    main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking (format auto detection should work now)
unpack = register_module('TOPUnpacker')
# unpack.param('swapBytes', True)
# unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
if not isOfflineFEDisabled:
    featureExtractor = register_module('TOPWaveformFeatureExtractor')
    main.add_module(featureExtractor)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', True)
converter.param('useChannelT0Calibration', True)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('lookBackWindows', lookbackWindows)
converter.param('storageDepth', 508)
converter.param('calibrationChannel', calCh)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 7.0)  # in [ns]
main.add_module(converter)

ntuple = register_module('TOPInterimFENtuple')
ntuple.param('saveWaveform', (args.saveWaveform))
ntuple.param('useDoublePulse', (not isOfflineFEDisabled))
ntuple.param('calibrationChannel', calCh)
# ntuple.param('minHeightFirstCalPulse', 600)  # in [ADC counts]
# ntuple.param('minHeightSecondCalPulse', 450)  # in [ADC counts]
ntuple.param('minHeightFirstCalPulse', 300)  # in [ADC counts]
ntuple.param('minHeightSecondCalPulse', 300)  # in [ADC counts]
ntuple.param('nominalDeltaT', 21.88)  # in [ns]
ntuple.param('nominalDeltaTRange', 5)  # in [ns]
ntuple.param('globalRefSlotNum', 1)
ntuple.param('globalRefAsicNum', 0)
ntuple.param('timePerWin', 23.581939)  # in [ns]
main.add_module(ntuple)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)

if not args.skipPlot:
    plotInterimFEDataNtupleSummary(outputRoot, 2, isOfflineFEDisabled)
