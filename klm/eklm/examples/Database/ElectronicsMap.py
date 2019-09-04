#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import EKLMDatabaseImporter, EKLMElectronicsMap
from basf2 import *

set_log_level(LogLevel.INFO)

# MC corresponds to the design electronics map

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

# EKLM electronics map
#
#     backward (section == 1)    forward (section == 2)
#           _1____2____               _2____1____       Sector number
#      EB0 /   ||   \\\\ EB1     EF0 /   ||   \\\\ EF1  Crate identifier
#         /3   ||3   \\\\           /1   ||1   \\\\     Copper number - EKLM_ID
#        | 0,1 ||2,3  ||||         | 0,1 ||2,3  ||||    Data concentrator number
# ____\  |____/  \____||||  ____\  |____/  \____||||  _____\
#     /  |    \  /    ||||      /  |    \  /    ||||       /  Beam (z) direction
#        | 4   || 4   ||||         | 2   ||2    ||||    Copper number - EKLM_ID
#         \2,3 || 0,1////           \2,3 ||0,1 ////     Data concentrator number
#      EB3 \___||___//// EB2     EF3 \___||___//// EF2  Crate identifier
#            4    3                    3    4           Sector number
#
# A part of the EKLM electronics has been connected in a wrong way and it was
# corrected during the experiment. Thus, the EKLM electronics map depends on
# time.


