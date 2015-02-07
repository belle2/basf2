#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>D. Cervenkov, cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>
    This module is part of the SVD validation suite. It creates 
    cluster position pull distributions for the 3 types of sensors:
    Layer3, Slanted and Other and for the two directions (U, V)
    and saves them to a ROOT file.
  </description>
</header>
"""

from basf2 import *
# Package that gives you SVD sensor type for given sensorID
import SensorType

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct ClusterData {\
		float clusterPosition;\
		float clusterPositionSigma;\
		float truehitPosition;\
		float pull;\
		};'
                  )

from ROOT import ClusterData


class SVDClusterPull(Module):

    def __init__(self):
        super(SVDClusterPull, self).__init__()
        ## Output ROOT file
        self.file = ROOT.TFile('SVDClusterPullData.root', 'recreate')
        ## TTrees for output data
        self.Layer3Utree = ROOT.TTree('Layer3Utree',
                                      'Cluster data for Layer3 U direction')
        self.Layer3Vtree = ROOT.TTree('Layer3Vtree',
                                      'Cluster data for Layer3 V direction')
        self.SlantedUtree = ROOT.TTree('SlantedUtree',
                                       'Cluster data for Slanted U direction')
        self.SlantedVtree = ROOT.TTree('SlantedVtree',
                                       'Cluster data for Slanted V direction')
        self.OtherUtree = ROOT.TTree('OtherUtree',
                                     'Cluster data for Other U direction')
        self.OtherVtree = ROOT.TTree('OtherVtree',
                                     'Cluster data for Other V direction')
        ## Instance of the data class
        self.data = ClusterData()
        # Declare tree branches
        # Each tree has the same branches but only one of the trees
        # gets filled by a cluster depending on the cluster and sensor type
        for key in ClusterData.__dict__.keys():
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
        """ Dummy method """

    def event(self):
        clusters = Belle2.PyStoreArray('SVDClusters')
        for cluster in clusters:
            clusterTruehits = cluster.getRelationsTo('SVDTrueHits')

            # We want only clusters with exactly one associated TrueHit
            if len(clusterTruehits) != 1:
                continue

            truehit = clusterTruehits[0]

            clusterPos = cluster.getPosition()
            clusterSigma = cluster.getPositionSigma()

            # The strip detectors are oriented in two different directions so
            # we have to take the truehit position in the relevant direction
            if cluster.isUCluster():
                truehitPos = truehit.getU()
            else:
                truehitPos = truehit.getV()

            pull = (clusterPos - truehitPos) / clusterSigma

            self.data.clusterPosition = clusterPos
            self.data.clusterPositionSigma = clusterSigma
            self.data.truehitPosition = truehitPos
            self.data.pull = pull

            sensorID = cluster.getSensorID()
            sensorType = SensorType.getSensorType(sensorID)

            # Just make sure we're writing to the right file
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
        self.file.cd()
        self.file.Write()
        self.file.Close()


