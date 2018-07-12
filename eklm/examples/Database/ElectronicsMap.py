#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

from basf2 import *
import ROOT
from ROOT.Belle2 import EKLMDatabaseImporter

set_log_level(LogLevel.INFO)

eventinfosetter = register_module('EventInfoSetter')

# Gearbox
gearbox = register_module('Gearbox')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

process(main)

dbImporter = EKLMDatabaseImporter()
dbImporter.loadDefaultElectronicsMap()

# EKLM electronics map
#
#     backward (endcap == 1)     forward (endcap == 2)
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

# Backward endcap
dbImporter.addSectorLane(1, 1, 1, 3, 0, 1)
dbImporter.addSectorLane(1, 2, 1, 3, 0, 2)
dbImporter.addSectorLane(1, 3, 1, 3, 0, 3)
dbImporter.addSectorLane(1, 4, 1, 3, 0, 4)
dbImporter.addSectorLane(1, 5, 1, 3, 0, 5)
dbImporter.addSectorLane(1, 6, 1, 3, 0, 6)
dbImporter.addSectorLane(1, 7, 1, 3, 1, 1)
dbImporter.addSectorLane(1, 8, 1, 3, 1, 2)
dbImporter.addSectorLane(1, 9, 1, 3, 1, 3)
dbImporter.addSectorLane(1, 10, 1, 3, 1, 4)
dbImporter.addSectorLane(1, 11, 1, 3, 1, 5)
dbImporter.addSectorLane(1, 12, 1, 3, 1, 6)

dbImporter.addSectorLane(1, 1, 2, 3, 2, 1)
dbImporter.addSectorLane(1, 2, 2, 3, 2, 2)
dbImporter.addSectorLane(1, 3, 2, 3, 2, 3)
dbImporter.addSectorLane(1, 4, 2, 3, 2, 4)
dbImporter.addSectorLane(1, 5, 2, 3, 2, 5)
dbImporter.addSectorLane(1, 6, 2, 3, 2, 6)
dbImporter.addSectorLane(1, 7, 2, 3, 3, 1)
dbImporter.addSectorLane(1, 8, 2, 3, 3, 2)
dbImporter.addSectorLane(1, 9, 2, 3, 3, 3)
dbImporter.addSectorLane(1, 10, 2, 3, 3, 4)
dbImporter.addSectorLane(1, 11, 2, 3, 3, 5)
dbImporter.addSectorLane(1, 12, 2, 3, 3, 6)

dbImporter.addSectorLane(1, 1, 3, 4, 0, 1)
dbImporter.addSectorLane(1, 2, 3, 4, 0, 2)
dbImporter.addSectorLane(1, 3, 3, 4, 0, 3)
dbImporter.addSectorLane(1, 4, 3, 4, 0, 4)
dbImporter.addSectorLane(1, 5, 3, 4, 0, 5)
dbImporter.addSectorLane(1, 6, 3, 4, 0, 6)
dbImporter.addSectorLane(1, 7, 3, 4, 1, 1)
dbImporter.addSectorLane(1, 8, 3, 4, 1, 2)
dbImporter.addSectorLane(1, 9, 3, 4, 1, 3)
dbImporter.addSectorLane(1, 10, 3, 4, 1, 4)
dbImporter.addSectorLane(1, 11, 3, 4, 1, 5)
dbImporter.addSectorLane(1, 12, 3, 4, 1, 6)

dbImporter.addSectorLane(1, 1, 4, 4, 2, 1)
dbImporter.addSectorLane(1, 2, 4, 4, 2, 2)
dbImporter.addSectorLane(1, 3, 4, 4, 2, 3)
dbImporter.addSectorLane(1, 4, 4, 4, 2, 4)
dbImporter.addSectorLane(1, 5, 4, 4, 2, 5)
dbImporter.addSectorLane(1, 6, 4, 4, 2, 6)
dbImporter.addSectorLane(1, 7, 4, 4, 3, 1)
dbImporter.addSectorLane(1, 8, 4, 4, 3, 2)
dbImporter.addSectorLane(1, 9, 4, 4, 3, 3)
dbImporter.addSectorLane(1, 10, 4, 4, 3, 4)
dbImporter.addSectorLane(1, 11, 4, 4, 3, 5)
dbImporter.addSectorLane(1, 12, 4, 4, 3, 6)

