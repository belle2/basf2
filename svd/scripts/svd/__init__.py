#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_svd_reconstruction(path, clusterName=""):

    svd_clusterizer = register_module('SVDClusterizer')
    svd_clusterizer.param('Clusters', clusterName)
    path.add_module(svd_clusterizer)


def add_svd_reconstruction_CoG(path, clusterName=""):

    fitter = register_module('SVDCoGTimeEstimator')
    path.add_module(fitter)

    svd_clusterizer = register_module('SVDSimpleClusterizer')
    svd_clusterizer.param('Clusters', clusterName)
    path.add_module(svd_clusterizer)


def add_svd_simulation(path):

    svd_digitizer = register_module('SVDDigitizer')
    path.add_module(svd_digitizer)
