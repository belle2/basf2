#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Make DQM histograms for TOP (hit-level only) using raw data
#
# Usage: basf2 runTOPDQM.py -i <raw_data_file>
# ---------------------------------------------------------------------------------------

import basf2 as b2

# Database
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('online')
# b2.conditions.append_testing_payloads('localDB-FEMaps/localDB.txt')  # SCROD mapping from local database

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
# converter.param('lookBackWindows', 28)  # for laser or pulsar data
main.add_module(converter)

# DQM setup
histo = b2.register_module("HistoManager")
histo.param('histoFileName', 'topDQM.root')
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
