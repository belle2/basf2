#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact> K. Adamczyk, adamczykarol@gmail.com </contact>
  <description>
    This module is used for the SVD validation.
    It gets information about clusters related to SVDRecoTracks, saving
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
gROOT.ProcessLine('struct EventDataRecoTrack {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor;\
    int sensor_type;\
    int strip_dir;\
    float cluster_clsTime;\
    float cluster_uvTimeDiff;\
};')

from ROOT import EventDataRecoTrack


class SVDValidationTTreeRecoTrack(Module):

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeRecoTrack, self).__init__()
        # Output ROOT file
        self.file = ROOT.TFile('../SVDValidationTTreeRecoTrack.root', 'recreate')
        # TTrees for output data
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')

        # Instance of the EventData class
        self.data = EventDataRecoTrack()

        # Declare tree branches
        for key in EventDataRecoTrack.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def beginRun(self):
        """ Does nothing """

    def event(self):
        """Take clusters from SVDRecoTracks with a truehit and save needed information"""
        tracks = Belle2.PyStoreArray('SVDRecoTracks')
        for track in tracks:
            # print("track.getInfo() = ", track.getInfo())
            # print("\n\nlen(tracks) = ", len(tracks))
            clusters = track.getRelationsWith('SVDClusters')
            # print("len(clusters) = ", len(clusters))
            # Start with clusters and use the relation to get the corresponding
            # digit and truehits
            #
            # Store info from previous cluster
            pre_layerNum = -999
            pre_sensor_type = -999  # 0: slanted; 1: barrel
            pre_strip_dir = -999  # 0: U; 1: V
            pre_cluster_clsTime = -999
            #
            for cluster in clusters:
                cluster_truehits = cluster.getRelationsTo('SVDTrueHits')
                # We want only clusters with exactly one associated TrueHit
                if len(cluster_truehits) != 1:
                    # print("len(cluster_truehits) != 1")
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
                    # print("layerNum = ", layerNum)
                    self.data.layer = layerNum
                    if (layerNum == 3):
                        sensorType = 1  # Barrel
                    else:
                        if (sensorNum == 1):
                            sensorType = 0
                        else:
                            sensorType = 1
                    self.data.sensor_type = sensorType
                    # print("sensorType = ", sensorType)
                    ladderNum = sensorID.getLadderNumber()
                    self.data.ladder = ladderNum
                    # Cluster information
                    self.data.cluster_clsTime = cluster.getClsTime()
                    cluster_position = cluster.getPosition()
                    if cluster.isUCluster():
                        cluster_position = cluster.getPosition(truehit.getV())
                    # Interstrip position calculations
                    if cluster.isUCluster():
                        strip_dir = 0
                    else:
                        strip_dir = 1
                    self.data.strip_dir = strip_dir
                    # print("strip_dir = ", strip_dir)
                    #
                    # U-V time diffrence for the same layers
                    self.data.cluster_uvTimeDiff = -999
                    if (self.data.layer == pre_layerNum) and (self.data.sensor_type == pre_sensor_type) and \
                            (self.data.strip_dir != pre_strip_dir):
                        self.data.cluster_uvTimeDiff = pre_cluster_clsTime - self.data.cluster_clsTime  # time_u - time_v
                    #
                    # Save info about previous cluster for next iteration
                    pre_layerNum = self.data.layer
                    pre_sensor_type = self.data.sensor_type
                    pre_strip_dir = self.data.strip_dir
                    pre_cluster_clsTime = self.data.cluster_clsTime
                    # Fill tree
                    self.file.cd()
                    self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
