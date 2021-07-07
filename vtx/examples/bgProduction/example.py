#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
# Input: Path to background file
# Output: output.root, mdst.root
#############################################################

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction, add_cdst_output, add_mc_reconstruction
from mdst import add_mdst_output

import basf2 as b2


# Need to use default global tag prepended with upgrade GT
from vtx import get_upgrade_globaltag, get_upgrade_background_files
b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())


# create path
main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# print event numbers
main.add_module('EventInfoPrinter')

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main, bkgfiles=get_upgrade_background_files(), useVTX=True)
# or add_simulation(main, components) to simulate a selection of detectors

# trigger simulation
add_tsim(main)

# reconstruction
add_mc_reconstruction(main, pruneTracks=False, useVTX=True)

# add_reconstruction(main, useVTX=True)
# or add_reconstruction(main, components, useVTX=True) to run the reconstruction of a selection of detectors

# full output
main.add_module('RootOutput', outputFileName='output.root')

# mdst output
add_mdst_output(main)

# cdst output (for calibration)
# add_cdst_output(main)

# process events and print call statistics
process(main)
print(statistics)
