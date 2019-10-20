#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Beam Spot Monitor Script
one file for all BeamSpot payload in the local database
usage:
> basf2 beamSpotMonitor.py current_localDB
"""

import basf2
import ROOT
import sys

if __name__ == '__main__':

    current_localDB = sys.argv[1]

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

    basf2.reset_database()
    basf2.use_database_chain()
    basf2.use_central_database("data_reprocessing_prompt")
    basf2.use_local_database(current_localDB + "/database.txt", current_localDB, invertLogging=True)

    main = basf2.create_path()

    eventinfosetter = basf2.register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': evtList, 'expList': expList, 'runList': runList})
    main.add_module(eventinfosetter)

    main.add_module('BeamSpotMonitor')

    basf2.print_path(main)
    basf2.process(main)
