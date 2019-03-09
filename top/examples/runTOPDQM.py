#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

set_log_level(LogLevel.ERROR)

# Define a global tag (note: the one given bellow will become out-dated!)
use_central_database('data_reprocessing_proc8')

# Create path
main = create_path()

# input: raw data
roinput = register_module('RootInput')
# roinput = register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP (uncomment for pocketDAQ!)
# converter = register_module('Convert2RawDet')
# main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = register_module('TOPUnpacker')
main.add_module(unpack)

# Convert to TOPDigits
converter = register_module('TOPRawDigitConverter')
main.add_module(converter)

# DQM setup
histo = register_module("HistoManager")
main.add_module(histo)

dqm = register_module('TOPDQM')
main.add_module(dqm)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
