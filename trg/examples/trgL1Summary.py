#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        GDL trigger Unpacker module
#
#    usage : %> basf2 TrgGdlUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
from basf2 import *

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 2:
    sys.exit("TrgGdlUnpacker.py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 2:
    f_in_root = argvs[1]

set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
input = register_module('SeqRootInput')
# unpacker
unpacker = register_module('TRGGDLUnpacker')

summary = register_module('TRGGDLSUMMARY')
# output
output = register_module('RootOutput')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)
main.add_module(summary)

input.param("inputFileName", f_in_root)
output.param("outputFileName", "trgL1Summary.root")

main.add_module(output, branchNames=["TRGGDLUnpackerStores"])

# Process all events
process(main)
