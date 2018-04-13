#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger DQM Module
#
#    usage : %> basf2 TrgGdlDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

from basf2 import *
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


set_log_level(LogLevel.INFO)

main = create_path()

# input
input = register_module('SeqRootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)

# output
output = register_module('RootOutput')

# Unpacker
trggdlUnpacker = register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)

#
trggdlsummary = register_module('TRGGDLSummary')
main.add_module(trggdlsummary)

progress = register_module('Progress')
main.add_module(progress)

main.add_module(output, branchNames=["TRGSummary"])

process(main)

print(statistics)
