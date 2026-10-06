#!/usr/bin/env python3

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
######################################################

import argparse
import basf2 as b2

parser = argparse.ArgumentParser(description="Creates Belle II detector geometry and checks for overlaps.")
parser.add_argument("points", nargs="?", type=int, default=100, help="Number of test points per volume (default: 100)")
parser.add_argument(
    "--exp",
    type=int,
    default=None,
    help="Experiment number for EventInfoSetter (e.g. 2003, 2004). If not specified, builds default geometry.")
parser.add_argument("--run", type=int, default=0, help="Run number for EventInfoSetter (default: 0)")
parser.add_argument(
    "--use-db",
    dest="use_db",
    action=argparse.BooleanOptionalAction,
    default=True,
    help="Use conditions database for geometry (default: True). Set --no-use-db to build from local XML files.")
parser.add_argument("--output", default="Overlaps.root", help="Output ROOT file for overlaps (default: Overlaps.root)")
args = parser.parse_args()

# Create main path
main = b2.Path()
# Add modules to main path
if args.use_db:
    if args.exp is not None:
        main.add_module("EventInfoSetter", expList=[args.exp], runList=[args.run], evtNumList=[1])
    else:
        main.add_module("EventInfoSetter")
    # Geometry parameter loader
    main.add_module("Gearbox")
    # Geometry builder from conditions database
    main.add_module("Geometry", useDB=True)
else:
    if args.exp is not None and args.exp != 0:
        b2.B2FATAL("Building geometry without database (--no-use-db) is only permitted for exp=0, run=0.")
    main.add_module("EventInfoSetter")
    # Geometry parameter loader from local XML files
    gearbox = b2.register_module('Gearbox')
    gearbox.param('fileName', 'geometry/Belle2.xml')
    main.add_module(gearbox)
    # Geometry builder using local XML files
    main.add_module("Geometry", excludedComponents=['PXD', 'HeavyMetalShield', 'SVD'], useDB=False)

# Overlap checker
main.add_module("OverlapChecker", points=args.points)
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName=args.output)
# Process one event
b2.process(main)
