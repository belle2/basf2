#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Import payload TOPCalTimeWalk
# --------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob

# Create path
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Initialize TOP geometry parameters from gearbox
main.add_module('TOPGeometryParInitializer', useDB=False)

# process single event
process(main)

# define a local database to save payloads(created automatically, if not existing)
conditions.expert_settings(save_payloads="localDB/localDB.txt")

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants
params = [-106.757,
          0.317404,
          4.11324e-06,
          -1.31485e-07,
          1.10216e-11]  # ps
a = 8806.6  # ps
b = 5.25439e-05  # ps

for i in range(len(params)):
    params[i] /= 1000  # ns
a /= 1000  # ns
b /= 1000  # ns

print('time_walk =', params)
print('a =', a, 'b = ', b)

dbImporter.importTimeWalk(params, a, b)

B2RESULT("Done")
