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
from L1trigger import add_tsim
from reconstruction import add_reconstruction, add_cdst_output
from mdst import add_mdst_output

# Need to use default global tag prepended with upgrade GT
from vtx import get_upgrade_globaltag
b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())


# create path
main = b2.create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# print event numbers
main.add_module('EventInfoPrinter')

main.add_module("EvtGenInput")

# detector simulation
add_simulation(main, useVTX=True)


# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main, useVTX=True)

# full output
main.add_module('RootOutput', outputFileName='vtx_output.root')

# mdst output
add_mdst_output(main, filename='vtx_mdst.root')

# cdst output (for calibration)
# add_cdst_output(main)

# process events and print call statistics
b2.process(main)
print(statistics)
