#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

b2.set_log_level(b2.LogLevel.ERROR)

# Define a global tag (note: the one given bellow will become out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Create path
main = b2.create_path()

# input: raw data
roinput = b2.register_module('RootInput')
# roinput = register_module('SeqRootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawTOP (uncomment for pocketDAQ!)
# converter = register_module('Convert2RawDet')
# main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
main.add_module(unpack)

# Convert to TOPDigits
converter = b2.register_module('TOPRawDigitConverter')
main.add_module(converter)

# DQM setup
histo = b2.register_module("HistoManager")
main.add_module(histo)

dqm = b2.register_module('TOPDQM')
main.add_module(dqm)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
