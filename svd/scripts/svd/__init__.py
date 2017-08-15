#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_svd_reconstruction(path):

    svd_clusterizer = register_module('SVDClusterizer')
    path.add_module(svd_clusterizer)


def add_svd_simulation(path):

    svd_digitizer = register_module('SVDDigitizer')
    path.add_module(svd_digitizer)
