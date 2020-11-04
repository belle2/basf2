#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Example of using Database importer for importing Module T0 constants to local database
# Name and location of local DB can also be changed
# --------------------------------------------------------------------------------

import basf2 as b2
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

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

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants
dbImporter.importPmtPulseHeightFitResult("/group/belle2/users/okuto/gain_summary_bfield_newHV.root")
