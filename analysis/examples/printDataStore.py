#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

import sys

# ----------------------------------------------------------------------------------
# This script loads specified ROOT file and prints the content of the DataStore
# for each event. To be used for debugging.
# ----------------------------------------------------------------------------------

if len(sys.argv) != 2:
    sys.exit('Must provide one input parameter:[input_root_file_name]')

inputMdstFileName = sys.argv[1]

inputMdst(inputMdstFileName)

printDataStore()

# Process the events
process(analysis_main)
