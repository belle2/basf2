#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run:
#   PXD selection on BelleII or Phase2 geometry
# Contributors: Peter Kodys                                              *
#############################################################

from basf2 import *
import glob
import rawdata

# output filename, can be overriden with -o
output_filename = "RootOutput_PXDOnly.root"

# create path
main = create_path()

main.add_module('Progress')

# REAL DATA:
branches = ['EventMetaData', 'RawFTSWs', 'RawPXDs']
main.add_module('RootInput', branchNames=branches)

# main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')
# main.add_module('Geometry', components=['PXD'])

# Finally add output, if you need
main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
process(main)
print(statistics)
