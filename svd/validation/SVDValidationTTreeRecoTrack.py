#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact> SVD Software Group, svd-software@belle2.org </contact>
  <description>
    This module is used for the SVD validation.
    It gets information about clusters related to SVDRecoTracks, saving
    in a ttree in a ROOT file.
  </description>
</header>
"""
from collections import OrderedDict

import basf2 as b2

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, addressof

# Define a ROOT struct to hold output data in the TTree
gROOT.ProcessLine('struct EventDataRecoTrack {\
    int sensor_id;\
    int layer;\
    int ladder;\
    int sensor_type;\
    int strip_dir;\
    int cluster_truehits_number;\
    float cluster_UVTimeDiff;\
    float cluster_UUTimeDiff;\
    float cluster_VVTimeDiff;\
    int clusters_number;\
};')

from ROOT import EventDataRecoTrack  # noqa


class SVDValidationTTreeRecoTrack(b2.Module):
    '''class to create the reco track ttree'''

    def __init__(self):
        """Initialize the module"""

        super(SVDValidationTTreeRecoTrack, self).__init__()
        #: output root file
        self.file = ROOT.TFile('../SVDValidationTTreeRecoTrack.root', 'recreate')
        #: output ttree
        self.tree = ROOT.TTree('tree', 'Event data of SVD validation events')
        #: instance of EventData class
        self.data = EventDataRecoTrack()

        # Declare tree branches
        for key in EventDataRecoTrack.__dict__:
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, addressof(self.data, key), key + formstring)

    def event(self):
        """Take clusters from SVDRecoTracks with a truehit and save needed information"""
        tracks = Belle2.PyStoreArray('SVDRecoTracks')
        for track in tracks:
            clusters = track.getRelationsWith('SVDClusters')
            dict_cluster = OrderedDict({'L3': [], 'L4': [], 'L5': [], 'L6': []})  # keys: layers, values: list of dicts
            # clusters are ordered acording layers and sides, so we can read two at once
            for i in range(0, len(clusters), 2):
                c_U, c_V = clusters[i], clusters[i + 1]  # read at once two clusters
                cluster_U_truehits = c_U.getRelationsTo('SVDTrueHits')  # SVDClustersToSVDTrueHits
                U_id = c_U.getSensorID()
                sensorNum_U = U_id.getSensorNumber()
                layer_U = U_id.getLayerNumber()
                clusters_number = -1
                if (layer_U == 3):
                    sensor_type_U = 1
                    clusters_number = len(clusters)
                else:
                    if (sensorNum_U == 1):
                        sensor_type_U = 0
                    else:
                        sensor_type_U = 1
                cluster_U = {'sensor_id': int(U_id),
                             'layer': U_id.getLayerNumber(),
                             'ladder': U_id.getLadderNumber(),
                             'sensor_type': sensor_type_U,
                             'strip_dir': 0,
                             'cluster_truehits_number': len(cluster_U_truehits),
                             'cluster_clsTime': c_U.getClsTime() if len(cluster_U_truehits) == 1 else -128,
                             'cluster_UVTimeDiff': -128,
                             'cluster_UUTimeDiff': -128,
                             'cluster_VVTimeDiff': -128,
                             'clusters_number': clusters_number}
                cluster_V_truehits = c_V.getRelationsTo('SVDTrueHits')  # SVDClustersToSVDTrueHits
                V_id = c_V.getSensorID()
                sensorNum_V = V_id.getSensorNumber()
                layer_V = V_id.getLayerNumber()
                if (layer_V == 3):
                    sensor_type_V = 1
                else:
                    if (sensorNum_V == 1):
                        sensor_type_V = 0
                    else:
                        sensor_type_V = 1
                cluster_V = {'sensor_id': int(V_id),
                             'layer': V_id.getLayerNumber(),
                             'ladder': V_id.getLadderNumber(),
                             'sensor_type': sensor_type_V,
                             'strip_dir': 1,
                             'cluster_truehits_number': len(cluster_V_truehits),
                             'cluster_clsTime': c_V.getClsTime() if len(cluster_V_truehits) == 1 else -128,
                             'cluster_UVTimeDiff': c_U.getClsTime() - c_V.getClsTime()
                             if (len(cluster_U_truehits) == 1) and (len(cluster_V_truehits) == 1) else -128,
                             'cluster_UUTimeDiff': -128,
                             'cluster_VVTimeDiff': -128,
                             'clusters_number': clusters_number}
                #
                if U_id.getLayerNumber() == 3:
                    dict_cluster['L3'].append(cluster_U)
                    dict_cluster['L3'].append(cluster_V)
                elif U_id.getLayerNumber() == 4:
                    dict_cluster['L4'].append(cluster_U)
                    dict_cluster['L4'].append(cluster_V)
                elif U_id.getLayerNumber() == 5:
                    dict_cluster['L5'].append(cluster_U)
                    dict_cluster['L5'].append(cluster_V)
                elif U_id.getLayerNumber() == 6:
                    dict_cluster['L6'].append(cluster_U)
                    dict_cluster['L6'].append(cluster_V)
                else:
                    raise Exception('Incorrect number of layer')

            for (i, layer) in enumerate(dict_cluster.items()):
                if i < len(dict_cluster) - 1:
                    next_layer = list(dict_cluster.items())[i + 1]
                    # print(layer[0], next_layer[0], end='')
                    if (layer[0] == "L3" and next_layer[0] == "L4" and len(layer[1]) != 0 and len(next_layer[1]) != 0) or \
                        (layer[0] == "L4" and next_layer[0] == "L5" and len(layer[1]) != 0 and len(next_layer[1]) != 0) or \
                            (layer[0] == "L5" and next_layer[0] == "L6" and len(layer[1]) != 0 and len(next_layer[1]) != 0):
                        cluster_UUTimeDiff = \
                            layer[1][0]['cluster_clsTime'] - next_layer[1][0]['cluster_clsTime']
                        cluster_VVTimeDiff = \
                            layer[1][1]['cluster_clsTime'] - next_layer[1][1]['cluster_clsTime']
                        layer[1][0].update({'cluster_UUTimeDiff': cluster_UUTimeDiff})
                        layer[1][0].update({'cluster_VVTimeDiff': -128})
                        layer[1][1].update({'cluster_UUTimeDiff': -128})
                        layer[1][1].update({'cluster_VVTimeDiff': cluster_VVTimeDiff})
                    else:
                        # print("continue")
                        continue
            # save all clusters
            for layer in dict_cluster.items():
                for c in layer[1]:
                    if len(c) != 0:
                        self.data.sensor_id = c['sensor_id']
                        self.data.layer = c['layer']
                        self.data.ladder = c['ladder']
                        self.data.sensor_type = c['sensor_type']
                        self.data.strip_dir = c['strip_dir']
                        self.data.cluster_truehits_number = c['cluster_truehits_number']
                        self.data.cluster_UVTimeDiff = c['cluster_UVTimeDiff']
                        self.data.cluster_UUTimeDiff = c['cluster_UUTimeDiff']
                        self.data.cluster_VVTimeDiff = c['cluster_VVTimeDiff']
                        self.data.clusters_number = c['clusters_number']
                    else:
                        print(layer[0], ": empty list")
                    # Fill tree
                    self.file.cd()
                    self.tree.Fill()

    def terminate(self):
        """Close the output file. """
        self.file.cd()
        self.file.Write()
        self.file.Close()
