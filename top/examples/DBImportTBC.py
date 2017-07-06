#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Example of using Database importer for importing TBC constants to local database
# User has to set the path to TOPTimeBaseCalibrator output files correctly
# Name and location of local DB can also be changed
# --------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

# define a local database (will be created automatically, if doesn't exist)
use_local_database("localDB/localDB.txt", "localDB", False)

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
process(main)

# and then run the importer (note: input file is not there - must change the path!)
dbImporter = TOPDatabaseImporter()

dir = 'tbc'  # location of output files from TOPTimeBaseCalibrator
fNames = glob.glob(dir + '/*.root')
if len(fNames) == 0:
    print('No root files found in', dir)
    sys.exit()
fileNames = ''

# convert a list to a single string (since list doesn't work for some reasons)
for fName in fNames:
    fileNames = fileNames + ' ' + fName

# import constants
dbImporter.importSampleTimeCalibration(fileNames)
