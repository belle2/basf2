#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
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
import SensorType

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
        self.Layer3Utree = ROOT.TTree('Layer3Utree',
                                      'Eta data for Layer3 U direction')
        self.Layer3Vtree = ROOT.TTree('Layer3Vtree',
                                      'Eta data for Layer3 V direction')
        self.SlantedUtree = ROOT.TTree('SlantedUtree',
                                       'Eta data for Slanted U direction')
        self.SlantedVtree = ROOT.TTree('SlantedVtree',
                                       'Eta data for Slanted V direction')
        self.OtherUtree = ROOT.TTree('OtherUtree',
                                     'Eta data for Other U direction')
        self.OtherVtree = ROOT.TTree('OtherVtree',
                                     'Eta data for Other V direction')
        ## Instance of EtaData class
        self.data = EtaData()
        # Declare tree branches
        for key in EtaData.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.Layer3Utree.Branch(key, AddressOf(self.data, key), key
                        + formstring)
                self.Layer3Vtree.Branch(key, AddressOf(self.data, key), key
                        + formstring)
                self.SlantedUtree.Branch(key, AddressOf(self.data, key), key
                        + formstring)
                self.SlantedVtree.Branch(key, AddressOf(self.data, key), key
                        + formstring)
                self.OtherUtree.Branch(key, AddressOf(self.data, key), key
                                       + formstring)
                self.OtherVtree.Branch(key, AddressOf(self.data, key), key
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
            sensorID = cluster.getSensorID()
            sensorType = SensorType.getSensorType(sensorID)

            if cluster.isUCluster():
                pitch = sensorInfo.getUPitch(cluster.getPosition())
            else:
                pitch = sensorInfo.getVPitch(cluster.getPosition())

            eta = cluster.getPosition() % pitch / pitch
            self.data.pitch = pitch
            self.data.eta = eta

            self.file.cd()
            if cluster.isUCluster():
                if 'Layer3' in sensorType:
                    self.Layer3Utree.Fill()
                elif 'Slanted' in sensorType:
                    self.SlantedUtree.Fill()
                else:
                    self.OtherUtree.Fill()
            else:
                if 'Layer3' in sensorType:
                    self.Layer3Vtree.Fill()
                elif 'Slanted' in sensorType:
                    self.SlantedVtree.Fill()
                else:
                    self.OtherVtree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()


