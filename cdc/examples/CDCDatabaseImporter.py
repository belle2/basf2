#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
CDC Database importer.
Simple example.
"""

import basf2 as b2
from ROOT.Belle2 import CDCDatabaseImporter
from ROOT.Belle2 import FileSystem

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
fffile = FileSystem.findFile('data/cdc/ffactor.dat')
dispfile = FileSystem.findFile('data/cdc/displacement_v1.1.dat')
alfile = FileSystem.findFile('data/cdc/alignment_v2.dat')
# misalfile = FileSystem.findFile('data/cdc/misalignment_v2.dat')
fefile = FileSystem.findFile('cdc/data/fee_phase3.dat')
# edfile = FileSystem.findFile('cdc/data/edeptoadc.dat')
b2.use_local_database("localDB/database.txt", "localDB")

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# process single event
b2.process(main)

dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)
# dbImporter = CDCDatabaseImporter()
dbImporter.importChannelMap(mapfile)
# dbImporter.importFEEParam(fefile)
dbImporter.importTimeZero(t0file)
dbImporter.importBadWire(bwfile)
dbImporter.importPropSpeed(psfile)
dbImporter.importTimeWalk(twfile)
dbImporter.importXT(xtfile)
dbImporter.importSigma(sgfile)
dbImporter.importFFactor(fffile)
dbImporter.importDisplacement(dispfile)
dbImporter.importWirPosAlign(alfile)
dbImporter.importADCDeltaPedestal()
dbImporter.importFEElectronics(fefile)
# dbImporter.importEDepToADC(edfile)
# dbImporter.importWirPosMisalign(misalfile)

# dbImporter.printChannelMap()
# dbImporter.printFEEParam()
# dbImporter.printTimeZero()
# dbImporter.printBadWire()
# dbImporter.printPropSpeed()
# dbImporter.printTimeWalk()
# dbImporter.printXT()
# dbImporter.printSigma()
# dbImporter.printFFactor()
# dbImporter.printDisplacement()
# dbImporter.printWirPosAlign()
# dbImporter.printWirPosMisalign()
# dbImporter.printFEElectronics()
# dbImporter.printEDepToADC()
