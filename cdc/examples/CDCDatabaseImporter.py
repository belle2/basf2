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
bwfile = FileSystem.findFile('data/cdc/badwire_off.dat')
psfile = FileSystem.findFile('data/cdc/propspeed_v0.dat')
twfile = FileSystem.findFile('data/cdc/tw_off.dat')
xtfile = FileSystem.findFile('data/cdc/xt_v3.dat')
sgfile = FileSystem.findFile('data/cdc/sigma_v1.dat')
alfile = FileSystem.findFile('data/cdc/alignment_v1.dat')
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
dbImporter.importBadWire(bwfile)
dbImporter.importPropSpeed(psfile)
dbImporter.importTimeWalk(twfile)
dbImporter.importXT(xtfile)
dbImporter.importSigma(sgfile)
dbImporter.importWirPosAlign(alfile)

# dbImporter.printChannelMap()
# dbImporter.printTimeZero()
# dbImporter.printBadWire()
# dbImporter.printPropSpeed()
# dbImporter.printTimeWalk()
# dbImporter.printXT()
# dbImporter.printSigma()
# dbImporter.printWirPosAlign()
