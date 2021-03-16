#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        ECL trigger Unpacker module
#
#    usage : %> basf2 TrgEclUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
import basf2 as b2

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]

if f_in_root[-6:] == ".sroot":
    input = b2.register_module('SeqRootInput')
if f_in_root[-5:] == ".root":
    input = b2.register_module('RootInput')

b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
# unpacker
unpacker = b2.register_module('TRGECLUnpacker')
# output
output = b2.register_module('RootOutput')

# Create main path
main = b2.create_path()

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
b2.process(main)
