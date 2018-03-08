#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD DQM plots from root formatted raw data
# using hot pixel masking.
#
# Execute as: basf2 pxd_dqm_gcr.py -i '/home/benjamin/GCR17/root/e0002r00451/*.root'
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de


import os
import sys
import argparse
import glob
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)

# ======================================
# Now lets create the necessary modules
#
# Create Event information

gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
geometry = register_module('Geometry')
geometry.param('components', ['PXD'])

# ============================================================================
# Unpack PXD data

main = create_path()
main.add_module("RootInput")
main.add_module("HistoManager")
main.add_module(gearbox)
main.add_module(geometry)
main.add_module("ActivatePXDPixelMasker")
main.add_module("PXDUnpacker")
main.add_module("PXDRawHitSorter")
main.add_module("PXDClusterizer")
main.add_module("PXDDAQDQM")
main.add_module("PXDRawDQM")
main.add_module("PXDROIDQM")
main.add_module("PXDDQMClusters")
main.add_module("Progress")

# Process events
process(main)

# Print call statistics
print(statistics)
