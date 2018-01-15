#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to alter the
# cuts of the filters used in VXDTF2.
# NOTE: This feature is not meant to be for default reconstruction!
#   Only experts should use it!
#############################################################

from basf2 import *


# The SectorMapBootstrap module will take care for the alterations.
# So create one for this example which will read the current sectormap from the
# database, alters some parameters, and writes the altered sectormap to a root
# file. The sectormap in the root file then can be compared to the original sectormap (also
# in a root file). If you want to read a sectormap from a root file please change
# the parameters accordingly.
SMBSM1 = register_module("SectorMapBootstrap")
SMBSM1.param("ReadSecMapFromDB", True)
SMBSM1.param("ReadSectorMap", False)
SMBSM1.param("SectorMapsInputFile", "SVDSectorMap_v000.root")
SMBSM1.param("SetupToRead", "SVDOnlyDefault")

# the following setting will make the module write the altered sectormap to a root file
SMBSM1.param("SectorMapsOutputFile", "testMap.root")
SMBSM1.param("WriteSectorMap", True)

##################################################################
# now the actual alteration of the cut values
###############################################################

# To change cutvalues of filters you have to provide a list (int, string) to the SectorMapBootstrap module which
# tells it which variables in the filters to change and how. The int is the index of the cut value you want to change
# and the string is a descriptor for a TF1 root function.
# The index you can find out by looking into the documentation for the SectorMapBootstrap module ( basf2 -m SectorMapBootstrap ).
# For the descriptor of the function one can use any regex which works with a TF1. The assumption
# is that "x" is the current cut value and "[0]" (0th parameter) is the FullSecID the filter is attached
# to. Examples for functions are (they dont make sense): "sin(x)", "12", "x + [0]", ...

# NOTE: the indizes in the example below may have changed if the code changed! So you have to cross check!

# Some example alterations for the two hit filters:
#    - set min max of DistanceInTimeUside to +/- inf (index 12 min, index 13 max)
#    - Distance1DZ; shift min by -4 (index 4), shift max by +4 (index 5)
SMBSM1.param("twoHitFilterAdjustFunctions", [(12, "-TMath::Infinity()"), (13, "TMath::Infinity()"), (4, "x-4"), (5, "x+4")])

# Some example alterations of the three hit filters:
#     - set CircleRadius low bound to 0 (index 0), shift upper bound by +3 (index 1)
#     - set CosAngleXY low bound to the FullSecID of the static sector it is attached to (index 15)
SMBSM1.param("threeHitFilterAdjustFunctions", [(0, "0"), (1, "x+3"), (15, "[0]")])

# this will, in addition to other debbugging output, print the original filter ("BEFORE")
# and the altered filter ("AFTER") to the screen.
# NOTE: there are order of 10th of thousends of filters both for 2-hits and 3-hits. So expect lots of output.
SMBSM1.logging.log_level = LogLevel.DEBUG


# needed else no sectormap from DB can be loaded
eventinfosetter = register_module('EventInfoSetter')

# create path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(SMBSM1)
print_path(main)
# run path
process(main)
print(statistics)
