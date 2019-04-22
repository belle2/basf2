#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Beam Spot Monitor Script
one file for all BeamSpot payload in the local database
usage:
> open this file and edit the file:
. current_localDB =
Then execute the script:
> basf2 beamSpotMonitor.py
"""

from basf2 import *
import ROOT

current_localDB = "put_here_the_path_to_your_localDB"

expList = []
runList = []
evtList = []
with open(current_localDB + '/database.txt') as fp:
    for line in fp:
        run = line.split(',')
        exp = run[0].split(' ')
        expList.append(int(exp[2]))
        runList.append(int(run[1]))
        evtList.append(int(1))

reset_database()
use_database_chain()
use_central_database("data_reprocessing_prompt")
use_local_database(current_localDB + "/database.txt", current_localDB, invertLogging=True)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': evtList, 'expList': expList, 'runList': runList})
main.add_module(eventinfosetter)

main.add_module('BeamSpotMonitor')

# process single event
print_path(main)
process(main)
