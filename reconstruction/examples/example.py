#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events.
#
# Usage: basf2 example.py
#
# Input: None
# Output: output.root, mdst.root
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output

# create path
main = b2.create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# print event numbers
main.add_module('EventInfoPrinter')

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main)
# or add_simulation(main, components) to simulate a selection of detectors and triggr

# reconstruction
add_reconstruction(main)
# or add_reconstruction(main, components) to run the reconstruction of a selection of detectors

# full output
main.add_module('RootOutput', outputFileName='output.root')

# mdst output
add_mdst_output(main)

# cdst output (for calibration)
# add_cdst_output(main)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
