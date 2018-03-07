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

# use_local_database()
# use_local_database("test_bklm.txt", "test_payloads")
# use use_central_database for uploading data to PNNL
# use_central_database("test_bklm", LogLevel.WARNING);
use_central_database("development", LogLevel.WARNING)


# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()

# download data if it is not available locally
# make sure you change paths
# os.path.isfile('BKLMElectronicsMapping_test.xml'):

# create a gearbox module to read the data so it can be used
paramloader = register_module('Gearbox')
# pathname = 'file://%s/AllData/' % (os.getcwd())
# paramloader.param('backends', [pathname])
# paramloader.initialize()

main = create_path()
main.add_module(eventinfo)
main.add_module(paramloader)
process(main)

# and run the importer
dbImporter = BKLMDatabaseImporter()
# dbImporter.exportBklmElectronicMapping()
# dbImporter.exportBklmDigitizationParams()
# dbImporter.exportBklmADCThreshold()
