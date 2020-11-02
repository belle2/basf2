#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce DQM plots for the event t0
#
# Author:  Ewan Hill  (ehill@mail.ubc.ca)
#
# Usage: basf2 runEventT0DQM.py
#
# Default input: cdst_hadronEvents.root
# Output:        EventT0DQM.root
#
# Optional input usage:
#    basf2 runEventT0DQM.py -i "<path/to/file.root>"
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import basf2 as b2

# main path
main = b2.create_path()

# read in a sample
input_file = 'cdst_hadronEvents.root'
main.add_module("RootInput", inputFileName=input_file)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="EventT0DQM.root")

# run the DQM module
eventT0DQMmodule = main.add_module("EventT0DQM")
eventT0DQMmodule.set_log_level(b2.LogLevel.INFO)  # LogLevel.DEBUG / LogLevel.INFO
eventT0DQMmodule.set_debug_level(21)

# == Show progress
main.add_module('Progress')

# Process events
b2.process(main)

print(b2.statistics)
