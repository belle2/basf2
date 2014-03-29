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
        ## Input file object.
        self.file = open('SVDHitErrorOutput.txt', 'w')
        ## Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)
        ## File to save histograms
        self.fHisto = ROOT.TFile('SVDPulls.root', 'RECREATE')
        ## Histogram for u pulls
        self.h_pull_u = ROOT.TH1F('h_pull_u', 'Pulls in u', 150, -10, 5)
        ## Histogram for v pulls
        self.h_pull_v = ROOT.TH1F('h_pull_v', 'Pulls in v', 100, -5, 5)

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('SensorID Layer Ladder Sensor Truehit_index '
                        + 'Cluster_index ')
        self.file.write('truehit_u truehit_v truehit_time_ns charge_GeV '
                        + 'theta_u theta_v ')
        self.file.write('cluster_isU cluster_pos cluster_error cluster_time '
                        + 'cluster_charge_e seed_charge_e cluster_size\n')

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

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster_index in range(nClusters):
            cluster = clusters[cluster_index]
            cluster_truehits = \
                relClustersToTrueHits.getToIndices(cluster_index)

            # Here we ask only for clusters with exactly one TrueHit.
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
                if sensor == 1 and layer != 3:
                    continue

                s_id = \
                    '{sID} {layer} {ladder} {sensor} {indexT:4d} {indexC:4d} '.format(
                    sID=sensorID,
                    layer=layer,
                    ladder=ladder,
                    sensor=sensor,
                    indexT=truehit_index,
                    indexC=cluster_index,
                    )
                s += s_id
                # TrueHit information
                thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                    0.0075)
                thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                    0.0075)
                s_th = \
                    '{uTH:10.5f} {vTH:10.5f} {tTH:10.2f} {eTH:10.7f} '.format(uTH=truehit.getU(),
                        vTH=truehit.getV(), tTH=truehit.getGlobalTime(),
                        eTH=truehit.getEnergyDep()) \
                    + '{thetaU:6.3f} {thetaV:6.3f} '.format(thetaU=thetaU,
                        thetaV=thetaV)
                s += s_th
                # Cluster information
                cluster_pull = 0.0
                if cluster.isUCluster():
                    cluster_pull = (cluster.getPosition() - truehit.getU()) \
                        / cluster.getPositionSigma()
                else:
                    cluster_pull = (cluster.getPosition() - truehit.getV()) \
                        / cluster.getPositionSigma()
                s_cl = \
                    '{isU} {uvC:10.5f} {uvCErr:10.5f} {tC:10.2f} {eC:10.1f} '.format(isU=cluster.isUCluster(),
                        uvC=cluster.getPosition(),
                        uvCErr=cluster.getPositionSigma(),
                        tC=cluster.getClsTime() - 50, eC=cluster.getCharge()) \
                    + '{eSeed:10.1f} {size:5d} {pull:10.3f}'.format(eSeed=cluster.getSeedCharge(),
                        size=cluster.getSize(), pull=cluster_pull)
                s += s_cl
                # NO DIGITS by now.
                s += '\n'
                self.file.write(s)
                if cluster.getSize() == 2:
                    if cluster.isUCluster():
                        self.h_pull_u.Fill((cluster.getPosition()
                                - truehit.getU()) / cluster.getPositionSigma())
                    else:
                        self.h_pull_v.Fill((cluster.getPosition()
                                - truehit.getV()) / cluster.getPositionSigma())

    def terminate(self):
        """ Close the output file."""

        self.file.close()
        # Save histograms to file
        self.fHisto.cd()
        self.h_pull_u.Write()
        self.h_pull_v.Write()
        self.fHisto.Flush()
        self.fHisto.Close()

    def decode(self, vxdid):
        """ Utility to decode sensor IDs """

        result = []
        for f in self.vxdid_factors:
            result.append(vxdid / f)
            vxdid = vxdid % f

        return result


