#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
CDC Database importer.
Simple example to import CDCBadBoards to the database.
"""

import basf2
from ROOT import Belle2  # noqa: make the Belle2 namespace available
from ROOT.Belle2 import CDCDatabaseImporter

basf2.conditions.expert_settings(save_payloads='boards/database.txt')

INPUT = "CDCBadBoards_example.txt"
# Specify the exp and run where iov is valid.
# N.B. -1 means unbound.
expFirst = 0
expLast = -1
runFirst = 0
runLast = -1

dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)

# basf2.conditions.testing_payloads = ["boards/database.txt"]
basf2.conditions.prepend_testing_payloads('boards/database.txt')

main = basf2.create_path()
# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)
# process single event
basf2.process(main)

action = ['IMPORT', 'TEST']  # [0]

for act in action:
    if act == 'IMPORT':
        dbImporter.importBadBoards(INPUT)
        basf2.conditions.reset()

    elif act == 'TEST':
        dbImporter.printBadBoards()