# Forward endcap
dbImporter.addSectorLane(2, 1, 1, 1, 2, 1)
dbImporter.addSectorLane(2, 2, 1, 1, 2, 2)
dbImporter.addSectorLane(2, 3, 1, 1, 2, 3)
dbImporter.addSectorLane(2, 4, 1, 1, 2, 4)
dbImporter.addSectorLane(2, 5, 1, 1, 2, 5)
dbImporter.addSectorLane(2, 6, 1, 1, 2, 6)
dbImporter.addSectorLane(2, 7, 1, 1, 2, 7)
dbImporter.addSectorLane(2, 8, 1, 1, 3, 1)
dbImporter.addSectorLane(2, 9, 1, 1, 3, 2)
dbImporter.addSectorLane(2, 10, 1, 1, 3, 3)
dbImporter.addSectorLane(2, 11, 1, 1, 3, 4)
dbImporter.addSectorLane(2, 12, 1, 1, 3, 5)
dbImporter.addSectorLane(2, 13, 1, 1, 3, 6)
dbImporter.addSectorLane(2, 14, 1, 1, 3, 7)

dbImporter.addSectorLane(2, 1, 2, 1, 0, 1)
dbImporter.addSectorLane(2, 2, 2, 1, 0, 2)
dbImporter.addSectorLane(2, 3, 2, 1, 0, 3)
dbImporter.addSectorLane(2, 4, 2, 1, 0, 4)
dbImporter.addSectorLane(2, 5, 2, 1, 0, 5)
dbImporter.addSectorLane(2, 6, 2, 1, 0, 6)
dbImporter.addSectorLane(2, 7, 2, 1, 0, 7)
dbImporter.addSectorLane(2, 8, 2, 1, 1, 1)
dbImporter.addSectorLane(2, 9, 2, 1, 1, 2)
dbImporter.addSectorLane(2, 10, 2, 1, 1, 3)
dbImporter.addSectorLane(2, 11, 2, 1, 1, 4)
dbImporter.addSectorLane(2, 12, 2, 1, 1, 5)
dbImporter.addSectorLane(2, 13, 2, 1, 1, 6)
dbImporter.addSectorLane(2, 14, 2, 1, 1, 7)

dbImporter.addSectorLane(2, 1, 3, 2, 2, 1)
dbImporter.addSectorLane(2, 2, 3, 2, 2, 2)
dbImporter.addSectorLane(2, 3, 3, 2, 2, 3)
dbImporter.addSectorLane(2, 4, 3, 2, 2, 4)
dbImporter.addSectorLane(2, 5, 3, 2, 2, 5)
dbImporter.addSectorLane(2, 6, 3, 2, 2, 6)
dbImporter.addSectorLane(2, 7, 3, 2, 2, 7)
dbImporter.addSectorLane(2, 8, 3, 2, 3, 1)
dbImporter.addSectorLane(2, 9, 3, 2, 3, 2)
dbImporter.addSectorLane(2, 10, 3, 2, 3, 3)
dbImporter.addSectorLane(2, 11, 3, 2, 3, 4)
dbImporter.addSectorLane(2, 12, 3, 2, 3, 5)
dbImporter.addSectorLane(2, 13, 3, 2, 3, 6)
dbImporter.addSectorLane(2, 14, 3, 2, 3, 7)

dbImporter.addSectorLane(2, 1, 4, 2, 0, 1)
dbImporter.addSectorLane(2, 2, 4, 2, 0, 2)
dbImporter.addSectorLane(2, 3, 4, 2, 0, 3)
dbImporter.addSectorLane(2, 4, 4, 2, 0, 4)
dbImporter.addSectorLane(2, 5, 4, 2, 0, 5)
dbImporter.addSectorLane(2, 6, 4, 2, 0, 6)
dbImporter.addSectorLane(2, 7, 4, 2, 0, 7)
dbImporter.addSectorLane(2, 8, 4, 2, 1, 1)
dbImporter.addSectorLane(2, 9, 4, 2, 1, 2)
dbImporter.addSectorLane(2, 10, 4, 2, 1, 3)
dbImporter.addSectorLane(2, 11, 4, 2, 1, 4)
dbImporter.addSectorLane(2, 12, 4, 2, 1, 5)
dbImporter.addSectorLane(2, 13, 4, 2, 1, 6)
dbImporter.addSectorLane(2, 14, 4, 2, 1, 7)

# Import
dbImporter.importElectronicsMap()
