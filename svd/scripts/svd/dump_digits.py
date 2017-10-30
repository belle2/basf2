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

    def __init__(self, calmap='', oo_map='testbeam/vxd/data/2017_svd_mapping.xml', filename='dumped_digits.txt'):
        """Initialize the module"""

        super(dump_digits, self).__init__()
        #: Input file object.
        self.outfile = open(filename, 'w')
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)
        # Get a handle on the GeoCache
        self.geoCache = Belle2.VXD.GeoCache.getInstance()
        # Record filenames
        self.oo_map = oo_map
        self.calmap = calmap

    def beginRun(self):
        """ Tasks at the start of a run """
        # Write header to output file
        self.outfile.write(
            'EventNo Layer Ladder Sensor Side StripNo TimeTrigger ' +
            'GoodStrip Noise Width TimeShift' +
            'Sample0 Sample1 Sample2 Sample3 Sample4 Sample5 Charge TimeFit Chi2\n')
        # Initialize stripmap
        self.strip_map = Belle2.SVD.StripCalibrationMap(self.oo_map, self.calmap)

    def event(self):
        """Cycle through RecoDigit/ShaperDigit pairs and dump the corresponding data"""

        evt_info = Belle2.PyStoreObj('EventMetaData')
        event_number = evt_info.getEvent()
        reco_digits = Belle2.PyStoreArray('SVDRecoDigits')

        for reco_digit in reco_digits:

            shaper_digit = reco_digit.getRelatedTo('SVDShaperDigits')

            s = ''

            # Sesnor/side/strip identification
            [layer, ladder, sensor] = self.decode(reco_digit.getRawSensorID())
            s += '{event} {layer} {ladder} {sensor} {side} {strip} '.format(
                event=event_number,
                layer=layer,
                ladder=ladder,
                sensor=sensor,
                side=('u' if reco_digit.isUStrip() else 'v'),
                strip=reco_digit.getCellID()
            )
            sensorID = reco_digit.getSensorID()
            stripNo = reco_digit.getCellID()
            # Trigger bin from SVDModeByte
            triggerBin = ord(shaper_digit.getModeByte().getTriggerBin())
            triggerTime = 0.25 * 31.44 * (-4 + triggerBin + 0.5)
            s += '{trigger:.3f} '.format(trigger=triggerTime)
            # Calibrations
            stripCals = self.strip_map.getStripData(sensorID, reco_digit.isUStrip(), stripNo)
            s += '{mask} {gain:.3f} {noise:.3f} {width} {delay:.3f} '.format(
                mask=('y' if stripCals.m_goodStrip else 'n'),
                gain=stripCals.m_calPeak,
                noise=stripCals.m_noise,
                width=stripCals.m_calWidth,
                delay=stripCals.m_calTimeDelay
            )
            # Mask: True if good strip
            stripMask = stripCals.m_goodStrip
            # Gain: electrons per ADU
            stripGain = stripCals.m_calPeak
            if stripGain == 0:
                stripGain = 1.0e6
            # Noise: in electrons, convert to ADU
            stripNoise = stripCals.m_noise / stripGain
            # Width: correct beta-prime scale
            stripWaveWidth = stripCals.m_calWidth
            # TimeDelay: beta-prime shift,
            stripTimeDelay = stripCals.m_calTimeDelay

            # Digit information
            samples = shaper_digit.getSamples()
            for iSample in range(6):
                s += '{0} '.format(samples[iSample])

            # Data from time fit
            s += '{amplitude:.3f} {time:.3f} {chi2:.3f}'.format(
                amplitude=reco_digit.getCharge(),
                time=reco_digit.getTime(),
                chi2=reco_digit.getChi2Ndf()
            )

            s += '\n'
            self.outfile.write(s)

    def terminate(self):
        """ Close the output file."""

        self.outfile.close()

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
