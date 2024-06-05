#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Prints info about TBC constants in database
#
# usage: basf2 getTBCInfo.py expNo runNo [globalTag or localDB]
# ---------------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter
import sys

argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "expNo runNo [globalTag or localDB]")
    sys.exit()
expNo = int(argvs[1])
runNo = int(argvs[2])

# Database
tag = '(main)'
if len(argvs) == 4:
    tag = argvs[3]
    if '.txt' in tag:
        b2.conditions.append_testing_payloads(tag)
    else:
        b2.conditions.append_globaltag(tag)


class PrintInfo(b2.Module):
    ''' Prints timebase calibration info '''

    def initialize(self):
        ''' Prints calibration status of boardstacks '''

        print()
        print('Experiment =', expNo, 'Run =', runNo, 'global tag =', tag)
        print()

        dbImporter = TOPDatabaseImporter()
        dbImporter.printSampleTimeCalibrationInfo()


# create path
main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [runNo], 'expList': [expNo]})
main.add_module(eventinfosetter)

# Geometry parameters
main.add_module('TOPGeometryParInitializer')

# Print TBC Info
main.add_module(PrintInfo())

# process single event
b2.process(main)
