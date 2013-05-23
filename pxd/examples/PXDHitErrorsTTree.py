#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2

from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine(
'struct EventData {\
    int vxd_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int truehit_index;\
    int cluster_index;\
    float truehit_u;\
    float truehit_v;\
    float truehit_time;\
    float truehit_charge;\
    float theta_u;\
    float theta_v;\
    float cluster_u;\
    float cluster_v;\
    float cluster_uError;\
    float cluster_vError;\
    float cluster_rho;\
    float cluster_charge;\
    float cluster_seed;\
    float cluster_size;\
    float cluster_uSize;\
    float cluster_vSize;\
    float cluster_uPull;\
    float cluster_vPull;\
};')

from ROOT import EventData


class PXDHitErrorsTTree(Module):

    """
    A simple module to check the reconstruction of PXDTrueHits.
    This module writes its output to a ROOT tree.
    """

    def __init__(self):
        """Initialize the module"""

        super(PXDHitErrorsTTree, self).__init__()
        ## Output ROOT file.
        self.file = ROOT.TFile('PXDHitErrorOutput.root', 'recreate')
        ## TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of PXD simulation')
        ## Instance of EventData class
        self.data = EventData()
        # Declare tree branches
        for key in EventData.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key),
                                 key + formstring)

        ## Factors for decoding VXDId's
        self.vxdid_factors = (8192, 256, 32)

    def beginRun(self):
        """ Does nothing """

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
                # Sesnor identification
                self.data.vxd_id = truehit.getRawSensorID()
                [self.data.layer, self.data.ladder, self.data.sensor] = \
                    self.decode(self.data.vxd_id)
                self.data.truehit_index = truehit_index
                self.data.cluster_index = cluster_index

                # TrueHit information
                self.data.truehit_u = truehit.getU()
                self.data.truehit_v = truehit.getV()
                self.data.truehit_time = truehit.getGlobalTime()
                self.data.truehit_charge = truehit.getEnergyDep()
                self.data.theta_u = \
                    math.atan2(truehit.getExitU() - truehit.getEntryU(), \
                    0.0075)
                self.data.theta_v = \
                    math.atan2(truehit.getExitV() - truehit.getEntryV(), \
                    0.0075)
                # Cluster information
                self.data.cluster_u = cluster.getU()
                self.data.cluster_v = cluster.getV()
                self.data.cluster_uError = cluster.getUSigma()
                self.data.cluster_vError = cluster.getVSigma()
                self.data.cluster_rho = cluster.getRho()
                self.data.cluster_charge = cluster.getCharge()
                self.data.cluster_seed = cluster.getSeedCharge()
                self.data.cluster_size = cluster.getSize()
                self.data.cluster_uSize = cluster.getUSize()
                self.data.cluster_vSize = cluster.getVSize()
                self.data.cluster_uPull = (cluster.getU() - truehit.getU())\
                            / cluster.getUSigma()
                self.data.cluster_vPull = (cluster.getV() - truehit.getV())\
                            / cluster.getVSigma()
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()

    def decode(self, vxdid):
        """ Utility to decode sensor IDs """

        result = []
        for f in self.vxdid_factors:
            result.append(vxdid / f)
            vxdid = vxdid % f

        return result
