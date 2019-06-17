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
    sys.exit("...py> # of arg is strange.\n 1.rootname\n Exit.")

if argc == 2:
    f_in_root = argvs[1]

# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

use_central_database("TRGGDL_201811")

# input
if f_in_root[-6:] == ".sroot":
    rootfiletype = "sroot"
    input = register_module('SeqRootInput')
if f_in_root[-5:] == ".root":
    rootfiletype = "root"
    input = register_module('RootInput')

input.param('inputFileName', f_in_root)

# unpacker
unpacker = register_module('TRGRAWDATA')
unpacker.param('print_dbmap', True)
unpacker.param('cc_check', True)
unpacker.param('print_cc', True)
unpacker.param('on_2d0', True)
unpacker.param('on_2d1', True)
unpacker.param('on_2d2', True)
unpacker.param('on_2d3', True)
# unpacker.param('on_2d1', True)
# unpacker.param('on_2d2', True)
# unpacker.param('on_2d3', True)
unpacker.param('on_gdl', True)
unpacker.param('on_sl5', True)
unpacker.param('on_sl6', True)
unpacker.param('on_sl8', False)
unpacker.param('on_etf', True)
unpacker.param('on_grl', False)
unpacker.param('on_nn0', True)
unpacker.param('on_nn1', True)
unpacker.param('on_nn2', True)
unpacker.param('on_nn3', True)
unpacker.param('on_3d0', True)
unpacker.param('on_3d1', True)
unpacker.param('on_3d2', True)
unpacker.param('on_3d3', True)
# unpacker.param('mydebug', True)
# No unpacking. Just print info of trigger readout board
# included in the data.
trgReadoutBoardSearch = False
# unpacker.param('trgReadoutBoardSearch', trgReadoutBoardSearch)

# output
# output = register_module('RootOutput')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(unpacker)

input.param("inputFileName", f_in_root)

# output.param("outputFileName", "gdltrg_test.root")
# main.add_module(output, branchNames=["TRGGDLUnpackerStores"])

if trgReadoutBoardSearch:
    process(main, max_event=100000)
else:
    process(main)
