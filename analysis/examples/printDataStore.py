#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# ----------------------------------------------------------------------------------
# This script loads specified ROOT file and prints the content of the DataStore
# for each event. To be used for debugging.
#
# Execute script with:
#
#
# basf2 printDataStore.py -i [input_ROOT_file]
#
# ----------------------------------------------------------------------------------

analysis_main.add_module('RootInput')

printDataStore()

# Process the events
process(analysis_main)
