#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Generate a random channel mask for given fraction of dead / fraction of noisy
#   Sam Cunliffe, Jan Strube
# --------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter

# use_local_database()
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
dbImporter.generateFakeChannelMask(0.0, 0.0)
# dbImporter.generateFakeChannelMask(0.1, 0.2) # change these vaulus to increase the fractions of dead/noisy channels
