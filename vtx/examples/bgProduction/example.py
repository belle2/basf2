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

from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction
from mdst import add_mdst_output

import basf2 as b2


from vtx import get_upgrade_background_files
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

# detector simulation
add_simulation(main, components=get_components(exp_number), bkgfiles=get_upgrade_background_files(), useVTX=True)

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main, components=get_components(exp_number), pruneTracks=False, useVTX=True)

# full output
main.add_module('RootOutput', outputFileName='output.root')

# mdst output
add_mdst_output(main)

# cdst output (for calibration)
# add_cdst_output(main)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
