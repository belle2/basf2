#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
#
# Test script to produce DQM plots for the event t0
#
# Usage: basf2 runEventT0DQM.py
#
# Default input: cdst_hadronEvents.root
# Output:        EventT0DQM.root
#
# Optional input usage:
#    basf2 runEventT0DQM.py -i "<path/to/file.root>"
#############################################################

import basf2 as b2
import reconstruction as reco
import rawdata as raw
b2.conditions.prepend_globaltag("patch_main_release-09")

# main path
main = b2.create_path()

# read in a sample
main.add_module("RootInput")

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="BeforeHLTFilterDQM.root")

raw.add_unpackers(main)
reco.add_reconstruction(main)

# run the DQM module
dqmModule = main.add_module("BeforeHLTFilterDQM")
dqmModule.set_log_level(b2.LogLevel.INFO)  # LogLevel.DEBUG / LogLevel.INFO
dqmModule.set_debug_level(21)

# == Show progress
main.add_module('Progress')

# Process events
b2.process(main)

print(b2.statistics)
