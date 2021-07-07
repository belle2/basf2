#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
from ROOT import Belle2

# ------------------------------------------------------------------------
# useful tool for checking the status of common T0 calibration in DB
#
# usage: basf2 checkDB-commonT0.py expNo runFirst runLast globalTag/localDB
# -------------------------------------------------------------------------

argvs = sys.argv
if len(argvs) < 5:
    print("usage: basf2", argvs[0], "expNo runFirst runLast globalTag/localDB")
    sys.exit()

expNo = int(argvs[1])
runFirst = int(argvs[2])
runLast = int(argvs[3])
tag = argvs[4]


class CheckCalibDB(b2.Module):
    ''' print content of TOPCalCommonT0 '''

    def initialize(self):
        ''' initialize '''

        #: payload
        self.db = Belle2.PyDBObj('TOPCalCommonT0')
        #: last run number
        self.lastRun = None
        #: bunch time separation
        self.bunchTimeSep = 47.163878 / 24

        print()
        print('Common T0 calibration status of GT =', tag)
        print('Experiment =', expNo, 'Runs =', runFirst, 'to', runLast)
        print()

    def event(self):
        ''' event processing '''

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        runNo = 'r' + '{:0=5d}'.format(evtMetaData.getRun())

        if not self.db:
            b2.B2ERROR(runNo + ': payload not found')
            return
        if not self.db.hasChanged():
            self.lastRun = runNo
            return
        if self.lastRun:
            print('... to ' + self.lastRun)
            self.lastRun = None

        if self.db.isCalibrated():
            status = 'calibrated'
        elif self.db.isUnusable():
            status = 'unusable'
        elif self.db.isRoughlyCalibrated():
            status = 'roughly calibrated'
        else:
            status = 'default'
        n = round(self.db.getT0() / self.bunchTimeSep, 0)
        if n == 0:
            print(runNo + ': T0 =',
                  round(self.db.getT0(), 4), '+/-', round(self.db.getT0Error(), 4),
                  status)
        else:
            print(runNo + ': T0 =',
                  round(self.db.getT0(), 4), '+/-', round(self.db.getT0Error(), 4),
                  status, '-- out of range')

    def terminate(self):
        ''' terminate '''

        if self.lastRun:
            print('... to ' + self.lastRun)
            self.lastRun = None


# Database
if '.txt' in tag:
    b2.use_local_database(tag)
else:
    b2.use_central_database(tag)

# Create path
main = b2.create_path()

# Set number of events to generate
evtList = [1 for run in range(runFirst, runLast + 1)]
runList = [run for run in range(runFirst, runLast + 1)]
expList = [expNo for run in range(runFirst, runLast + 1)]

# Event info setter
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': evtList, 'runList': runList, 'expList': expList})
main.add_module(eventinfosetter)

# Run checker
main.add_module(CheckCalibDB())

# Process events
b2.process(main)
