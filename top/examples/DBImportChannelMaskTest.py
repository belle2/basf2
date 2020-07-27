#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Generate a random channel mask for given fraction of dead / fraction of noisy
#   Sam Cunliffe, Jan Strube
# --------------------------------------------------------------------------------

from basf2 import *
import os
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter

# define local database with write access
use_local_database("localDB/localDB.txt", "localDB", False)

# create path
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
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
dbImporter.generateFakeChannelMask(0.0, 0.0)
# change these vaulus to increase the fractions of dead/noisy channels
# dbImporter.generateFakeChannelMask(0.1, 0.2)
