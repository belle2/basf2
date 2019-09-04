#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *
import ROOT
from ROOT.Belle2 import BKLMDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

set_log_level(LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

# Import database
dbImporter = BKLMDatabaseImporter()
dbImporter.loadDefaultBklmElectronicMapping()

# Switch lanes for real-data map.
if (not mc):
    dbImporter.setElectronicMappingLane(0, 7, 1, 2)
    dbImporter.setElectronicMappingLane(0, 7, 2, 1)

dbImporter.importBklmElectronicMapping()
