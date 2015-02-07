#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <output>SVDClusterPullData.root</output>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>
....This module is part of the SVD validation suite. It creates 
....cluster position pull distributions for U and V directions
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
        """Initialize the module"""

        super(SVDClusterPull, self).__init__()
        ## Output ROOT file.
        self.file = ROOT.TFile('SVDClusterPullData.root', 'recreate')
        ## TTree for output data
        self.Utree = ROOT.TTree('Utree', 'Cluster data for U direction')
        self.Vtree = ROOT.TTree('Vtree', 'Cluster data for V direction')
        ## Instance of EventData class
        self.data = ClusterData()
        # Declare tree branches
        for key in ClusterData.__dict__.keys():
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

            truehit = clusterTruehits[0]

            clusterPos = cluster.getPosition()
            clusterSigma = cluster.getPositionSigma()

            if cluster.isUCluster():
                truehitPos = truehit.getU()
            else:
                truehitPos = truehit.getV()

            pull = (clusterPos - truehitPos) / clusterSigma

            self.data.clusterPosition = clusterPos
            self.data.clusterPositionSigma = clusterSigma
            self.data.truehitPosition = truehitPos
            self.data.pull = pull

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


