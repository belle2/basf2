#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from simulation import add_simulation

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: [mc|data] [input_Belle_MDST_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

setupB2BIIDatabase(isMC)

inputBelleMDSTFile = sys.argv[2]

main = create_path()

# Input MDST Module
input = register_module('B2BIIMdstInput')
input.param('inputFileName', inputBelleMDSTFile)
# input.logging.set_log_level(LogLevel.DEBUG)
# input.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
main.add_module(input)

# Fix MSDT Module
fix = register_module('B2BIIFixMdst')
# fix.logging.set_log_level(LogLevel.DEBUG)
# fix.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
main.add_module(fix)

emptypath = create_path()
fix.if_value('<=0', emptypath)  # discard 'bad events' marked by fixmdst

# Convert MDST Module
convert = register_module('B2BIIConvertMdst')
# convert.logging.set_log_level(LogLevel.DEBUG)
# convert.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
main.add_module(convert)

# Print out the contents of the DataStore
dump = register_module('PrintCollections')
main.add_module(dump)

# progress
progress = register_module('Progress')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)
