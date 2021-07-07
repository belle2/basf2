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
# a sample of 10 BBbar events. The PXD background generator
# module is used to generate background samples for
# background overlay on the fly.
#
# Usage: basf2 background_generator.py
#
# Input: None
# Output: output.root, mdst.root
#
# Based on `reconstruction/examples/example.py`
#############################################################

import basf2 as b2
from background import get_background_files
from pxd.background_generator import inject_simulation, PXDBackgroundGenerator
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

# list of background overlay files that are available on the system
files = get_background_files()

# instantiate the PXD background generator module
generator_module = PXDBackgroundGenerator(model='resnet')

# create a drop-in simulation function that incorporates the module
add_simulation = inject_simulation(generator_module)

# detector and L1 trigger simulation with background overlay
add_simulation(main, bkgfiles=files)

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
