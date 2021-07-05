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
# Generate a random channel mask for given fraction of dead / fraction of noisy
#   Sam Cunliffe, Jan Strube
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter

# define local database with write access
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

# Geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
b2.process(main)

# and then run the importer
dbImporter = TOPDatabaseImporter()
dbImporter.generateFakeChannelMask(0.0, 0.0)
# change these vaulus to increase the fractions of dead/noisy channels
# dbImporter.generateFakeChannelMask(0.1, 0.2)
