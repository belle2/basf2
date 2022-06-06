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
Simple example to import CDClayerTimeCut payload from json file to the database.
"""

import basf2
from ROOT.Belle2 import FileSystem
from ROOT.Belle2 import CDCDatabaseImporter

# Input json file containing the CDClayerTimeCut
# N.B. In the json file, -1 as upper limit means unbound.
INPUT = FileSystem.findFile("data/cdc/CDClayerTimeCut_example.json")

# Specify the exp and run where iov is valid.
# N.B. -1 means unbound.
expFirst = 0
expLast = -1
runFirst = 0
runLast = -1

basf2.conditions.testing_payloads = ["localdb/database.txt"]

main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# process single event
basf2.process(main)

dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)
dbImporter.importCDClayerTimeCut(INPUT)

# This line print the result for exp0/run0
dbImporter.printCDClayerTimeCut()
