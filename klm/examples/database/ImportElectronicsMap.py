#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import BKLM electronics mapping payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import KLMDatabaseImporter, KLMElementNumbers, BKLMElementNumbers

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

# Database importer.
dbImporter = KLMDatabaseImporter()
dbImporter.loadBKLMElectronicsMap()

if mc:
    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importElectronicsMap()

    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importElectronicsMap()

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importElectronicsMap()

else:
    # Switch lanes for real-data map.
    # The fibers of layer 1 and 2 are switched in BB6.
    dbImporter.setElectronicsMapLane(KLMElementNumbers.c_BKLM,
                                     BKLMElementNumbers.c_BackwardSection,
                                     7, 1, 2)
    dbImporter.setElectronicsMapLane(KLMElementNumbers.c_BKLM,
                                     BKLMElementNumbers.c_BackwardSection,
                                     7, 2, 1)

    dbImporter.setIOV(1, 0, 9, -1)
    dbImporter.importElectronicsMap()

    # Import mapping for experiment 10 and 11.
    dbImporter.loadBKLMElectronicsMap(True)

    # Still switch the lanes.
    dbImporter.setElectronicsMapLane(KLMElementNumbers.c_BKLM,
                                     BKLMElementNumbers.c_BackwardSection,
                                     7, 1, 2)
    dbImporter.setElectronicsMapLane(KLMElementNumbers.c_BKLM,
                                     BKLMElementNumbers.c_BackwardSection,
                                     7, 2, 1)

    dbImporter.setIOV(10, 0, -1, -1)
    dbImporter.importElectronicsMap()
