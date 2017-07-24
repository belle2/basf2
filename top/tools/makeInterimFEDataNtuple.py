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

input_file = args.inputFile
IsGlobalDAQ = False
IsOfflineFEDisabled = args.noOfflineFE
IsGlobalDAQForced = args.globalDAQ
IsPocketDAQForced = args.pocketDAQ
CalCh = args.calChannel
lookbackWindows = args.lookbackWindows
if CalCh < 0 or CalCh > 7:
    print("ERROR : invalid calibration asic channel :" + str(CalCh))
    print("        (should be [0-7])")
    sys.exit()

if re.search(r"run[0-9]+_slot[0-1][0-9]", input_file):
    output_root = re.search(r"run[0-9]+_slot[0-1][0-9]", input_file).group() + "_ntuple.root"
elif re.search(r"(top|cosmic|cdc|ecl|klm|test)\.[0-9]+\.[0-9]+", input_file):
    IsGlobalDAQ = True
    output_root = re.search(
        r"(top|cosmic|cdc|ecl|klm|test)\.[0-9]+\.[0-9]+",
        input_file).group() + "_ntuple.root"
else:
    output_root = input + "_ntuple.root"

if args.outputFile != "NoOutputFile":
    output_root = args.outputFile

if IsGlobalDAQForced and (not IsPocketDAQForced):
    IsGlobalDAQ = True
elif (not IsGlobalDAQForced) and IsPocketDAQForced:
    IsGlobalDAQ = False
elif IsGlobalDAQForced and IsPocketDAQForced:
    print("ERROR : both of --GlobalDAQ or --PocketDAQ can not be given.")
    sys.exit()

print(input_file + " --> " + output_root)
print("Is global DAQ? : " + str(IsGlobalDAQ))
print("OfflineFE : " + str(not IsOfflineFEDisabled))
print("Save waveform? : " + str(args.saveWaveform))
print("Cal. asic ch : " + str(CalCh))
print("# of lookback windows : " + str(lookbackWindows))
print()
print("start process...")

# Create path
main = create_path()

roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
roinput.param('inputFileName', input_file)
main.add_module(roinput)

# HistoManager
histoman = register_module('HistoManager')
histoman.param('histoFileName', output_root)
main.add_module(histoman)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if not IsGlobalDAQ:
    converter = register_module('Convert2RawDet')
    main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking
unpack = register_module('TOPUnpacker')
unpack.param('swapBytes', True)
unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# Add multiple hits by running feature extraction offline
if not IsOfflineFEDisabled:
    featureExtractor = register_module('TOPWaveformFeatureExtractor')
    main.add_module(featureExtractor)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('lookBackWindows', lookbackWindows)
converter.param('storageDepth', 510)
converter.param('calibrationChannel', CalCh)  # if set, cal pulses will be flagged
converter.param('calpulseHeightMin', 450)  # in [ADC counts]
converter.param('calpulseHeightMax', 900)  # in [ADC counts]
converter.param('calpulseWidthMin', 2.0)  # in [ns]
converter.param('calpulseWidthMax', 6.0)  # in [ns]
main.add_module(converter)

ntuple = register_module('TOPInterimFENtuple')
ntuple.param('saveWaveform', (args.saveWaveform))
ntuple.param('useDoublePulse', (not IsOfflineFEDisabled))
ntuple.param('calibrationChannel', CalCh)
# ntuple.param('averageSamplingRate', 2.71394)
ntuple.param('minHeightFirstCalPulse', 600)  # in [ADC counts]
ntuple.param('minHeightSecondCalPulse', 450)  # in [ADC counts]
ntuple.param('nominalDeltaT', 21.85)  # in [ns]
ntuple.param('nominalDeltaTRange', 2)  # in [ns]
main.add_module(ntuple)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)

if not args.skipPlot:
    plotInterimFEDataNtupleSummary(output_root, 2, IsOfflineFEDisabled)
    # plotInterimFEDataNtupleAdvanced(output_root)
