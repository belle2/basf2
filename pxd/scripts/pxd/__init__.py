#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_pxd_reconstruction(path, clusterName=None, digitsName=None, usePXDClusterShapes=False):

    if usePXDClusterShapes:
        if 'ActivatePXDClusterPositionEstimator' not in [e.name() for e in path.modules()]:
            shape_activator = register_module('ActivatePXDClusterPositionEstimator')
            shape_activator.set_name('ActivatePXDClusterPositionEstimator')
            path.add_module(shape_activator)

    if 'PXDClusterizer' not in [e.name() for e in path.modules()]:
        clusterizer = register_module('PXDClusterizer')
        clusterizer.set_name('PXDClusterizer')
        if clusterName:
            clusterizer.param('Clusters', clusterName)
        if digitsName:
            clusterizer.param('Digits', digitsName)
        path.add_module(clusterizer)


def add_pxd_simulation(path, digitsName=None):

    digitizer = register_module('PXDDigitizer')
    if digitsName:
        digitizer.param('Digits', digitsName)
    path.add_module(digitizer)
