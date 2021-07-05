#!/usr/bin/env python
#
# -----------------------------------------------------------------------------------
#
#                        TOP L1 trigger Unpacker module
#
#    usage : %> basf2 trgtopUnpacker.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2

import sys

argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

if argc != 2:
    sys.exit("trgtopUnpacker.py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 2:
    f_in_root = argvs[1]

# set_log_level(LogLevel.ERROR)
b2.set_log_level(b2.LogLevel.INFO)

b2.use_central_database("online")

# input
if f_in_root[-6:] == ".sroot":
    rootfiletype = "sroot"
    input = b2.register_module('SeqRootInput')
if f_in_root[-5:] == ".root":
    rootfiletype = "root"
    input = b2.register_module('RootInput')

input.param('inputFileName', f_in_root)

# unpacker
unpacker = b2.register_module('TRGTOPUnpacker')
# No unpacking. Just print info of trigger readout board
# included in the data.
# trgReadoutBoardSearch = False
# unpacker.param('trgReadoutBoardSearch', trgReadoutBoardSearch)

# output
output = b2.register_module('RootOutput')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)

input.param("inputFileName", f_in_root)
output.param("outputFileName", "trgtop_test.root")

main.add_module(output, branchNames=["TRGTOPUnpackerStores"])

# if trgReadoutBoardSearch:
#    process(main, max_event=100)
# el
b2.process(main)
