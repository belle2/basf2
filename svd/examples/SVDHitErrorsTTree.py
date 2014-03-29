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
gROOT.ProcessLine('struct SVDEventData {\
    int vxd_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int u_cluster;\
    int truehit_index;\
    int cluster_index;\
    float truehit_u;\
    float truehit_v;\
    float truehit_time;\
    float truehit_charge;\
    float theta_u;\
    float theta_v;\
    float cluster_x;\
    float cluster_xError;\
    float cluster_charge;\
    float cluster_seed;\
    float cluster_size;\
    float cluster_xPull;\
};'
                  )

from ROOT import SVDEventData


class SVDHitErrorsTTree(Module):

    """
    A simple module to check the reconstruction of SVDTrueHits.
    This module writes its output to a ROOT tree.
    """

    def __init__(self):
        """Initialize the module"""

        super(SVDHitErrorsTTree, self).__init__()
        ## Output ROOT file.
        self.file = ROOT.TFile('SVDHitErrorOutput.root', 'recreate')
        ## TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD simulation')
        ## Instance of SVDEventData class
        self.data = SVDEventData()
        # Declare tree branches
        for key in SVDEventData.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key
                                 + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """ Find clusters with a truehit and print some stats."""

        clusters = Belle2.PyStoreArray('SVDClusters')

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster in clusters:
            cluster_truehits = cluster.getRelationsTo('SVDTrueHits')

            # Here we ask only for clusters with exactly one TrueHit.
            if len(cluster_truehits) != 1:
                continue

            for truehit in cluster_truehits:
                # Now let's store some data
                # Sesnor identification
                vxd_id = truehit.getSensorID()
                self.data.vxd_id = vxd_id.getID()
                self.data.layer = vxd_id.getLayerNumber()
                self.data.ladder = vxd_id.getLadderNumber()
                self.data.sensor = vxd_id.getSensorNumber()
                if cluster.isUCluster():
                    self.data.u_cluster = 1
                else:
                    self.data.u_cluster = 0
                self.data.truehit_index = truehit.getArrayIndex()
                self.data.cluster_index = cluster.getArrayIndex()

                # Get sensor geometry information
                sensor_info = Belle2.VXD.GeoCache.get(vxd_id)
                thickness = sensor_info.getThickness()

                # TrueHit information
                self.data.truehit_u = truehit.getU()
                self.data.truehit_v = truehit.getV()
                self.data.truehit_time = truehit.getGlobalTime()
                self.data.truehit_charge = truehit.getEnergyDep()
                self.data.theta_u = math.atan2(truehit.getExitU()
                        - truehit.getEntryU(), thickness)
                self.data.theta_v = math.atan2(truehit.getExitV()
                        - truehit.getEntryV(), thickness)
                # Cluster information
                self.data.cluster_x = cluster.getPosition()
                self.data.cluster_xError = cluster.getPositionSigma()
                self.data.cluster_charge = cluster.getCharge()
                self.data.cluster_seed = cluster.getSeedCharge()
                self.data.cluster_size = cluster.getSize()
                if cluster.isUCluster():
                    self.data.cluster_xPull = (cluster.getPosition()
                            - truehit.getU()) / cluster.getPositionSigma()
                else:
                    self.data.cluster_xPull = (cluster.getPosition()
                            - truehit.getV()) / cluster.getPositionSigma()
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()


