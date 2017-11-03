#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2


class dump_clusters(Module):

    """A simple module to timing of SVD clusters.
    Intended for use with the RandomizeEventTimes feature of SVDDigitizer,
    which stores (randomized) event times in EventMetaData.
    """

    def __init__(self, filename='dump_clusters.txt', collection='SVDClusters'):
        """Initialize the module"""

        super(dump_clusters, self).__init__()
        #: Input file object.
        self.file = open(filename, 'w')
        self.collection = collection
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)
        # Get a handle on the GeoCache
        self.geoCache = Belle2.VXD.GeoCache.getInstance()

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('VxdID Layer Ladder Sensor Side  NStrips Time_event Time_cluster Charge_cluster Size_cluster\n')

    def event(self):
        """Save information on all clusters together with event time data. """

        last_sensorID = 0
        thresholdU = 0
        thresholdV = 0

        eventData = Belle2.PyStoreObj('EventMetaData')
        eventTime = eventData.getTime() - 1000  # there is standard offset
        clusters = Belle2.PyStoreArray(self.collection)
        nClusters = clusters.getEntries()

        for cluster in clusters:

            sensorID = cluster.getRawSensorID()
            # Only get new values if we are on a new sensor
            if sensorID != last_sensorID:
                last_sensorID = sensorID
                sensor_info = self.geoCache.get(cluster.getSensorID())

            uSide = cluster.isUCluster()
            nStrips = sensor_info.getUCells() if uSide else sensor_info.getVCells()

            # Sesnor identification
            [layer, ladder, sensor] = self.decode(sensorID)
            side_str = 'u' if uSide else 'v'

            s = '{sID} {layer} {ladder} {sensor} {side} {nstrips} {etime}'.format(
                sID=sensorID,
                layer=layer,
                ladder=ladder,
                sensor=sensor,
                side=side_str,
                nstrips=nStrips,
                etime=eventTime
            )
            # Cluster information
            s += ' {clstime} {clscharge} {clssize}\n'.format(
                clstime=cluster.getClsTime(),
                clscharge=cluster.getCharge(),
                clssize=cluster.getSize()
            )
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
