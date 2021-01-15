#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      GDL trigger DST Module
#
#    usage : %> basf2 trggdlDST.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

import basf2 as b2
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("trggdlDST.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


b2.set_log_level(b2.LogLevel.INFO)

main = b2.create_path()

# input
if f_in_root[-6:] == ".sroot":
    rootfiletype = "sroot"
    input = b2.register_module('SeqRootInput')
if f_in_root[-5:] == ".root":
    rootfiletype = "root"
    input = b2.register_module('RootInput')

input.param('inputFileName', f_in_root)
main.add_module(input)

# Unpacker
trggdlUnpacker = b2.register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)

# Fill DST
trggdldst = b2.register_module('TRGGDLDST')
main.add_module(trggdldst)

output = b2.register_module('RootOutput')
output.param("outputFileName", "gdldst.root")
main.add_module(output, branchNames=["TRGGDLDST"])

progress = b2.register_module('Progress')
main.add_module(progress)

b2.process(main)

print(b2.statistics)
