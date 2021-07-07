#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file creates the Belle II detector
# geometry and checks for overlaps
#
# Usage: basf2 check_vtx_geometry.py -- --fast --vtx=VTX-CMOS-5layer-Discs
#
######################################################

from basf2 import Path, process
from sys import argv

import argparse
ap = argparse.ArgumentParser()
ap.add_argument("--vtx", default="VTX-CMOS-7layer", help="VTX geometry variant from xml")
ap.add_argument('--fast', default=False, action='store_true', help="Use this option to restrict overlap checking to VXD volume")
ap.add_argument("--points", type=int, default=10000, help="Number of test points")
args = vars(ap.parse_args())

print("INFO: Checking overlaps for " + args['vtx'])

if args['fast']:
    print("INFO: Doing fast overlap check")
    # Remove all parts to speedup check. Only additional VTX will be tested. Pretty fast.
    excluded_parts = ['COIL', 'STR', 'ServiceGapsMaterial', 'BeamPipe', 'Cryostat', 'FarBeamLine', 'HeavyMetalShield', 'VXDService',
                      'PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM']
else:
    print("INFO: Doing full overlap check")
    # Only remove PXD+SVD and test full Belle 2 w/ VTX. Can take some time.
    excluded_parts = ['PXD', 'SVD']


# Create main path
main = Path()
# Add modules to main path
main.add_module("EventInfoSetter")
# Geometry parameter loader
main.add_module("Gearbox")
# Geometry builder
main.add_module('Geometry', excludedComponents=excluded_parts,
                additionalComponents=[args['vtx']],
                useDB=False)
# Overlap checker
main.add_module("OverlapChecker", points=int(args['points']))
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName="Overlaps_{}.root".format(args['vtx']))
# Process one event
process(main)
