#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events.
#
# Usage: basf2 example.py
#
# Input: None
# Output: output.root, mdst.root
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from mdst import add_mdst_output

# create path
main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# print event numbers
main.add_module('EventInfoPrinter')

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main)
# or add_simulation(main, components) to simulate a selection of detectors

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)
# or add_reconstruction(main, components) to run the reconstruction of a selection of detectors

# full output
main.add_module('RootOutput', outputFileName='output.root')

# mdst output
add_mdst_output(main)

# process events and print call statistics
process(main)
print(statistics)
