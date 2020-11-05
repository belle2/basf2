#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test EventLevelClusteringInfo counting of out of time ECLCalDigits.

Creates 1 ECLCalDigit in each ECL detector region (FWD, Barrel,  BWD).

The created digits cover all possible combinations of:
time above threshold for counting
time below threshold for counting
energy above threshold for counting
energy below threshold for counting

In order to be counted, both energy and time have to be above threshold
"""

import basf2 as b2
from ROOT import Belle2
from unittest import TestCase
import itertools

from b2test_utils import skip_test_if_light
skip_test_if_light()  # light builds don't contain ECLCalDigits

b2.set_random_seed(42)

# global variable that holds expected number of out of time digits
expectOutOfTime = {"FWD": 0, "BRL": 0, "BWD": 0}


class addECLCalDigitsModule(b2.Module):
    """
    Add combinations of ECLCalDigits above/below threshold to be counted as out of time
    """

    def __init__(self):
        """
        Prepare ECLCalDigits parameters
        """
        super().__init__()

        #: count number of times event method is called (each time use different combinations of ECLCalDigits
        self.eventCounter = 0

        aboveEnergyThresh = [True, False]
        aboveTimeThresh = [True, False]
        thresholdNames = ["aboveEnergythresh", "aboveTimethresh"]

        thresholdsPerRegion = [dict(zip(thresholdNames, thresholds))
                               for thresholds in itertools.product(aboveEnergyThresh, aboveTimeThresh)]

        fwdThresholds, brlThresholds, bwdThresholds = itertools.tee(thresholdsPerRegion, 3)

        regions = ["FWD", "BRL", "BWD"]

        #: parameters to create custom ECLCalDigits
        self.digitParams = [dict(zip(regions, thresholds))
                            for thresholds in itertools.product(fwdThresholds, brlThresholds, bwdThresholds)]

        #: default energy threshold
        self.energyThresh = -1
        #: default time threshold
        self.timeThresh = -1

    def initialize(self):
        """ module initialize - register ECLCalDigit in datastore """

        #: ECLCalDigits datastore
        self.eclCalDigits = Belle2.PyStoreArray(Belle2.ECLCalDigit.Class(), "ECLCalDigits")
        self.eclCalDigits.registerInDataStore()

    def event(self):
        """
        Add ECLCalDigits according to self.digitParams
        """
        eclCalDigits = Belle2.PyStoreArray('ECLCalDigits')

        # cellIds for corresponding to different regions
        cellId = {"FWD": 1, "BRL": 1153, "BWD": 7777}

        b2.B2DEBUG(37, "Event " + str(self.eventCounter))

        # ECLCalDigit parameter for this event
        digitParam = self.digitParams[self.eventCounter]

        # Loop on detector regions
        for region in cellId:

            # Create new ECLCalDigit
            eclCalDigit = Belle2.ECLCalDigit()

            # Fill ECLCalDigit
            eclCalDigit.setCellId(cellId[region])

            # Increment cellId.
            # Important if we ever expand this test to have more than 1 digit per region.
            cellId[region] += 1

            energy = digitParam[region]["aboveEnergythresh"] * (self.energyThresh + 1)
            time = digitParam[region]["aboveTimethresh"] * (self.timeThresh + 1)
            eclCalDigit.setEnergy(energy)
            eclCalDigit.setTime(time)

            # Add ECLDigit to datastore
            newDigit = eclCalDigits.appendNew()
            newDigit.__assign__(eclCalDigit)

            # Set expected number of out of time calDigits per region
            expectOutOfTime[region] = int(digitParam[region]["aboveEnergythresh"] and digitParam[region]["aboveTimethresh"])
            b2.B2DEBUG(35, region + ": expecting " + str(expectOutOfTime[region]))
            b2.B2DEBUG(39, "region = " + region + ", time = " + str(time) + ", energy = " + str(energy))

        # Increment event counter
        self.eventCounter += 1


class checkNumOutOfTimeDigitsModule(b2.Module):
    """
    module which checks the number of out of time digits in EventLevelClusteringInfo is as expected
    """

    def event(self):
        """
        event function
        """
        eventLevelClusteringInfo = Belle2.PyStoreObj('EventLevelClusteringInfo').obj()

        tc = TestCase('__init__')

        tc.assertEqual(eventLevelClusteringInfo.getNECLCalDigitsOutOfTimeFWD(), expectOutOfTime["FWD"])
        tc.assertEqual(eventLevelClusteringInfo.getNECLCalDigitsOutOfTimeBarrel(), expectOutOfTime["BRL"])
        tc.assertEqual(eventLevelClusteringInfo.getNECLCalDigitsOutOfTimeBWD(), expectOutOfTime["BWD"])
        tc.assertEqual(eventLevelClusteringInfo.getNECLCalDigitsOutOfTime(),
                       expectOutOfTime["FWD"] + expectOutOfTime["BRL"] + expectOutOfTime["BWD"])


main = b2.create_path()

# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

geometry = b2.register_module('Geometry', useDB=False)
geometry.param('components', ['ECL'])
main.add_module(geometry)

# Add ECLCalDigits
addECLCalDigits = main.add_module(addECLCalDigitsModule())
addECLCalDigits.logging.log_level = b2.LogLevel.DEBUG
addECLCalDigits.logging.debug_level = 10

# Set number of events according to number of different combination in addECLCalDigits
eventinfosetter.param({'evtNumList': [len(addECLCalDigits.digitParams)], 'runList': [0]})

ECLDigitCalibrator = b2.register_module('ECLDigitCalibrator')
main.add_module(ECLDigitCalibrator)

# Extract energy and time threshold from ECLDigitCalibrator module
for param in ECLDigitCalibrator.available_params():
    if param.name == "backgroundEnergyCut":
        addECLCalDigits.energyThresh = param.values
    elif param.name == "backgroundTimingCut":
        addECLCalDigits.timeThresh = param.values

main.add_module(checkNumOutOfTimeDigitsModule())

# Process events
b2.process(main)
