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

# --------------------------------------------------------------------
# useful tool for checking the status of TOP calibration in central DB
#
# usage: basf2 checkCalibDB.py expNo runNo [globalTag]
# --------------------------------------------------------------------

argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "expNo runNo [globalTag]")
    sys.exit()

expNo = int(argvs[1])
runNo = int(argvs[2])
tag = '(default)'
if len(argvs) == 4:
    tag = argvs[3]


class CheckCalibDB(b2.Module):
    ''' print calibration status of TOPCal payloads '''

    def printChannel(self, payload):
        ''' print status of a payload given by the argument '''

        db = Belle2.PyDBObj(payload)
        if not db:
            b2.B2ERROR(payload + ' not found')
            return
        calibrated = 0
        all = 0
        for moduleID in range(1, 17):
            for channel in range(512):
                all += 1
                if db.isCalibrated(moduleID, channel):
                    calibrated += 1
        print(payload + ': ' + str(calibrated) + '/' + str(all) + ' calibrated')

    def printChannelMask(self):
        ''' print status of channel masks '''

        payload = 'TOPCalChannelMask'
        db = Belle2.PyDBObj(payload)
        if not db:
            b2.B2ERROR(payload + ' not found')
            return
        active = db.getNumOfActiveChannels()
        all = db.getNumOfChannels()
        print(payload + ': ' + str(active) + '/' + str(all) + ' active')

    def printModule(self, payload):
        ''' print status of a payload given by the argument '''

        db = Belle2.PyDBObj(payload)
        if not db:
            b2.B2ERROR(payload + ' not found')
            return
        calibrated = 0
        all = 0
        for moduleID in range(1, 17):
            all += 1
            if db.isCalibrated(moduleID):
                calibrated += 1
        print(payload + ': ' + str(calibrated) + '/' + str(all) + ' calibrated')

    def printCommon(self, payload):
        ''' print status of a payload given by the argument '''

        db = Belle2.PyDBObj(payload)
        if not db:
            b2.B2ERROR(payload + ' not found')
            return
        calibrated = 0
        all = 1
        if db.isCalibrated():
            calibrated += 1
        print(payload + ': ' + str(calibrated) + '/' + str(all) + ' calibrated')

    def printTimeBase(self):
        ''' print status of time base calibration '''

        payload = 'TOPCalTimebase'
        db = Belle2.PyDBObj(payload)
        if not db:
            b2.B2ERROR(payload + ' not found')
            return
        calibrated = 0
        all = 8192
        for sampleTimes in db.getSampleTimes():
            if sampleTimes.isCalibrated():
                calibrated += 1
        print(payload + ': ' + str(calibrated) + '/' + str(all) + ' calibrated')

    def event(self):
        ''' event processing '''

        print()
        print('Calibration status of GT =', tag)
        print('Experiment =', expNo, 'Run =', runNo)
        print()
        self.printTimeBase()
        self.printChannel('TOPCalChannelT0')
        self.printModule('TOPCalModuleT0')
        self.printCommon('TOPCalCommonT0')
        self.printChannel('TOPCalChannelNoise')
        self.printChannel('TOPCalChannelPulseHeight')
        self.printChannel('TOPCalChannelRQE')
        self.printChannel('TOPCalChannelThresholdEff')
        self.printChannel('TOPCalChannelThreshold')
        self.printChannel('TOPCalIntegratedCharge')
        self.printModule('TOPCalModuleAlignment')
        self.printChannelMask()
        print()


# Central database
if len(argvs) == 4:
    b2.use_central_database(tag)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [runNo], 'expList': [expNo]})
main.add_module(eventinfosetter)

# Run checker
main.add_module(CheckCalibDB())

# Process events
b2.process(main)
