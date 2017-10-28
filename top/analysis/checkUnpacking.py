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

# Create path
main = create_path()

# input
roinput = register_module('SeqRootInput')
# roinput = register_module('RootInput')
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
converter = register_module('Convert2RawDet')
main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# Unpacking
unpack = register_module('TOPUnpacker')
unpack.param('swapBytes', True)
unpack.param('dataFormat', 0x0301)
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
