#!/usr/bin/env python3

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


class dump_clusters(b2.Module):

    """A simple module to timing of SVD clusters.
    Intended for use with the RandomizeEventTimes feature of SVDDigitizer,
    which stores (randomized) event times in EventMetaData.
    For background studies, it distinguishes signal and background clusters..
    """

    def __init__(self, filename='dump_clusters.txt', collection='SVDClusters'):
        """Initialize the module"""

        super().__init__()
        #: Input file object.
        self.file = open(filename, 'w')
#: name of the StoreArraw SVDClusters
        self.collection = collection
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write(
            'EventNo Layer Ladder Sensor Side Position Bg Time_event' +
            'Time_cluster Charge_cluster Size_cluster Chi2_cluster\n'
        )

    def event(self):
        """Save information on all clusters together with event time data. """

        eventData = Belle2.PyStoreObj('EventMetaData')
        eventNo = eventData.getEvent()
        svd_evt_info = Belle2.PyStoreObj('SVDEventInfo')
        mode_byte = svd_evt_info.getModeByte()
        clusters = Belle2.PyStoreArray(self.collection)

        for cluster in clusters:

            bg_label = 's'
            reco_digit = cluster.getRelatedTo('SVDRecoDigits')
            triggerTime = 0.0
            if reco_digit:
                # Trigger bin from SVDModeByte
                triggerBin = ord(mode_byte.getTriggerBin())
                triggerTime = 0.25 * 31.44 * (-4 + triggerBin + 0.5)

            sensorID = cluster.getRawSensorID()

            uSide = cluster.isUCluster()

            # Sesnor identification
            [layer, ladder, sensor] = self.decode(sensorID)
            side_str = 'u' if uSide else 'v'

            s = f'{eventNo} {layer} {ladder} {sensor} {side_str} {cluster.getPosition()} {bg_label} {triggerTime}'
            # Cluster information
            s += f' {cluster.getClsTime()} {cluster.getCharge()} {cluster.getSize()} {cluster.getChi2()}\n'
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
