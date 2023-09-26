#!/usr/bin/env python3

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
import ROOT
from ROOT import Belle2


class PXDHitErrors(b2.Module):

    """A simple module to check the reconstruction of PXDTrueHits."""

    def __init__(self):
        """Initialize the module"""

        super().__init__()
        #: Input file object.
        self.file = open('PXDHitErrorOutput.txt', 'w')
        #: Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)
        #: File to save histograms
        self.fHisto = ROOT.TFile('PXDPulls.root', 'RECREATE')
        #: Histogram for u pulls
        self.h_pull_u = ROOT.TH1F('h_pull_u', 'Pulls in u', 150, -10, 5)
        #: Histogram for v pulls
        self.h_pull_v = ROOT.TH1F('h_pull_v', 'Pulls in v', 100, -5, 5)

    def beginRun(self):
        """ Write legend for file columns """

        self.file.write('vxd.id layer ladder sensor truehit.index cluster.index ')
        self.file.write('truehit.u truehit.v truehit.time truehit.charge theta.u theta.v ')
        self.file.write('u v u.error v.error rho charge seed size u.size v.size u.pull v.pull\n')

    def event(self):
        """Find clusters with a truehit and print some stats."""

        truehits = Belle2.PyStoreArray('PXDTrueHits')
        # nTruehits = truehits.getEntries()
        clusters = Belle2.PyStoreArray('PXDClusters')
        nClusters = clusters.getEntries()
        # digits = Belle2.PyStoreArray('PXDDigits')
        # nDigits = digits.getEntries()
        relClustersToTrueHits = \
            Belle2.PyRelationArray('PXDClustersToPXDTrueHits')
        # nClusterRelations = relClustersToTrueHits.getEntries()
        # relClustersToDigits = Belle2.PyRelationArray('PXDClustersToPXDDigits')
        # nDigitRelations = relClustersToDigits.getEntries()

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

                s_id = f'{sensorID} {layer} {ladder} {sensor} {truehit_index:4d} {cluster_index:4d} '
                s += s_id
                # TrueHit information
                thetaU = math.atan2(truehit.getExitU() - truehit.getEntryU(),
                                    0.0075)
                thetaV = math.atan2(truehit.getExitV() - truehit.getEntryV(),
                                    0.0075)
                s_th = f'{truehit.getU():10.5f} {truehit.getV():10.5f} {truehit.getGlobalTime():10.2f} ' + \
                    f'{truehit.getEnergyDep():10.7f} {thetaU:6.3f} {thetaV:6.3f} '
                s += s_th
                # Cluster information
                cluster_pull_u = 0
                cluster_pull_v = 0
                try:
                    cluster_pull_u = (cluster.getU() - truehit.getU()) \
                        / cluster.getUSigma()
                    cluster_pull_v = (cluster.getV() - truehit.getV()) \
                        / cluster.getVSigma()
                except ZeroDivisionError:
                    if cluster.getUSigma() < 1.0e-8:
                        b2.B2ERROR(f'Zero error in u, clsize {cluster.getUSize()}.')
                    else:
                        b2.B2ERROR(f'Zero error in v, clsize {cluster.getVSize()}.')

                s_cl = \
                    f'{cluster.getU():10.5f} {cluster.getV():10.5f} {cluster.getUSigma():10.5f} {cluster.getVSigma():10.5f} ' + \
                    f'{cluster.getRho():10.4f} {cluster.getCharge():9.1f} {cluster.getSeedCharge():9.1f} ' + \
                    f'{cluster.getSize():5d} {cluster.getUSize():5d} {cluster.getVSize():5d} ' + \
                    f'{cluster_pull_u:10.3f} {cluster_pull_v:10.3f}'
                s += s_cl
                # NO DIGITS by now.
                s += '\n'
                self.file.write(s)
                if cluster.getUSize() == 2:
                    self.h_pull_u.Fill(cluster_pull_u)
                if cluster.getVSize() == 2:
                    self.h_pull_v.Fill(cluster_pull_v)

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
