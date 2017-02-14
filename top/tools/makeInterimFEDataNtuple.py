#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Unpack raw data in Interim FE format v2.1 to TOPDigits and gives summary plots
# Usage: basf2 MY_TOPInterimFECheck.py <input_file.sroot>
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys
import re
from plotInterimFEDataNtuple import plotInterimFEDataNtuple

args = sys.argv
if len(args) < 2:
    print("usage:")
    print(args[0] + " (input_filename.sroot) [output_file.root]")
    print("*if output filename is not given, output file name is automatically set as folows :")
    print("  runXXXXXX_slotYY_ana.root")
    sys.exit()

input_file = args[1]
matchtest = re.search(r"run[0-9]*_slot[0-1][0-9]", input_file).group()
if matchtest:
    output_root = matchtest + "_ana.root"
else:
    output_root = input_file + "_ana.root"
print(input_file + " --> " + output_root)

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

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
converter.param('useSampleTimeCalibration', False)
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
main.add_module(converter)

ana = register_module('TOPInterimFENtuple')
main.add_module(ana)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)

plotInterimFEDataNtuple(output_root)
