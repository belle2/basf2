#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import BKLM electronics mapping payloads.

import sys
from ROOT.Belle2 import BKLMDatabaseImporter

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

dbImporter = BKLMDatabaseImporter()
dbImporter.loadDefaultBklmElectronicMapping()

# Switch lanes for real-data map
# since the fibers of layer 1 and 2 are switched in BB6.
if (not mc):
    dbImporter.setElectronicMappingLane(0, 7, 1, 2)
    dbImporter.setElectronicMappingLane(0, 7, 2, 1)

dbImporter.importBklmElectronicMapping()
