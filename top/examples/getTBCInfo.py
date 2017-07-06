#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Get info about TBC constants in database
# Name and location of local DB can be changed if needed
# ---------------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

# database
reset_database()
use_local_database("localDB/localDB.txt", "localDB")

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

# and then run the importer
dbImporter = TOPDatabaseImporter()
dbImporter.getSampleTimeCalibrationInfo()
