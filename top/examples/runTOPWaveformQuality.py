#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Display of waveforms with feature extraction points superimposed
# Unpacker is set for Interim FE format v2.1
# ---------------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False
from ROOT import gROOT
gROOT.SetBatch()

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH1F, TCanvas, TGraph

import argparse
parser = argparse.ArgumentParser(description='Go through a data file, apply calibration, and write the waveforms to a root file.',
                                 usage='%(prog)s [options]')

# parser.add_argument(
#     '--inputRun',
#     metavar='InputRun',
#     required=True,
#     help='the name for the input data files.')
#
args = parser.parse_args()

set_log_level(LogLevel.INFO)

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP
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

# TOP's data quality module
histomanager = register_module("HistoManager")
main.add_module(histomanager)
wfqp = register_module('TOPWaveformQualityPlotter')
main.add_module(wfqp)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
