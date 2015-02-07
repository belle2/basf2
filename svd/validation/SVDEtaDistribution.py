#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>SVDEtaDistributionData.root</output>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>
....This module is part of the SVD validation suite. It creates 
....separate eta distributions for U and V directions, where 
....eta = (cluster_pos % pitch) / pitch
....and saves them to a ROOT file.
  </description>
</header>
"""

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine('struct EtaData {\
    float pitch;\
    float eta;\
};')

from ROOT import EtaData


class SVDEtaDistribution(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDEtaDistribution, self).__init__()
        ## Output ROOT file.
        self.file = ROOT.TFile('SVDEtaDistributionData.root', 'recreate')
        ## TTree for output data
        self.Utree = ROOT.TTree('Utree', 'Eta data for U direction')
        self.Vtree = ROOT.TTree('Vtree', 'Eta data for V direction')
        ## Instance of EtaData class
        self.data = EtaData()
        # Declare tree branches
        for key in EtaData.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.Utree.Branch(key, AddressOf(self.data, key), key
                                  + formstring)
                self.Vtree.Branch(key, AddressOf(self.data, key), key
                                  + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find clusters and save needed information."""

        clusters = Belle2.PyStoreArray('SVDClusters')
        for cluster in clusters:
            clusterTruehits = cluster.getRelationsTo('SVDTrueHits')

            # We want only clusters with exactly one associated TrueHit
            if len(clusterTruehits) != 1:
                continue

            sensorInfo = Belle2.VXD.GeoCache.get(cluster.getSensorID())

            if cluster.isUCluster():
                pitch = sensorInfo.getUPitch(cluster.getPosition())
            else:
                pitch = sensorInfo.getVPitch(cluster.getPosition())

            eta = cluster.getPosition() % pitch / pitch
            self.data.pitch = pitch
            self.data.eta = eta

            self.file.cd()
            if cluster.isUCluster():
                self.Utree.Fill()
            else:
                self.Vtree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()


