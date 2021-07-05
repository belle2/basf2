#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------------
# Import payload TOPCalTimeWalk
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter

# Create path
main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Initialize TOP geometry parameters from gearbox
main.add_module('TOPGeometryParInitializer', useDB=False)

# process single event
b2.process(main)

# define a local database to save payloads(created automatically, if not existing)
b2.conditions.expert_settings(save_payloads="localDB/localDB.txt")

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

b2.B2RESULT("Done")
