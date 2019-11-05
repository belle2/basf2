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
# Input: cdst_hadronEvents.root
# Output: EventT0DQM.root
#
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

import os
from basf2 import *

# main path
main = create_path()

# read in a sample
input_file = 'cdst_hadronEvents.root'
main.add_module("RootInput", inputFileName=input_file)

# register the HistoManager and specify output file
main.add_module("HistoManager", histoFileName="EventT0DQM.root")

# run the DQM module
main.add_module("EventT0DQM")

# Process events
process(main)
