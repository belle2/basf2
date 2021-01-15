#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Display of waveforms with feature extraction points superimposed
# Unpacker is set for Interim FE format v2.1
# ---------------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False  # noqa
from ROOT import gROOT
gROOT.SetBatch()  # noqa

import basf2 as b2

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

b2.set_log_level(b2.LogLevel.INFO)

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP
converter = b2.register_module('Convert2RawDet')
main.add_module(converter)

# geometry parameters
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = b2.register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# TOP's data quality module
histomanager = b2.register_module("HistoManager")
main.add_module(histomanager)
wfqp = b2.register_module('TOPWaveformQualityPlotter')
main.add_module(wfqp)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
