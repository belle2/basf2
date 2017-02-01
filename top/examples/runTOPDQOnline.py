#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from basf2 import *
import sys
import argparse
from ROOT import Belle2
from ROOT import TH1F, TCanvas, TGraph

set_log_level(LogLevel.ERROR)

parser = argparse.ArgumentParser(description='Go through a sroot file, show the waveforms in real time.',
                                 usage='%(prog)s [options]')

parser.add_argument(
    '--inputRun',
    metavar='InputRun (i.e. file name = InputRun.sroot)',
    required=True,
    help='the name for the input data files.')

args = parser.parse_args()

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
roinput.param('inputFileName', args.inputRun)
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

# Unpacking
unpack = register_module('TOPUnpacker')
unpack.param('swapBytes', True)
unpack.param('dataFormat', 0x0301)
main.add_module(unpack)

# TOP's data quality module
histomanager = register_module("HistoManager")
main.add_module(histomanager)
dqm = register_module('TOPDataQualityOnline')
main.add_module(dqm)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
