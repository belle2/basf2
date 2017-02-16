#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------------
# Example of steering file for importing calibration constants to local database
# Kichimi-san calibration constants
# ------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

use_local_database("localDB/localDB.txt", "localDB", False)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

gearbox = register_module('Gearbox')
main.add_module(gearbox)

geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

process(main)

# and run the importer
dbImporter = TOPDatabaseImporter()

# import Kichimi-san TBC (given as histograms in root files)
dir = '/ghi/fs01/belle2/bdata/group/detector/TOP/DB_TBT0_20170212'
fNames = glob.glob(dir + '/TB/s*.root')
if len(fNames) == 0:
    print('No root files found in', dir)
    sys.exit()
fileNames = ''
# convert a list to a single string (since vector doesn't work for some reason)
for fName in fNames:
    fileNames = fileNames + ' ' + fName
# import constants
dbImporter.importSampleTimeCalibrationKichimi(fileNames)

# import Kichimi-san T0 (given as histograms in root files)
fNames = glob.glob(dir + '/T0/s*.root')
if len(fNames) == 0:
    print('No root files found in', dir)
    sys.exit()
fileNames = ''
# convert a list to a single string (since vector doesn't work for some reason)
for fName in fNames:
    fileNames = fileNames + ' ' + fName
# import constants
dbImporter.importChannelT0CalibrationKichimi(fileNames)
