#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        ECL trigger Unpacker module
#
#    usage : %> basf2 TrgEclUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
from basf2 import *

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]

if f_in_root[-6:] == ".sroot":
    input = register_module('SeqRootInput')
if f_in_root[-5:] == ".root":
    input = register_module('RootInput')

set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
# unpacker
unpacker = register_module('TRGECLUnpacker')
# output
output = register_module('RootOutput')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)

input.param("inputFileName", f_in_root)
output.param("outputFileName", f_out_root)

# main.add_module(output);
main.add_module(
    output,
    branchNames=[
        "TRGECLUnpackerStores",
        "TRGECLUnpackerEvtStores",
        "TRGECLUnpackerSumStores"])

# Process all events
process(main)
