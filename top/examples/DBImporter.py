#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Example of using Database importer
# Note: this example is not meant to be executed, but to see how to write the code
# --------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

# define local database with write access
use_local_database("localDB/localDB.txt", "localDB", False)

# create path
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
process(main)

# and then run the importer
dbImporter = TOPDatabaseImporter()
dbImporter.importDummyCalTimebase(0, 0, 0, -1)
