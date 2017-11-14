#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_svd_reconstruction(path, isROIsimulation=False, useNN=False, useCoG=False):

    if(useNN and useCoG):
        print("WARNING! you can't select both NN and CoG for SVD reconstruction. Using the default algorithm (TB-equivalent)")
        add_svd_reconstruction_tb(path, isROIsimulation)

    elif(not useNN and not useCoG):
        print("SVD Reconstruction algorithm: TB-quivalent")
        add_svd_reconstruction_tb(path, isROIsimulation)

    elif(useNN):
        print("SVD Reconstruction algorithm: Neural Network")
        add_svd_reconstruction_nn(path, isROIsimulation)

    elif(useCoG):
        print("SVD Reconstruction algorithm: Center Of Gravity")
        add_svd_reconstruction_CoG(path, isROIsimulation)


def add_svd_reconstruction_tb(path, isROIsimulation=False):

    if(isROIsimulation):
        splitterName = '__ROISVDDigitSplitter'
        sorterName = '__ROISVDDigitSorter'
        clusterizerName = '__ROISVDClusterizer'
        clusterName = '__ROIsvdClusters'
    else:
        splitterName = 'SVDDigitSplitter'
        sorterName = 'SVDDigitSorter'
        clusterizerName = 'SVDClusterizer'
        clusterName = ""

    if splitterName not in [e.name() for e in path.modules()]:
        splitter = register_module('SVDDigitSplitter')
        splitter.set_name(splitterName)
        path.add_module(splitter)

    if sorterName not in [e.name() for e in path.modules()]:
        sorter = register_module('SVDDigitSorter')
        sorter.set_name(sorterName)
        path.add_module(sorter)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = register_module('SVDClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('Clusters', clusterName)
        path.add_module(clusterizer)


def add_svd_reconstruction_nn(path, isROIsimulation=False, direct=False):

    if direct:
        if(isROIsimulation):
            clusterizerName = '__ROISVDClusterizerDirect'
            clusterName = '__ROIsvdClusters'
        else:
            clusterizerName = 'SVDClusterizerDirect'
            clusterName = ""

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDClusterizerDirect')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)
    else:
        if(isROIsimulation):
            fitterName = '__ROISVDNNShapeReconstructor'
            clusterizerName = '__ROISVDNNClusterizer'
            clusterName = '__ROIsvdClusters'
        else:
            fitterName = 'SVDNNShapeReconstructor'
            clusterizerName = 'SVDNNClusterizer'
            clusterName = ''

        if fitterName not in [e.name() for e in path.modules()]:
            fitter = register_module('SVDNNShapeReconstructor')
            fitter.set_name(fitterName)
            path.add_module(fitter)

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDNNClusterizer')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)


def add_svd_reconstruction_CoG(path, isROIsimulation=False):

    if(isROIsimulation):
        fitterName = '__ROISVDCoGTimeEstimator'
        clusterizerName = '__ROISVDSimpleClusterizer'
        clusterName = '__ROIsvdClusters'
    else:
        fitterName = 'SVDCoGTimeEstimator'
        clusterizerName = 'SVDSimpleClusterizer'
        clusterName = ""

    if fitterName not in [e.name() for e in path.modules()]:
        fitter = register_module('SVDCoGTimeEstimator')
        fitter.set_name(fitterName)
        path.add_module(fitter)

    if clusterizerName not in [e.name() for e in path.modules()]:
        clusterizer = register_module('SVDSimpleClusterizer')
        clusterizer.set_name(clusterizerName)
        clusterizer.param('Clusters', clusterName)
        path.add_module(clusterizer)


def add_svd_reconstruction_nn(path, isROIsimulation=False, direct=False):

    if direct:
        if(isROIsimulation):
            clusterizerName = '__ROISVDClusterizerDirect'
            clusterName = '__ROIsvdClusters'
        else:
            clusterizerName = 'SVDClusterizerDirect'
            clusterName = ""

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDClusterizerDirect')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)
    else:
        if(isROIsimulation):
            fitterName = '__ROISVDNNShapeReconstructor'
            clusterizerName = '__ROISVDNNClusterizer'
            clusterName = '__ROIsvdClusters'
        else:
            fitterName = 'SVDNNShapeReconstructor'
            clusterizerName = 'SVDNNClusterizer'
            clusterName = ''

        if fitterName not in [e.name() for e in path.modules()]:
            fitter = register_module('SVDNNShapeReconstructor')
            fitter.set_name(fitterName)
            path.add_module(fitter)

        if clusterizerName not in [e.name() for e in path.modules()]:
            clusterizer = register_module('SVDNNClusterizer')
            clusterizer.set_name(clusterizerName)
            clusterizer.param('Clusters', clusterName)
            path.add_module(clusterizer)


def add_svd_simulation(path, createDigits=False):

    digitizer = register_module('SVDDigitizer')
    if createDigits:
        digitizer.param('GenerateDigits', True)
    path.add_module(digitizer)
