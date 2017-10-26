#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_svd_reconstruction(path, isROIsimulation=False):

    if(isROIsimulation):
        clusterizerName = '__ROISVDClusterizer'
        clusterName = '__ROIsvdClusters'
    else:
        clusterizerName = 'SVDClusterizer'
        clusterName = ""

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = register_module('SVDClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('Clusters', clusterName)
        path.add_module(clusterizer)


def add_svd_simulation(path):

    digitizer = register_module('SVDDigitizer')
    path.add_module(digitizer)