def eklm_electronics_map(stage):

    electronics_map = EKLMElectronicsMap()

    # Backward section
    electronics_map.addSectorLane(1, 1, 1, 3, 0, 1)
    electronics_map.addSectorLane(1, 2, 1, 3, 0, 2)
    electronics_map.addSectorLane(1, 3, 1, 3, 0, 3)
    electronics_map.addSectorLane(1, 4, 1, 3, 0, 4)
    electronics_map.addSectorLane(1, 5, 1, 3, 0, 5)
    electronics_map.addSectorLane(1, 6, 1, 3, 0, 6)
    electronics_map.addSectorLane(1, 7, 1, 3, 1, 1)
    electronics_map.addSectorLane(1, 8, 1, 3, 1, 2)
    electronics_map.addSectorLane(1, 9, 1, 3, 1, 3)
    # Switch lanes for real-data map.
    # The wrong connection was fixed between phase 2 and phase 3.
    if (mc or (stage >= 2)):
        electronics_map.addSectorLane(1, 10, 1, 3, 1, 4)
        electronics_map.addSectorLane(1, 11, 1, 3, 1, 5)
    else:
        electronics_map.addSectorLane(1, 10, 1, 3, 1, 5)
        electronics_map.addSectorLane(1, 11, 1, 3, 1, 4)
    electronics_map.addSectorLane(1, 12, 1, 3, 1, 6)

    if (stage == 1):
        electronics_map.addSectorLane(1, 1, 2, 3, 2, 1)
        electronics_map.addSectorLane(1, 2, 2, 3, 2, 2)
        electronics_map.addSectorLane(1, 3, 2, 3, 2, 3)
        electronics_map.addSectorLane(1, 4, 2, 3, 2, 4)
        electronics_map.addSectorLane(1, 5, 2, 3, 2, 5)
        electronics_map.addSectorLane(1, 6, 2, 3, 2, 6)
        electronics_map.addSectorLane(1, 7, 2, 3, 3, 1)
        electronics_map.addSectorLane(1, 8, 2, 3, 3, 2)
        electronics_map.addSectorLane(1, 9, 2, 3, 3, 3)
        electronics_map.addSectorLane(1, 10, 2, 3, 3, 4)
        electronics_map.addSectorLane(1, 11, 2, 3, 3, 5)
        electronics_map.addSectorLane(1, 12, 2, 3, 3, 6)
    else:
        electronics_map.addSectorLane(1, 1, 2, 3, 3, 6)
        electronics_map.addSectorLane(1, 2, 2, 3, 3, 5)
        electronics_map.addSectorLane(1, 3, 2, 3, 3, 4)
        electronics_map.addSectorLane(1, 4, 2, 3, 3, 3)
        electronics_map.addSectorLane(1, 5, 2, 3, 3, 2)
        electronics_map.addSectorLane(1, 6, 2, 3, 3, 1)
        electronics_map.addSectorLane(1, 7, 2, 3, 2, 6)
        electronics_map.addSectorLane(1, 8, 2, 3, 2, 5)
        electronics_map.addSectorLane(1, 9, 2, 3, 2, 4)
        electronics_map.addSectorLane(1, 10, 2, 3, 2, 3)
        electronics_map.addSectorLane(1, 11, 2, 3, 2, 2)
        electronics_map.addSectorLane(1, 12, 2, 3, 2, 1)

    if (stage == 1):
        electronics_map.addSectorLane(1, 1, 3, 4, 0, 1)
        electronics_map.addSectorLane(1, 2, 3, 4, 0, 2)
        electronics_map.addSectorLane(1, 3, 3, 4, 0, 3)
        electronics_map.addSectorLane(1, 4, 3, 4, 0, 4)
        electronics_map.addSectorLane(1, 5, 3, 4, 0, 5)
        electronics_map.addSectorLane(1, 6, 3, 4, 0, 6)
        electronics_map.addSectorLane(1, 7, 3, 4, 1, 1)
        electronics_map.addSectorLane(1, 8, 3, 4, 1, 2)
        electronics_map.addSectorLane(1, 9, 3, 4, 1, 3)
        electronics_map.addSectorLane(1, 10, 3, 4, 1, 4)
        electronics_map.addSectorLane(1, 11, 3, 4, 1, 5)
        electronics_map.addSectorLane(1, 12, 3, 4, 1, 6)
    else:
        electronics_map.addSectorLane(1, 1, 3, 4, 1, 6)
        electronics_map.addSectorLane(1, 2, 3, 4, 1, 5)
        electronics_map.addSectorLane(1, 3, 3, 4, 1, 4)
        electronics_map.addSectorLane(1, 4, 3, 4, 1, 3)
        electronics_map.addSectorLane(1, 5, 3, 4, 1, 2)
        electronics_map.addSectorLane(1, 6, 3, 4, 1, 1)
        electronics_map.addSectorLane(1, 7, 3, 4, 0, 6)
        electronics_map.addSectorLane(1, 8, 3, 4, 0, 5)
        electronics_map.addSectorLane(1, 9, 3, 4, 0, 4)
        electronics_map.addSectorLane(1, 10, 3, 4, 0, 3)
        electronics_map.addSectorLane(1, 11, 3, 4, 0, 2)
        electronics_map.addSectorLane(1, 12, 3, 4, 0, 1)

    electronics_map.addSectorLane(1, 1, 4, 4, 2, 1)
    electronics_map.addSectorLane(1, 2, 4, 4, 2, 2)
    electronics_map.addSectorLane(1, 3, 4, 4, 2, 3)
    electronics_map.addSectorLane(1, 4, 4, 4, 2, 4)
    electronics_map.addSectorLane(1, 5, 4, 4, 2, 5)
    electronics_map.addSectorLane(1, 6, 4, 4, 2, 6)
    electronics_map.addSectorLane(1, 7, 4, 4, 3, 1)
    electronics_map.addSectorLane(1, 8, 4, 4, 3, 2)
    electronics_map.addSectorLane(1, 9, 4, 4, 3, 3)
    electronics_map.addSectorLane(1, 10, 4, 4, 3, 4)
    electronics_map.addSectorLane(1, 11, 4, 4, 3, 5)
    electronics_map.addSectorLane(1, 12, 4, 4, 3, 6)

    # Forward section
    electronics_map.addSectorLane(2, 1, 1, 1, 2, 1)
    electronics_map.addSectorLane(2, 2, 1, 1, 2, 2)
    electronics_map.addSectorLane(2, 3, 1, 1, 2, 3)
    electronics_map.addSectorLane(2, 4, 1, 1, 2, 4)
    electronics_map.addSectorLane(2, 5, 1, 1, 2, 5)
    electronics_map.addSectorLane(2, 6, 1, 1, 2, 6)
    electronics_map.addSectorLane(2, 7, 1, 1, 2, 7)
    electronics_map.addSectorLane(2, 8, 1, 1, 3, 1)
    electronics_map.addSectorLane(2, 9, 1, 1, 3, 2)
    electronics_map.addSectorLane(2, 10, 1, 1, 3, 3)
    electronics_map.addSectorLane(2, 11, 1, 1, 3, 4)
    electronics_map.addSectorLane(2, 12, 1, 1, 3, 5)
    electronics_map.addSectorLane(2, 13, 1, 1, 3, 6)
    electronics_map.addSectorLane(2, 14, 1, 1, 3, 7)

    electronics_map.addSectorLane(2, 1, 2, 1, 0, 1)
    electronics_map.addSectorLane(2, 2, 2, 1, 0, 2)
    electronics_map.addSectorLane(2, 3, 2, 1, 0, 3)
    electronics_map.addSectorLane(2, 4, 2, 1, 0, 4)
    electronics_map.addSectorLane(2, 5, 2, 1, 0, 5)
    electronics_map.addSectorLane(2, 6, 2, 1, 0, 6)
    electronics_map.addSectorLane(2, 7, 2, 1, 0, 7)
    electronics_map.addSectorLane(2, 8, 2, 1, 1, 1)
    electronics_map.addSectorLane(2, 9, 2, 1, 1, 2)
    electronics_map.addSectorLane(2, 10, 2, 1, 1, 3)
    electronics_map.addSectorLane(2, 11, 2, 1, 1, 4)
    electronics_map.addSectorLane(2, 12, 2, 1, 1, 5)
    electronics_map.addSectorLane(2, 13, 2, 1, 1, 6)
    electronics_map.addSectorLane(2, 14, 2, 1, 1, 7)

    electronics_map.addSectorLane(2, 1, 3, 2, 2, 1)
    electronics_map.addSectorLane(2, 2, 3, 2, 2, 2)
    electronics_map.addSectorLane(2, 3, 3, 2, 2, 3)
    electronics_map.addSectorLane(2, 4, 3, 2, 2, 4)
    electronics_map.addSectorLane(2, 5, 3, 2, 2, 5)
    electronics_map.addSectorLane(2, 6, 3, 2, 2, 6)
    electronics_map.addSectorLane(2, 7, 3, 2, 2, 7)
    electronics_map.addSectorLane(2, 8, 3, 2, 3, 1)
    electronics_map.addSectorLane(2, 9, 3, 2, 3, 2)
    electronics_map.addSectorLane(2, 10, 3, 2, 3, 3)
    electronics_map.addSectorLane(2, 11, 3, 2, 3, 4)
    electronics_map.addSectorLane(2, 12, 3, 2, 3, 5)
    electronics_map.addSectorLane(2, 13, 3, 2, 3, 6)
    electronics_map.addSectorLane(2, 14, 3, 2, 3, 7)

    electronics_map.addSectorLane(2, 1, 4, 2, 0, 1)
    electronics_map.addSectorLane(2, 2, 4, 2, 0, 2)
    electronics_map.addSectorLane(2, 3, 4, 2, 0, 3)
    electronics_map.addSectorLane(2, 4, 4, 2, 0, 4)
    electronics_map.addSectorLane(2, 5, 4, 2, 0, 5)
    electronics_map.addSectorLane(2, 6, 4, 2, 0, 6)
    electronics_map.addSectorLane(2, 7, 4, 2, 0, 7)
    electronics_map.addSectorLane(2, 8, 4, 2, 1, 1)
    electronics_map.addSectorLane(2, 9, 4, 2, 1, 2)
    electronics_map.addSectorLane(2, 10, 4, 2, 1, 3)
    electronics_map.addSectorLane(2, 11, 4, 2, 1, 4)
    electronics_map.addSectorLane(2, 12, 4, 2, 1, 5)
    electronics_map.addSectorLane(2, 13, 4, 2, 1, 6)
    electronics_map.addSectorLane(2, 14, 4, 2, 1, 7)

    return electronics_map


dbImporter = EKLMDatabaseImporter()

if (mc):
    electronics_map = eklm_electronics_map(1)

    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importElectronicsMap(electronics_map)

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importElectronicsMap(electronics_map)

    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importElectronicsMap(electronics_map)

else:
    electronics_map = eklm_electronics_map(1)
    dbImporter.setIOV(0, 0, 3, -1)
    dbImporter.importElectronicsMap(electronics_map)

    electronics_map = eklm_electronics_map(2)
    dbImporter.setIOV(4, 0, -1, -1)
    dbImporter.importElectronicsMap(electronics_map)
