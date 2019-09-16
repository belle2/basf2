#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of ECL trigger DQM Module
#
#    usage : %> basf2 TrgEclDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

from basf2 import *
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 3:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]


set_log_level(LogLevel.INFO)

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)


# Unpacker
trgeclUnpacker = register_module("TRGECLUnpacker")
trggdlUnpacker = register_module("TRGGDLUnpacker")
trggdlsummary = register_module("TRGGDLSummary")

main.add_module(trgeclUnpacker)
main.add_module(trggdlUnpacker)
main.add_module(trggdlsummary)


# QAM
trgeclqam = register_module('TRGECLQAM')
trgeclqam.param('outputFileName', f_out_root)
main.add_module(trgeclqam)

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
