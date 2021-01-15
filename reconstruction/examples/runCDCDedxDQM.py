#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# Test script to produce DQM plots for CDC dE/dx
#
# Usage: basf2 runCDCDedxDQM.py
#
# Input: B2Electrons.root
# Output: CDCDedxDQM.root
#
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import basf2 as b2

# main path
main = b2.create_path()

# read in a sample
input_file = 'B2Electrons.root'
main.add_module("RootInput", inputFileName=input_file)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="CDCDedxDQM.root")

# run the DQM module
main.add_module("CDCDedxDQM")

# Process events
b2.process(main)
