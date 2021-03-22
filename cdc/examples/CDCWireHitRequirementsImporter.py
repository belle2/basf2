#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
CDC Database importer.
Simple example to import CDCWireHitRequirements to the database.
"""

import basf2
from ROOT.Belle2 import FileSystem
from ROOT.Belle2 import CDCDatabaseImporter

# Input json file containing the CDCWireHitRequirements
# N.B. In the json file, -1 as upper limit means unbound.
INPUT = FileSystem.findFile("cdc/data/CDCWireHitRequirements_example.json")

# Specify the exp and run where iov is valid.
# N.B. -1 means unbound.
expFirst = 0
expLast = -1
runFirst = 0
runLast = -1
basf2.use_local_database("localdb/database.txt", "localdb")

main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# process single event
basf2.process(main)

dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)
dbImporter.importCDCWireHitRequirements(INPUT)

# This line print the result for exp0/run0
dbImporter.printCDCWireHitRequirements()
