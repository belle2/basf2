#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2


class PXDHitErrors(Module):

    """A simple module to check the reconstruction of PXDTrueHits."""

    def __init__(self):
        """Initialize the module"""

        super(PXDHitErrors, self).__init__()
        self.setName('PXDHitErrors')
        # # Name of output file
        self.file = open('PXDHitErrorOutput.txt', 'w')
        # # Factors to decode VXDids
        self.vxdid_factors = (8192, 256, 32)  # For decoding VxdID's

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('LEGEND TO COLUMNS: \n')
        self.file.write('SensorID Layer Ladder Sensor Truehit_index ')
        self.file.write('Cluster_index \n')
        self.file.write('TrueHit: u[cm], v[cm], charge[GeV], ')
        self.file.write(' theta_u, theta_v \n')
        self.file.write('Cluster: u[cm], v[cm], charge[e-], seed charge[e-], ')
        self.file.write('size, size_u, size_v \n')
        self.file.write('Digits: n_digits {u[cm] v[cm] charge[e-]} \n')
        self.file.write('\n')

    def event(self):
        """Find clusters with a truehit and print some stats."""

        truehits = Belle2.PyStoreArray('PXDTrueHits')
        nTruehits = truehits.getEntries()
        clusters = Belle2.PyStoreArray('PXDClusters')
        nClusters = clusters.getEntries()
        digits = Belle2.PyStoreArray('PXDDigits')
        nDigits = digits.getEntries()
        relClustersToTrueHits = \
            Belle2.PyRelationArray('PXDClustersToPXDTrueHits')
        nClusterRelations = relClustersToTrueHits.getEntries()
        relClustersToDigits = Belle2.PyRelationArray('PXDClustersToPXDDigits')
        nDigitRelations = relClustersToDigits.getEntries()

        # Use the relation to get the corresponding digits and truehits.
        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            cluster_truehits = \
                relClustersToTrueHits.getToIndices(cluster_index)

            for truehit_index in cluster_truehits:
                truehit = truehits[truehit_index]
                # Now let's store some data
                s = ''
                # Sesnor identification
                sensorID = truehit.getRawSensorID()
                [layer, ladder, sensor] = self.decode(sensorID)
                s_id = \
                    '{sID} {layer} {ladder} {sensor} {iTH:4d} {iCL:4d} '.format(
                    sID=sensorID,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    iTH=truehit_index,
                    iCL=cluster_index,
                    )
                s += s_id
                # TrueHit information
                thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                    0.0075)
                thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                    0.0075)
                s_th = \
                    '{uTH:10.5f} {vTH:10.5f} {eTH:10.7f} '.format(uTH=truehit.getU(),
                        vTH=truehit.getV(), eTH=truehit.getEnergyDep(),
                        thetaU=thetaU, thetaV=thetaV)
                s += s_th
                # Cluster information
                s_cl = \
                    '{uCL:10.5f} {vCL:10.5f} {eCL:10.1f} {eSeed:10.1f} '.format(uCL=cluster.getU(),
                        vCL=cluster.getV(), eCL=cluster.getCharge(),
                        eSeed=cluster.getSeedCharge()) \
                    + '{size:5d} {sizeU:5d} {sizeV:5d} '.format(size=cluster.getSize(),
                        sizeU=cluster.getUSize(), sizeV=cluster.getVSize())
                s += s_cl
                # We can add some digits, too.
                digit_indices = relClustersToDigits.getToIndices(cluster_index)
                s_dig0 = '{nDigits:4d} '.format(nDigits=len(digit_indices))
                s += s_dig0
                for digit_index in digit_indices:
                    digit = digits[digit_index]
                    s_dig = \
                        '{u:10.5f} {v:10.5f} {e:10.1f} '.format(u=digit.getUCellPosition(),
                            v=digit.getVCellPosition(), e=digit.getCharge())
                    s += s_dig

                s += '\n'
                self.file.write(s)

    def terminate(self):
        """ Close the output file."""

        self.file.close()

    def decode(self, vxdid):
        """ Utility to decode sensor IDs """

        result = []
        for f in self.vxdid_factors:
            result.append(vxdid / f)
            vxdid = vxdid % f

        return result


