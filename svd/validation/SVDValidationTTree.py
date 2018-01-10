#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact> G. Caria, gcaria@student.unimelb.edu.au </contact>
  <description>
    This module is used for the SVD validation.
    It gets information about truehits and  clusters, saving
    in a ttree in a ROOT file.
  </description>
</header>
"""
import sys
import math

from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf
from ROOT import PyConfig
from ROOT import TVector3

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventData {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    float strip_pitch;\
    float cluster_theta;\
    float cluster_phi;\
    float cluster_position;\
    float cluster_positionSigma;\
    float cluster_clsTime;\
    float cluster_clsTimeSigma;\
    float cluster_charge;\
    float cluster_seedCharge;\
    float cluster_size;\
    float cluster_snr;\
    float cluster_interstripPosition;\
    float cluster_pull;\
    float cluster_residual;\
    float truehit_position;\
    float truehit_deposEnergy;\
    float truehit_lossmomentum;\
    };')

from ROOT import EventData


class SVDValidationTTree(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTree, self).__init__()
        # Output ROOT file
        self.file = ROOT.TFile('../SVDValidationTTree.root', 'recreate')
        # TTrees for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')

        # Instance of the EventData class
        self.data = EventData()

        # Declare tree branches
        for key in EventData.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Find clusters with a truehit and save needed information"""

        # Start with clusters and use the relation to get the corresponding
        # digit and truehits
        clusters = Belle2.PyStoreArray('SVDClusters')
        for cluster in clusters:
            cluster_truehits = cluster.getRelationsTo('SVDTrueHits')
            # We want only clusters with exactly one associated TrueHit
            if len(cluster_truehits) != 1:
                continue
            for truehit in cluster_truehits:
                sensorInfo = Belle2.VXD.GeoCache.get(cluster.getSensorID())
                # Let's store some data
                # Sensor identification
                sensorID = cluster.getSensorID()
                self.data.sensor_id = int(sensorID)
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
                # Cluster information
                self.data.cluster_clsTime = cluster.getClsTime()
                self.data.cluster_clsTimeSigma = cluster.getClsTimeSigma()
                self.data.cluster_charge = cluster.getCharge()
                self.data.cluster_seedCharge = cluster.getSeedCharge()
                self.data.cluster_size = cluster.getSize()
                self.data.cluster_snr = cluster.getSNR()
                cluster_position = cluster.getPosition()
                if cluster.isUCluster():
                    cluster_position = cluster.getPosition(truehit.getV())
                # Interstrip position calculations
                if cluster.isUCluster():
                    strip_dir = 0
                    strip_pitch = sensorInfo.getUPitch(cluster_position)
                else:
                    strip_dir = 1
                    strip_pitch = sensorInfo.getVPitch(cluster_position)
                self.data.strip_dir = strip_dir
                self.data.strip_pitch = strip_pitch
                cluster_interstripPosition = cluster_position % strip_pitch / strip_pitch
                self.data.cluster_interstripPosition = cluster_interstripPosition
                # theta and phi definitions
                if cluster.isUCluster():
                    uPos = cluster_position
                    vPos = 0
                else:
                    uPos = 0
                    vPos = cluster_position
                localPosition = TVector3(uPos, vPos, 0)  # sensor center at (0, 0, 0)
                globalPosition = sensorInfo.pointToGlobal(localPosition)
                x = globalPosition[0]
                y = globalPosition[1]
                z = globalPosition[2]
                # see https://d2comp.kek.jp/record/242?ln=en for the Belle II
                # coordinate system and related variables
                rho = math.sqrt(x * x + y * y)
                r = math.sqrt(x * x + y * y + z * z)
                # get theta as arccosine(z/r)
                thetaRadians = math.acos(z / r)
                theta = (thetaRadians * 180) / math.pi
                # get phi as arccosine(x/rho)
                phiRadians = math.acos(x / rho)
                if y < 0:
                    phi = 360 - (phiRadians * 180) / math.pi
                else:
                    phi = (phiRadians * 180) / math.pi
                self.data.cluster_theta = theta
                self.data.cluster_phi = phi
                # Pull calculations
                clusterPos = cluster_position
                clusterPosSigma = cluster.getPositionSigma()
                if cluster.isUCluster():
                    truehitPos = truehit.getU()
                else:
                    truehitPos = truehit.getV()
                cluster_residual = clusterPos - truehitPos
                cluster_pull = cluster_residual / clusterPosSigma
                self.data.cluster_position = clusterPos
                self.data.cluster_positionSigma = clusterPosSigma
                self.data.cluster_residual = cluster_residual
                self.data.cluster_pull = cluster_pull
                # Truehit information
                self.data.truehit_position = truehitPos
                self.data.truehit_deposEnergy = truehit.getEnergyDep()
                self.data.truehit_lossmomentum = truehit.getEntryMomentum().Mag() - truehit.getExitMomentum().Mag()
                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
