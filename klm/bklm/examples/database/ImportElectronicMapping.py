#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import BKLM electronics mapping payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import BKLMDatabaseImporter

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

# Import database
dbImporter = BKLMDatabaseImporter()
dbImporter.loadDefaultElectronicMapping()

if mc:
    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importElectronicMapping()

    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importElectronicMapping()

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importElectronicMapping()

else:
    # Switch lanes for real-data map.
    # since the fibers of layer 1 and 2 are switched in BB6.
    dbImporter.setElectronicMappingLane(0, 7, 1, 2)
    dbImporter.setElectronicMappingLane(0, 7, 2, 1)

    dbImporter.setIOV(1, 0, 9, -1)
    dbImporter.importElectronicMapping()

    # Import mapping for experiment 10 and 11
    dbImporter.loadDefaultElectronicMapping(True)

    # Still switch the lanes
    dbImporter.setElectronicMappingLane(0, 7, 1, 2)
    dbImporter.setElectronicMappingLane(0, 7, 2, 1)

    dbImporter.setIOV(10, 0, 11, -1)
    dbImporter.importElectronicMapping()
