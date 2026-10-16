#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events.
#
# Set environment variables BELLE2_VTX_BACKGROUND_DIR and
# BELLE2_VTX_UPGRADE_GT according to confluence page
# https://confluence.desy.de/display/BI/Full+simulation+effort
# recommondation.
#
# Usage: basf2 vtx_example.py
#
# Input: None
# Output: vtx_output.root, vtx_mdst.root
#
# Example steering file for VTX upgrade - 2020 Belle II Collaboration
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from mdst import add_mdst_output
from geometry import get_components

# create path
main = b2.create_path()

# specify number of events to be generated
exp_number = 2003
main.add_module('EventInfoSetter', evtNumList=[10], expList=[exp_number])

# print event numbers
main.add_module('EventInfoPrinter')

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main, components=get_components(exp_number), useVTX=True)

# reconstruction
add_reconstruction(main, components=get_components(exp_number), useVTX=True)

# full output
main.add_module('RootOutput', outputFileName='vtx_output.root')

# mdst output
add_mdst_output(main, filename='vtx_mdst.root')

# cdst output (for calibration)
# add_cdst_output(main)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
