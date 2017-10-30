#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_svd_reconstruction(path, clusterName=""):

    svd_clusterizer = register_module('SVDClusterizer')
    svd_clusterizer.param('Clusters', clusterName)
    path.add_module(svd_clusterizer)


def add_svd_reconstruction_nn(path, clusterName="", direct=False):

    if direct:
        svd_clusterizer = register_module('SVDClusterizerDirect')
        svd_clusterizer.param('Clusters', clusterName)
        path.add_module(svd_clusterizer)
    else:
        svd_recofitter = register_module('SVDNNShapeFitter')
        svd_clusterizer = register_module('SVDNNClusterizer')
        svd_clusterizer.param('Clusters', clusterName)
        path.add_module(svd_recofitter)
        path.add_module(svd_clusterizer)


def add_svd_simulation(path):

    svd_digitizer = register_module('SVDDigitizer')
    path.add_module(svd_digitizer)
