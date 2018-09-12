#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      GDL trigger DST Module
#
#    usage : %> basf2 trggdlDST.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

from basf2 import *
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("trggdlDST.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


set_log_level(LogLevel.INFO)

main = create_path()

# input
input = register_module('SeqRootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)

# Unpacker
trggdlUnpacker = register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)

# Fill DST
trggdldst = register_module('TRGGDLDST')
main.add_module(trggdldst)

output = register_module('RootOutput')
output.param("outputFileName", "gdldst.root")
main.add_module(output, branchNames=["TRGGDLDST"])

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
