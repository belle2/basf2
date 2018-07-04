#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger DQM Module
#
#    usage : %> basf2 trggdlDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

from basf2 import *
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("trggdlDQM.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


set_log_level(LogLevel.INFO)

main = create_path()

# input
input = register_module('SeqRootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)

histo = register_module('HistoManager')
histo.param("histoFileName", "./trggdlDQM.root")

# Unpacker
trggdlUnpacker = register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)
main.add_module(histo)

# DQM
trggdldqm = register_module('TRGGDLDQM')
main.add_module(trggdldqm)

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
