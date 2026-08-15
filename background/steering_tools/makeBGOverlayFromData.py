##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys

import basf2
from background import add_bgo_modules

# --------------------------------------------------------------------------------------
# Make data sample for BG overlay from experimental raw data
#
# Usage: basf2 makeBGOverlayFromData.py -i <inputFileName> -o <outputFileName> globalTag
#
# --------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 2:
    print("usage: basf2", argvs[0], "globalTag -i <inputFileName> -o <outputFileName>")
    sys.exit()

# Define global tag
globalTag = argvs[1]
basf2.conditions.prepend_globaltag(globalTag)

# Create paths
main = basf2.Path()

# Input
main.add_module('RootInput')

# Add all the modules
add_bgo_modules(main)

# Process events
basf2.process(main, calculateStatistics=True)
