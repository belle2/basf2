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

# Switch lanes for real-data map.
# since the fibers of layer 1 and 2 are switched in BB6.
if (not mc):
    dbImporter.setElectronicMappingLane(0, 7, 1, 2)
    dbImporter.setElectronicMappingLane(0, 7, 2, 1)

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importElectronicMapping()
