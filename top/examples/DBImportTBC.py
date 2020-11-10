#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Example of using Database importer for importing TBC constants to local database
# User has to set the path to TOPTimeBaseCalibrator output files correctly
# Name and location of local DB can also be changed
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter
import sys
import glob

# define a local database with write access
# (will be created automatically, if doesn't exist)
b2.use_local_database("localDB/localDB.txt", "localDB", False)

# create path
main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
b2.process(main)

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
