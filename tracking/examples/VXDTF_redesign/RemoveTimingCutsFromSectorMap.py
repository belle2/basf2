#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to remove the timing cuts from a given SectorMap and stores the
# new SectorMap to a file
# Two command line arguments are assumed:
#     - the first is the name (including path) of the SectorMap to be altered
#     - the second argument is optional, if specified the setup with corresponding name will be read from the file
# NOTE: This script should only be used by experts!
#############################################################

from basf2 import *
import sys

# file with the sector map
if(len(sys.argv) <= 1):
    print("Error: you have to specify the file the SectorMap is stored in!")
    exit(1)
sectorMapFile = sys.argv[1]

# this should currently be the only setup used in reconstruction
setupToRead = "SVDOnlyDefault"
# the second parameter is interpreted as setup to read
if(len(sys.argv) > 2):
    setupToRead = sys.argv[2]

# read the sectormap from file
SMBSM1 = register_module("SectorMapBootstrap")
SMBSM1.param("ReadSecMapFromDB", False)
SMBSM1.param("ReadSectorMap", True)
SMBSM1.param("SectorMapsInputFile", sectorMapFile)
SMBSM1.param("SetupToRead", setupToRead)

# assumes it is a root file so replace the last 5 letters
outputMapFile = sectorMapFile[:-5] + '_timingRemoved.root'
# the following setting will make the module write the altered sectormap to a root file
SMBSM1.param("SectorMapsOutputFile", outputMapFile)
SMBSM1.param("WriteSectorMap", True)

##################################################################
# now the actual alteration of the cut values
###############################################################

# NOTE: the indizes in the example below may have changed if the code changed! So you have to cross check!

# three hit filter
# (#19 <= DistanceInTime <= #20)
SMBSM1.param('threeHitFilterAdjustFunctions', [(19, "-TMath::Infinity()"), (20, "TMath::Infinity()")])
# two hit filters:
# (#12 <= DistanceInTimeUside <= #13)
# (#10 <= DistanceInTimeVside <= #11)
SMBSM1.param('twoHitFilterAdjustFunctions', [(12, "-TMath::Infinity()"), (13, "TMath::Infinity()"),
                                             (10, "-TMath::Infinity()"), (11, "TMath::Infinity()")])

# this will, in addition to other debbugging output, print the original filter ("BEFORE")
# and the altered filter ("AFTER") to the screen.
# NOTE: there are order of 10th of thousends of filters both for 2-hits and 3-hits. So expect lots of output.
# SMBSM1.logging.log_level = LogLevel.DEBUG


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
