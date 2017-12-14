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

# Specify the exp and run where iov is valid.
# N.B. -1 means unbound.
expFirst = 0
expLast = -1
runFirst = 0
runLast = -1

mapfile = FileSystem.findFile('data/cdc/ch_map.dat')
t0file = FileSystem.findFile('data/cdc/t0_v1.dat')
bwfile = FileSystem.findFile('data/cdc/badwire_v1.dat')
psfile = FileSystem.findFile('data/cdc/propspeed_v0.dat')
twfile = FileSystem.findFile('data/cdc/tw_off.dat')
xtfile = FileSystem.findFile('data/cdc/xt_v3.0.1_chebyshev.dat.gz')
sgfile = FileSystem.findFile('data/cdc/sigma_v2.dat')
dispfile = FileSystem.findFile('data/cdc/displacement_v1.1.dat')
alfile = FileSystem.findFile('data/cdc/alignment_v2.dat')
# misalfile = FileSystem.findFile('data/cdc/misalignment_v2.dat')
use_local_database("localDB/database.txt", "localDB")

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

dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)
# dbImporter = CDCDatabaseImporter()
dbImporter.importChannelMap(mapfile)
dbImporter.importTimeZero(t0file)
dbImporter.importBadWire(bwfile)
dbImporter.importPropSpeed(psfile)
dbImporter.importTimeWalk(twfile)
dbImporter.importXT(xtfile)
dbImporter.importSigma(sgfile)
dbImporter.importDisplacement(dispfile)
dbImporter.importWirPosAlign(alfile)
dbImporter.importADCDeltaPedestal()
# dbImporter.importWirPosMisalign(misalfile)

# dbImporter.printChannelMap()
# dbImporter.printTimeZero()
# dbImporter.printBadWire()
# dbImporter.printPropSpeed()
# dbImporter.printTimeWalk()
# dbImporter.printXT()
# dbImporter.printSigma()
# dbImporter.printDisplacement()
# dbImporter.printWirPosAlign()
# dbImporter.printWirPosMisalign()
