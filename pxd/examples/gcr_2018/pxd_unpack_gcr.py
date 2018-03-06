#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import glob
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)


# ============================================================================
# Now lets create the necessary modules to perform a simulation
#
# Create Event information

rootinput = register_module('RootInput')
progress = register_module('Progress')
pxdunpacker = register_module('PXDUnpacker')
output = register_module('RootOutput')

# Only input RawPXD
rootinput.param('branchNames', ['EventMetaData', 'RawPXDs', 'ROIs'])


# ============================================================================
# Unpack PXD data

main = create_path()
main.add_module(rootinput)
main.add_module(pxdunpacker)
main.add_module(progress)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
