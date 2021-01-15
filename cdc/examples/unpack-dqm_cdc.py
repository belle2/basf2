#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This steering unpacking the CDC rawdata. -> DQM
#
######################################################

import basf2 as b2

# Set the log level to show only error and fatal messages
b2.set_log_level(b2.LogLevel.INFO)

# Set your suitable DB
b2.reset_database()
b2.use_database_chain()
b2.use_central_database('data_reprocessing_prompt')

# input file must be a data taken by Suppress mode of FE (Belle2 normal mode for physics run)
# Input (ROOT file).
input = b2.register_module('RootInput')
# Input (Seq. ROOT file).
# input = register_module('SeqRootInput')

# output
unpacker = b2.register_module('CDCUnpacker')
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
histo = b2.register_module("HistoManager")  # Histogram Manager
ex1 = b2.register_module("cdcDQM7")

histo.param("histoFileName", "cdc_histo.root")  # File to save accumulated histograms

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)
main.add_module(histo)   # Should be placed right after input module
main.add_module(ex1)

# Process all events
b2.process(main)

print(b2.statistics)
