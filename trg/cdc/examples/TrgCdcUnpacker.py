#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        CDCTSF trigger Unpacker module
#
#    usage : %> basf2 TrgCdcTsfUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
import basf2 as b2

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 3:
    sys.exit("TrgCdcTsfUnpacker.py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]

b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
input = b2.register_module('RootInput')
# unpacker
unpacker = b2.register_module('TRGCDCTSFUnpacker')
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
main.add_module(output, branchNames=["TRGCDCTSFUnpackerStores"])

# Process all events
b2.process(main)
