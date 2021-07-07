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
Beam Spot Calibration Check Script
one file for all BeamSpot payload in the local database
usage:
> basf2 beamSpotMonitor.py current_localDB
"""

import basf2
import sys
from basf2 import conditions as b2conditions

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

    b2conditions.testing_payloads = [str(current_localDB) + "/database.txt"]

    main = basf2.create_path()

    eventinfosetter = basf2.register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': evtList, 'expList': expList, 'runList': runList})
    main.add_module(eventinfosetter)

    main.add_module('BeamSpotMonitor')

    basf2.print_path(main)
    basf2.process(main)
