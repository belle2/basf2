#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Example of using Database importer for importing Module T0 constants to local database
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
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
process(main)

# and then run the importer (note: input file is not there - must change the path!)
dbImporter = TOPDatabaseImporter()

# import constants
dbImporter.importDummyCalModuleAlignment()
dbImporter.importDummyCalModuleT0()
dbImporter.importDummyCalChannelT0()
dbImporter.importDummyCalTimebase()
dbImporter.importDummyCalChannelNoise()
dbImporter.importDummyCalChannelPulseHeight()
dbImporter.importDummyCalChannelRQE()
dbImporter.importDummyCalChannelThresholdEff()
dbImporter.importDummyCalChannelThreshold()
dbImporter.importDummyCalCommonT0()
dbImporter.importDummyCalIntegratedCharge()
