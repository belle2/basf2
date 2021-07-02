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
# Simple steering file to demonstrate how to run:
#   PXD selection on BelleII or Phase2 geometry
# Contributors: Peter Kodys                                              *
#############################################################

import basf2 as b2

# output filename, can be overriden with -o
output_filename = "RootOutput_PXDOnly.root"

# create path
main = b2.create_path()

main.add_module('Progress')

# REAL DATA:
branches = ['EventMetaData', 'RawFTSWs', 'RawPXDs']
main.add_module('RootInput', branchNames=branches)

# main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')
# main.add_module('Geometry', components=['PXD'])

# Finally add output, if you need
main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
