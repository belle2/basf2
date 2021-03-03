#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events. It uses the PXD background
# generator module.
#
# Usage: basf2 background_generator.py
#
# Input: None
# Output: output.root, mdst.root
#
# Example steering file - 2021 Belle II Collaboration
# Based on reconstruction/examples/example.py
#############################################################

import basf2 as b2
from background import get_background_files
from pxd.background_generator import Specs
from simulation import add_simulation
from L1trigger import add_tsim
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

# detector simulation
files = get_background_files()
specs = Specs(model='convnet', seed=0)
add_simulation(main, bkgfiles=files, pxd_background_generator=specs)

# trigger simulation
add_tsim(main)

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
