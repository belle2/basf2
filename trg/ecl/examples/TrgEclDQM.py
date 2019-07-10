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
if argc != 2:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


set_log_level(LogLevel.INFO)

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)

histo = register_module('HistoManager')
histo.param("histoFileName", "./histo_output.root")

# Unpacker
trgeclUnpacker = register_module("TRGECLUnpacker")
main.add_module(trgeclUnpacker)
main.add_module(histo)

# DQM
trgecldqm = register_module('TRGECLDQM')
main.add_module(trgecldqm)

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
