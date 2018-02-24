#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import glob
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)

parser = argparse.ArgumentParser(description="PXD DQM from one run with multuiple subruns from gcr 2018")
parser.add_argument('--histofile', default="PXD_DQM.root", type=str, help='Name of histofile')
args = parser.parse_args()

# ============================================================================
# Now lets create the necessary modules to perform a simulation
#
# Create Event information

rootinput = register_module('RootInput')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
histoman = register_module('HistoManager')
progress = register_module('Progress')
pxdhitsorter = register_module('PXDRawHitSorter')
pxdclusterizer = register_module('PXDClusterizer')
output = register_module('RootOutput')

# Select Beast2_phase2 geometry
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')

# Select to build only PXD s
geometry.param('components', ['PXD'])

# Select name of histofile
histoman.param('histoFileName', args.histofile)

# ============================================================================
# Unpack PXD data

main = create_path()
main.add_module(rootinput)
main.add_module(histoman)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module("ActivatePXDPixelMasker")
main.add_module(pxdhitsorter)
main.add_module(pxdclusterizer)
main.add_module(register_module('PXDDAQDQM'))
main.add_module(register_module('PXDRawDQM'))
main.add_module(register_module('PXDROIDQM'))
main.add_module(register_module('PXDDQMClusters'))
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
