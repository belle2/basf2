#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2


class dump_digits(Module):

    """A simple module to check the reconstruction of SVD hits."""

    def __init__(self):
        """Initialize the module"""

        super(dump_digits, self).__init__()
        #: Input file object.
        self.file = open('dumped_digits.txt', 'w')
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)
        # Get a handle on the GeoCache
        self.geoCache = Belle2.VXD.GeoCache.getInstance()

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('VxdID Layer Ladder Sensor Side  Sample0 Sample1 Sample2 Sample3 Sample4 lSample5')

    def event(self):
        """Find clusters with a truehit and print some stats."""

        last_sensorID = 0
        thresholdU = 0
        thresholdV = 0

        digits = Belle2.PyStoreArray('SVDShaperDigits')
        nDigits = digits.getEntries()

        for digit in digits:

            sensorID = digit.getRawSensorID()
            # Only get new values if we are on a new sensor
            if sensorID != last_sensorID:
                last_sensorID = sensorID
                sensor_info = self.geoCache.get(digit.getSensorID())
                thresholdU = 3.0 * sensor_info.getElectronicNoiseU() / sensor_info.getAduEquivalentU()
                thresholdV = 3.0 * sensor_info.getElectronicNoiseV() / sensor_info.getAduEquivalentV()

            uSide = digit.isUStrip()
            digit_threshold = thresholdU if uSide else thresholdV

            if not self.three_test(digit, digit_threshold):
                continue

            # Sesnor identification
            [layer, ladder, sensor] = self.decode(sensorID)
            side_str = 'u' if uSide else 'v'

            s = ''

            s_id = '{sID} {layer} {ladder} {sensor} {side} '.format(
                sID=sensorID,
                layer=layer,
                ladder=ladder,
                sensor=sensor,
                side=side_str
            )
            s += s_id
            # Digit information
            samples = digit.getSamples()
            for iSample in range(6):
                s += '{0} '.format(samples[iSample])
            s += '\n'
            self.file.write(s)

    def terminate(self):
        """ Close the output file."""

        self.file.close()

    def decode(self, vxdid):
        """ Utility to decode sensor IDs """

        result = []
        for f in self.vxdid_factors:
            result.append(vxdid // f)
            vxdid = vxdid % f

        return result

    def three_test(self, digit, threshold):
        ''' 3-samples digit test '''

        counter = 0
        for sample in digit.getSamples():
            if counter == 3:
                continue
            elif sample >= threshold:
                counter += 1
            else:
                counter = 0
        return (counter >= 3)
