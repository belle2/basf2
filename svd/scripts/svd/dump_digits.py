#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

# Some ROOT tools
from ROOT import Belle2


class dump_digits(b2.Module):

    """A simple module to check SVD digit fits."""

    def __init__(self, filename='dumped_digits.txt'):
        """Initialize the module"""

        super(dump_digits, self).__init__()
        #: Input file object.
        self.outfile = open(filename, 'w')
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)
        # Get a handle on the GeoCache
#: geoCache instance
        self.geoCache = Belle2.VXD.GeoCache.getInstance()
        # Record filenames
#: noise calibrations database object
        self.noise_cal = Belle2.SVDNoiseCalibrations()
#: pulse shape (gain, peak time, ...) database object
        self.pulse_cal = Belle2.SVDPulseShapeCalibrations()

    def beginRun(self):
        """ Tasks at the start of a run """
        # Write header to output file
        self.outfile.write(
            'EventNo Layer Ladder Sensor Side StripNo TimeTrigger ' +
            'GoodStrip Gain Noise Width TimeShift ' +
            'Sample0 Sample1 Sample2 Sample3 Sample4 Sample5 Charge TimeFit Chi2\n')

    def event(self):
        """Cycle through RecoDigit/ShaperDigit pairs and dump the corresponding data"""

        evt_info = Belle2.PyStoreObj('EventMetaData')
        svd_evt_info = Belle2.PyStoreObj('SVDEventInfo')
        event_number = evt_info.getEvent()
        mode_byte = svd_evt_info.getModeByte()
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
            triggerBin = ord(mode_byte.getTriggerBin())
            triggerTime = 0.25 * 31.44 * (-4 + triggerBin + 0.5)
            s += '{trigger:.3f} '.format(trigger=triggerTime)
            # Calibrations
            stripNoise = self.noise_cal.getNoise(sensorID, reco_digit.isUStrip(), stripNo)
            stripGain = 22500 / self.pulse_cal.getADCFromCharge(sensorID, reco_digit.isUStrip(), stripNo, 22500)

            stripT0 = self.pulse_cal.getPeakTime(sensorID, reco_digit.isUStrip(), stripNo)
            stripWidth = self.pulse_cal.getWidth(sensorID, reco_digit.isUStrip(), stripNo)
            s += '{mask} {gain:.3f} {noise:.3f} {width} {delay:.3f} '.format(
                mask='y',
                gain=stripGain,
                noise=stripNoise,
                width=stripWidth,
                delay=stripT0
            )

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
