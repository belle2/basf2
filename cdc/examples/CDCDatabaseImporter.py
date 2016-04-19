#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
CDC Database importer.
Simple example.
"""

from basf2 import *
import ROOT
from ROOT.Belle2 import CDCDatabaseImporter
from ROOT.Belle2 import FileSystem
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

mapfile = FileSystem.findFile('data/cdc/ch_map.dat')
t0file = FileSystem.findFile('data/cdc/t0.dat')
use_local_database("cdc_crt/database.txt", "cdc_crt")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# process single event
process(main)

dbImporter = CDCDatabaseImporter()
dbImporter.importChannelMap(mapfile)
dbImporter.importTimeZero(t0file)
# dbImporter.printChannelMap()
# dbImporter.printTimeZero()
