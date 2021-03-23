#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                     Example of ECL trigger
#
#    usage : %> basf2 TrgEclRawdataAnalysis.py [input sroot file name]
#
# -----------------------------------------------------------------------------------
import basf2 as b2

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 2:
    sys.exit("ReadEclTrgUnpacker.py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 2:
    f_in_root = argvs[1]

b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
input = b2.register_module('SeqRootInput')
# unpacker
unpacker = b2.register_module('TRGECLUnpacker')
# output
output = b2.register_module('RootOutput')
# unpacker
trgeclrawana = b2.register_module('TRGECLRawdataAnalysis')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)
main.add_module(trgeclrawana)

input.param("inputFileName", f_in_root)
output.param("outputFileName", "TRGECLRawdataAnalysis_Cluster.root")

# main.add_module(output);
main.add_module(output, branchNames=["TRGECLUnpackerStores", "TRGECLClusters", "TRGECLTimings"])

# Process all events
b2.process(main)
#
#
print(b2.statistics)
#
