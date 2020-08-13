#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        CDCTSF trigger Unpacker module
#
#    usage : %> basf2 TrgCdcTsfUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
from basf2 import *

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 3:
    sys.exit("TrgCdcTsfUnpacker.py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]

set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
input = register_module('RootInput')
# unpacker
unpacker = register_module('TRGCDCTSFUnpacker')
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
main.add_module(output, branchNames=["TRGCDCTSFUnpackerStores"])

# Process all events
process(main)
