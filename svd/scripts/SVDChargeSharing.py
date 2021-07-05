#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import math
import basf2 as b2

# Some ROOT tools
from ROOT import Belle2


class SVDChargeSharing(b2.Module):

    """A module to gather data on charge sharing in the SVD."""

    def __init__(self):
        """Initialize the module"""

        super(SVDChargeSharing, self).__init__()
        #: Input file object.
        self.file = open('SVDChargeSharingData.txt', 'w')

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('sensor_type layer ladder sensor truehit_index cluster_index ')
        self.file.write('truehit_u truehit_eta_u truehit_v truehit_eta_v charge_keV ')
        self.file.write('theta_u theta_v cluster_isU cluster_pos cluster_eta ')
        self.file.write('cluster_charge_ADU seed_charge_ADU cluster_size\n')

    def event(self):
        """Find clusters with a truehit and print some stats."""

        # truehits = Belle2.PyStoreArray('SVDTrueHits')
        # nTruehits = truehits.getEntries()
        clusters = Belle2.PyStoreArray('SVDClusters')
        nClusters = clusters.getEntries()
        print("nClusters = "+str(nClusters))
        geoCache = Belle2.VXD.GeoCache.getInstance()
        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            # Get SensorInfo for the sensor
            truehit = cluster.getRelated('SVDTrueHits')
            # Here we ask only for clusters with exactly one TrueHit.
            if (not truehit):
                continue

            # Now let's store some data
            s = ''
            # Sesnor identification
            sensorID = Belle2.VxdID(truehit.getRawSensorID())
            info = geoCache.get(sensorID)
            layer = sensorID.getLayerNumber()
            ladder = sensorID.getLadderNumber()
            sensor = sensorID.getSensorNumber()
            # No wedge sensor data
            if sensor == 1 and layer != 3:
                continue

            sensorType = 'barrel' if layer > 3 else 'layer3'
            s += '{sID} {layer} {ladder} {sensor} {indexT:4d} {indexC:4d} '.format(
                sID=sensorType,
                layer=layer,
                ladder=ladder,
                sensor=sensor,
                indexT=truehit.getArrayIndex(),
                indexC=cluster_index
            )
            # TrueHit information
            truehit_u = truehit.getU()
            truehit_v = truehit.getV()
            pitch_u = info.getUPitch()
            truehit_eta_u = (truehit_u / pitch_u) % 1
            pitch_v = info.getVPitch()
            truehit_eta_v = (truehit_v / pitch_v) % 1
            thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                info.getThickness())
            thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                info.getThickness())
            s += '{uTH:10.5f} {uTHeta:10.5f} {vTH:10.5f} {vTHeta:10.5f} {eTH:10.7f} '.format(
                uTH=truehit_u, uTHeta=truehit_eta_u,
                vTH=truehit_v, vTHeta=truehit_eta_v,
                eTH=1.0e6 * truehit.getEnergyDep()
            ) + '{thetaU:6.3f} {thetaV:6.3f} '.format(
                thetaU=thetaU, thetaV=thetaV
            )
            # Cluster information
            cluster_pitch = (pitch_u if cluster.isUCluster() else pitch_v)
            cluster_eta = (cluster.getPosition() / cluster_pitch) % 1
            s += '{isU} {uvC:10.5f} {uvCeta:10.5f} {eC:10.1f} '.format(
                isU=cluster.isUCluster(),
                uvC=cluster.getPosition(),
                uvCeta=cluster_eta,
                eC=cluster.getCharge()
            ) + '{eSeed:10.1f} {size:5d}'.format(
                eSeed=cluster.getSeedCharge(),
                size=cluster.getSize())

            s += '\n'
            self.file.write(s)

    def terminate(self):
        """ Close the output file."""

        self.file.close()
