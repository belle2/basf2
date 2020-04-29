#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Import clocks payloads
# --------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import ClockDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

# Create path
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# process single event
process(main)

# define a local database (will be created automatically, if doesn't exist)
use_local_database("localDB/localDB.txt")

# and then run the importer
dbImporter = ClockDatabaseImporter()

dbImporter.setGlobalClock(508.887/4.)

dbImporter.setClock('top', 'sampling', 6)
dbImporter.setClock('svd', 'sampling', 4)
dbImporter.setClock('ecl', 'sampling', 3 * 24)
dbImporter.setClock('ecl', 'fitting', 3)
dbImporter.setClock('klm', 'dummy', 5)

# import constants
dbImporter.importClock()
