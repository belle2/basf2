#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>G. Caria, gcaria@student.unimelb.edu.au</contact>
  <description>
    This module is used for the SVD validation. It creates efficiency
    plots for the two directions (U, V) and saves them to a ROOT file.
  </description>
</header>
"""
import sys
import math

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import gROOT, AddressOf
from ROOT import PyConfig
from ROOT import TVector3

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EffData {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    float truehit_eff_theta;\
    float truehit_eff_phi;\
    int truehit_eff_hascluster;\
    };')

from ROOT import EffData


class SVDValidationTTreeEfficiency(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeEfficiency, self).__init__()
        # Output ROOT file
        self.file = ROOT.TFile('SVDValidationTTreeEfficiency.root', 'recreate')
        # TTrees for output data
        self.treeU = ROOT.TTree('treeU', 'Event data of SVD validation events')
        self.treeV = ROOT.TTree('treeV', 'Event data of SVD validation events')
        # Instance of the EffData class
        self.data = EffData()

        # Declare tree branches
        for key in EffData.__dict__.keys():
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.treeU.Branch(key, AddressOf(self.data, key), key
                                  + formstring)
                self.treeV.Branch(key, AddressOf(self.data, key), key
                                  + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find truehit --> cluster(s)"""

        truehits = Belle2.PyStoreArray('SVDTrueHits')
        for truehit in truehits:
            # mcp_rel = truehit.getRelationsFrom('MCParticles')
            # negative weights are from ignored particles, we don't like them and
            # we ignore all truehits which have a negative weight
            # if mcp_rel.weight(0) < 0:
            #    continue
            # get truehit information
            sensorID = truehit.getSensorID()
            self.data.sensor_id = sensorID
            sensorNum = sensorID.getSensorNumber()
            self.data.sensor = sensorNum
            layerNum = sensorID.getLayerNumber()
            self.data.layer = layerNum
            if (layerNum == 3):
                sensorType = 1
            else:
                if (sensorNum == 1):
                    sensorType = 0
                else:
                    sensorType = 1
            self.data.sensor_type = sensorType
            ladderNum = sensorID.getLadderNumber()
            self.data.ladder = ladderNum
            sensorInfo = Belle2.VXD.GeoCache.get(truehit.getSensorID())
            # theta and phi definitions
            uPos = truehit.getU()
            vPos = truehit.getV()
            localPosition = TVector3(uPos, vPos, 0)  # sensor center at (0, 0, 0)
            globalPosition = sensorInfo.pointToGlobal(localPosition)
            x = globalPosition[0]
            y = globalPosition[1]
            z = globalPosition[2]
            # see https://d2comp.kek.jp/record/242?ln=en for the Belle II
            # coordinate system and related variables
            rho = math.sqrt(x * x + y * y)
            r = math.sqrt(x * x + y * y + z * z)
            thetaRadians = math.acos(z / r)
            theta = (thetaRadians * 180) / math.pi
            phiRadians = math.acos(x / rho)
            if y < 0:
                phi = 360 - (phiRadians * 180) / math.pi
            else:
                phi = (phiRadians * 180) / math.pi
            self.data.truehit_eff_theta = theta
            self.data.truehit_eff_phi = phi
            # we assume there are no clusters for this truehit
            has_cluster_u = 0
            has_cluster_v = 0
            # now let's look for clusters
            truehit_clusters = truehit.getRelationsFrom("SVDClusters")
            for j, cluster in enumerate(truehit_clusters):
                # we ignore all clusters where less then 100 electrons come from
                # our truehit
                #    if truehit_clusters.weight(j) < 100:
                #        continue
                if cluster.isUCluster():
                    # we have found a cluster and seen that it's a u cluster,
                    # so we change the variable to 1 and fill the u tree
                    has_cluster_u = 1  # needed for later
                    self.data.treuhit_eff_hascluster = has_cluster_u
                    self.treeU.Fill()
                else:
                    # if it's not u then it's a v cluster, so we fill the v tree with 1
                    has_cluster_v = 1  # needed for later
                    self.data.truehit_eff_hascluster = has_cluster_v
                    self.treeV.Fill()
            # the loop on the clusters is now finished,
            # but we're still in the truehit loop
            # if we haven't found a cluster the variables (both u and v
            # or only one of them) shouldn't have changed,
            # i.e. they are still zero.
            # We check if it's zero and only in this case then
            # fill the tree with 0
            if has_cluster_u == 0:
                self.data.treuhit_eff_hascluster = has_cluster_u
                self.treeU.Fill()
            if has_cluster_v == 0:
                self.data.truehit_eff_hascluster = has_cluster_v
                self.treeV.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
