#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This steering unpacking the CDC rawdata. -> DQM
#
######################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.INFO)

# Set your suitable DB
reset_database()
use_database_chain()
use_central_database('data_reprocessing_prompt')

# input file must be a data taken by Suppress mode of FE (Belle2 normal mode for physics run)
# Input (ROOT file).
input = register_module('RootInput')
# Input (Seq. ROOT file).
# input = register_module('SeqRootInput')

# output
unpacker = register_module('CDCUnpacker')
# FE channel <-> CDC cell ID map.
# unpacker.param('xmlMapFileName', 'ch_map.dat')
# Enable/Disable to store the RawCDC Object.
unpacker.param('enableStoreCDCRawHit', True)
# unpacker.param('enableStoreRawCDC', True)
# Enable/Disable print out the ADC/TDC data to the terminal.
# unpacker.param('enablePrintOut', True)
# Set/Unset the relation between RawCDC and CDCHit.
# unpacker.param('setRelationRaw2Hit', False)

# dqm
histo = register_module("HistoManager")  # Histogram Manager
ex1 = register_module("cdcDQM7")

histo.param("histoFileName", "cdc_histo.root")  # File to save accumulated histograms

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)
main.add_module(histo)   # Should be placed right after input module
main.add_module(ex1)

# Process all events
process(main)

print(statistics)
