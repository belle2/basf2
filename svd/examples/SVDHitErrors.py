#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2


class SVDHitErrors(Module):

    """A simple module to check the reconstruction of SVDTrueHits."""

    def __init__(self):
        """Initialize the module"""

        super(SVDHitErrors, self).__init__()
        self.setName('SVDHitErrors')
        self.file = open('SVDHitErrorOutput.txt', 'w')
        self.vxdid_factors = (8192, 256, 32)  # For decoding VxdID's

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('LEGEND TO COLUMNS: \n')
        self.file.write('SensorID Layer Ladder Sensor Truehit_index Cluster_index \n'
                        )
        self.file.write('TrueHit: u[cm], v[cm], charge[GeV], theta_u, theta_v \n'
                        )
        self.file.write('Cluster: isU[0/1], uv[cm], charge[e-], seed charge[e-], size \n'
                        )
        self.file.write('Digits: n_digits {u[cm] charge[e-]} \n')
        self.file.write('\n')

    def event(self):
        """Find clusters with a truehit and print some stats."""

        truehits = Belle2.PyStoreArray('SVDTrueHits')
        nTruehits = truehits.getEntries()
        clusters = Belle2.PyStoreArray('SVDClusters')
        nClusters = clusters.getEntries()
        digits = Belle2.PyStoreArray('SVDDigits')
        nDigits = digits.getEntries()
        relClustersToTrueHits = \
            Belle2.PyRelationArray('SVDClustersToSVDTrueHits')
        nClusterRelations = relClustersToTrueHits.getEntries()
        relClustersToDigits = Belle2.PyRelationArray('SVDClustersToSVDDigits')
        nDigitRelations = relClustersToDigits.getEntries()

        # Start with the clusters and use the relation to get the corresponding digits and truehits.
        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            cluster_truehits = \
                relClustersToTrueHits.getToIndices(cluster_index)

            # Here we deliberately ask only for clsuters with exactly one TrueHit.
            # We don't want combined clusters!
            if len(cluster_truehits) != 1:
                continue

            for truehit_index in cluster_truehits:
                truehit = truehits[truehit_index]
                # Now let's store some data
                s = ''
                # Sesnor identification
                sensorID = truehit.getRawSensorID()
                [layer, ladder, sensor] = self.decode(sensorID)
                s_id = \
                    '{sID} {layer} {ladder} {sensor} {indexTH:4d} {indexCL:4d} '.format(
                    sID=sensorID,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    indexTH=truehit_index,
                    indexCL=cluster_index,
                    )
                s += s_id
                # TrueHit information
                thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                    0.0075)
                thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                    0.0075)
                s_th = \
                    '{uTH:10.5f} {vTH:10.5f} {eTH:10.7f} {thetaU:6.3f} {thetaV:6.3f} '.format(uTH=truehit.getU(),
                        vTH=truehit.getV(), eTH=truehit.getEnergyDep(),
                        thetaU=thetaU, thetaV=thetaV)
                s += s_th
                # Cluster information
                s_cl = \
                    '{isU} {uvCL:10.5f} {eCL:10.1f} {eSeed:10.1f} {size:5d} '.format(isU=cluster.isUCluster(),
                        uvCL=cluster.getPosition(), eCL=cluster.getCharge(),
                        eSeed=cluster.getSeedCharge(), size=cluster.getSize())
                s += s_cl
                # NO DIGITS by now.
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


