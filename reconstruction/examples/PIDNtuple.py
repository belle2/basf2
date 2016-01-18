#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file writes PID likelihoods, tracks, and MC
# truth information to a flat ntuple. It can be run
# interactively (takes less than 2 seconds per run with
# 2500 events each).
#
# Usage:
# a) basf2 PIDNtuple.py -i output.root pid_example.root
# --> saves PID information from ROOT file generated using
#     example.py (must run example.py first) and saves to
#     pid_example.root
#
# b) basf2 PIDNtuple.py -i /hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/
#     BGx0/mc35_ccbar_BGx0_s00/ccbar_e0001r02\*.root pid_charm02.root
# --> will save PID information from runs 200-299 into
#     pid_charm02.root (must run on kekcc)
#
# Input: mdst file specified by command line arguments
#        (example a above uses output.root from example.py)
# Output: ROOT file specified by command line arguments
#        (example b above writes to pid_example.root)
#
# Example steering file - 2011 Belle II Collaboration
#############################################################

from basf2 import *
import sys

# parse command line parameters
argvs = sys.argv
argc = len(argvs)

if argc != 2:
    print('***Error: output file not given -> specify it as an argument')
    sys.exit()

outFile = argvs[1]
print('Output file: ' + outFile)

# set_log_level(LogLevel.ERROR)

# create path
main = create_path()

# define input
roinput = register_module('RootInput')
main.add_module(roinput)

# write the results to a flat ntuple
pidNtuple = register_module('PIDNtuple')
pidNtuple.param('outputFileName', outFile)
main.add_module(pidNtuple)

# show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# process events and print call statistics
process(main)
print(statistics)
