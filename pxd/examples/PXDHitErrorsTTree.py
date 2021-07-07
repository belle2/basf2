#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import EventData
import math
import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2

from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine('struct EventData {\
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
};'
                  )


class PXDHitErrorsTTree(b2.Module):

    """
    A simple module to check the reconstruction of PXDTrueHits.
    This module writes its output to a ROOT tree.
    """

    def __init__(self):
        """Initialize the module"""

        super(PXDHitErrorsTTree, self).__init__()
        #: Output ROOT file.
        self.file = ROOT.TFile('PXDHitErrorOutput.root', 'recreate')
        #: TTree for output data
        self.tree = ROOT.TTree('tree', 'Event data of PXD simulation')
        #: Instance of EventData class
        self.data = EventData()
        # Declare tree branches
        for key in EventData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(
                    key,
                    AddressOf(
                        self.data,
                        key),
                    key +
                    formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find clusters with a truehit and print some stats."""

        clusters = Belle2.PyStoreArray('PXDClusters')

        # Start with clusters and use the relation to get the corresponding
        # digits and truehits.
        for cluster in clusters:
            cluster_truehits = cluster.getRelationsTo('PXDTrueHits')

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
                self.data.theta_u = math.atan2(
                    truehit.getExitU() - truehit.getEntryU(), thickness)
                self.data.theta_v = math.atan2(
                    truehit.getExitV() - truehit.getEntryV(), thickness)
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
                self.data.cluster_uPull = (
                    cluster.getU() - truehit.getU()) / cluster.getUSigma()
                self.data.cluster_vPull = (
                    cluster.getV() - truehit.getV()) / cluster.getVSigma()
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()
