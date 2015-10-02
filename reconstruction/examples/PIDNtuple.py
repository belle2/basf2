#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys

# ---------------------------------------------------------------------------
# write PID likelihoods + track + MCtruth to flat ntuple
# can be run interactively (takes less than 2 seconds per run with 2500 events)
# example: basf2 PIDNtuple.py -i /hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/
# BGx0/mc35_ccbar_BGx0_s00/ccbar_e0001r02\*.root pid_charm02.root
# --> will save data from runs 200-299 into pid_charm02.root
# ---------------------------------------------------------------------------

argvs = sys.argv
argc = len(argvs)

if argc != 2:
    print('***Error: output file not given -> specify it as an argument')
    sys.exit()

outFile = argvs[1]
print('Output file: ' + outFile)

# set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Input
roinput = register_module('RootInput')
main.add_module(roinput)

pidNtuple = register_module('PIDNtuple')
pidNtuple.param('outputFileName', outFile)
main.add_module(pidNtuple)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
