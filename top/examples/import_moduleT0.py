#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Example of using Database importer for importing Module T0 constants to local database
# Name and location of local DB can also be changed
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter

# define a local database (will be created automatically, if doesn't exist)
b2.use_local_database("localDB/localDB.txt", "localDB", False)

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

# import constants
dbImporter.importModuleT0Calibration("/group/belle2/users/senga/minuit021_390.dat")
