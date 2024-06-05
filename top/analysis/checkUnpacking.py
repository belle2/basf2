#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Checks the unpacking of raw data
# Usage: basf2 checkUnpacking.py -i <file_name.root> [<debug_level>]
#   debug_level 21: to print additional information on errors
#   debug_level 22: to print buffer sizes and additional information on errors
# ---------------------------------------------------------------------------------------

import basf2 as b2
import sys

debuglevel = 0
argvs = sys.argv
if len(argvs) > 1:
    debuglevel = int(argvs[1])

# Define a global tag
b2.conditions.override_globaltags()
b2.conditions.append_globaltag('online')

# Create path
main = b2.create_path()

# input
# roinput = b2.register_module('SeqRootInput')  # sroot files
roinput = b2.register_module('RootInput')  # root files
main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects (needed if PocketDAQ)
converter = b2.register_module('Convert2RawDet')
main.add_module(converter)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Unpacking (format auto detection works now)
unpack = b2.register_module('TOPUnpacker')
unpack.logging.log_level = b2.LogLevel.DEBUG
unpack.logging.debug_level = debuglevel
main.add_module(unpack)

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
