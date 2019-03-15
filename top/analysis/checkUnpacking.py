#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Checks the unpacking of raw data given in Interim FE format
# Usage: basf2 checkUnpacking.py -i <file_name.sroot> [<debug_level>]
#   debug_level 100: to print additional information on errors
#   debug_level 200: to print buffer sizes and additional information on errors
# ---------------------------------------------------------------------------------------

from basf2 import *
import sys

debuglevel = 0
argvs = sys.argv
if len(argvs) > 1:
    debuglevel = int(argvs[1])

# Define a global tag (note: the one given bellow will become out-dated!)
use_central_database('data_reprocessing_proc8')

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
converter = register_module('Convert2RawDet')
main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = register_module('TOPUnpacker')
unpack.logging.log_level = LogLevel.DEBUG
unpack.logging.debug_level = debuglevel
main.add_module(unpack)

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print statistics
print(statistics)
