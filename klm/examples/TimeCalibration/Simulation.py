#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Reconstruction of generated ee->mumu(ISR) events.

import glob
import os
import sys
import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_cdst_output

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

# Create the main path and add the modules
main = basf2.create_path()
main.add_module(input)

background_files = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    background_directory = os.environ['BELLE2_BACKGROUND_DIR']
    background_files = glob.glob(background_directory + '/*.root')
else:
    basf2.B2WARNING('The variable BELLE2_BACKGROUND_DIR is not set. '
                    'Beam background is not used in the simulation')
add_simulation(main, bkgfiles=background_files)

add_reconstruction(main)
add_cdst_output(main, filename=sys.argv[2], rawFormat=False)
main.add_module('Progress')

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
