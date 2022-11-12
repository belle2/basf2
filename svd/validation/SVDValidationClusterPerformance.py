# !/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <input>SVDValidationTTreeTrueHit.root</input>
    <input>SVDValidationTTreeCluster.root</input>
    <output>SVDClusterPerformance.root</output>
    <description>
    Validation plots related to cluster performance.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

import ROOT as R

import plotUtils as pu

inputTH = R.TFile.Open("../SVDValidationTTreeTrueHit.root")
inputC = R.TFile.Open("../SVDValidationTTreeCluster.root")

treeTH = inputTH.Get("tree")
treeC = inputC.Get("tree")

histsCP = R.TFile.Open("SVDClusterPerformance.root", "recreate")


pu.plotter(
    name='ClusterTime',
    title='Cluster time',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_clsTime',
    cut=pu.cut_matched,
    descr='Reconstructed time of the cluster for all clusters related to at least one TrueHit.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=True)


pu.plotter(
    name='PositionResidual',
    title='Cluster position residual',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=pu.cut_matched,
    descr='Definition: (reconstructed position of the cluster) - (position of the best TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='PositionResidual_size1',
    title='Cluster position residual for one strip',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=pu.cut_size1 + pu.cut_matched,
    descr='Definition: (reconstructed position of the cluster) - (position of the best TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='PositionResidual_size2',
    title='Cluster position residual for two strips',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=pu.cut_size2 + pu.cut_matched,
    descr='Definition: (reconstructed position of the cluster) - (position of the best TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='PositionResidual_size3plus',
    title='Cluster position residual for 3 or more strips',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=pu.cut_size3plus + pu.cut_matched,
    descr='Definition: (reconstructed position of the cluster) - (position of the best TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='PositionPull',
    title='Cluster position pull',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=pu.cut_matched,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=True)


pu.plotter(
    name='PositionPull_size1',
    title='Cluster position pull for one strip',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=pu.cut_size1 + pu.cut_matched,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=False)


pu.plotter(
    name='PositionPull_size2',
    title='Cluster position pull for two strip',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=pu.cut_size2 + pu.cut_matched,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=False)


pu.plotter(
    name='PositionPull_size3plus',
    title='Cluster position pull for 3 or more strips',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=pu.cut_size3plus + pu.cut_matched,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=False)


pu.plotter(
    name='TimeResolution',
    title='Cluster time resolution',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time resolution (ns)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_clsTime - truehit_time',
    cut=pu.cut_matched,
    descr='Definition: (reconstructed time of the cluster) - (time of the best TrueHit)\
    for all signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='ClusterCharge',
    title='Cluster charge',
    nbins=50,
    xmin=0,
    xmax=120000,
    x_label='Cluster charge (# of electrons)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_charge',
    cut=pu.cut_matched,
    descr='Reconstructed charge of the cluster related to at least one TrueHit.\
    Distribution for signal clusters.',
    check='Distribution peaks around 20-40 ke.',
    isShifter=True)

pu.plotter(
    name='ClusterSN',
    title='Cluster Signal/Noise ratio',
    nbins=121,
    xmin=-0.5,
    xmax=120.5,
    x_label='cluster SNR',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_snr',
    cut=pu.cut_matched,
    descr='Signal/Noise ratio of the clusters related to at least one TrueHit.',
    check='Distribution peaks around 20.',
    isShifter=True)

pu.plotter(
    name='InterstripPosition',
    title='Interstrip position',
    nbins=50,
    xmin=0.0,
    xmax=1.0,
    x_label='Interstrip Position',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_interstripPosition',
    cut=pu.cut_matched,
    descr='Definition: (cluster_position % strip_pitch / strip_pitch).\
    Distribution for signal clusters.',
    check='',
    isShifter=False)

pu.plotter(
    name='ClusterSize',
    title='Cluster size',
    nbins=9,
    xmin=0.5,
    xmax=9.5,
    x_label='Cluster size (# of strips in cluster)',
    y_label='counts',
    granules=pu.gD,
    tree=treeC,
    expr='cluster_size',
    cut=pu.cut_matched,
    descr='Number of strips in the Cluster related to at least one TrueHit.',
    check='Distribution peaks in range 2-3.',
    isShifter=False)

pu.plotRegions(
    name='ClusterizationEfficiency_U',
    title='Efficiency of clusterization for U side',
    x_label='SVD regions',
    y_label='Efficiency',
    granules=pu.granulesLayersTypes,
    tree=treeTH,
    expr='strip_dir',
    cutALL=pu.cut_notV,
    cut=pu.cut_notV + pu.cut_reco,
    descr='Definition: (number of clusters related to at least one TrueHit) / (number of best TrueHits)',
    check='Efficiency should be close to 1 in all bins. Note that only one TrueHit (the best one, \
        i.e. the one with the largest energy deposit) is counted\
        in case one cluster is related to more than one TrueHit (e.g. in case of delta rays).',
    isShifter=True)


pu.plotRegions(
    name='ClusterizationEfficiency_V',
    title='Efficiency of clusterization for V side',
    x_label='SVD regions',
    y_label='Efficiency',
    granules=pu.granulesLayersTypes,
    tree=treeTH,
    expr='strip_dir',
    cutALL=pu.cut_notU,
    cut=pu.cut_notU + pu.cut_reco,
    descr='Definition: (number of clusters related to at least one TrueHit) / (number of best TrueHits)',
    check='Efficiency should be close to 1 in all bins. Note that only one TrueHit (the best one, \
        i.e. the one with the largest energy deposit) is counted\
        in case one cluster is related to more than one TrueHit (e.g. in case of delta rays).',
    isShifter=True)


pu.plotRegions(
    name='ClusterizationPurity_U',
    title='Fraction of U-side signal clusters (purity)',
    x_label='SVD regions',
    y_label='Purity',
    granules=pu.granulesLayersTypes,
    tree=treeC,
    expr='strip_dir',
    cutALL=pu.cut_U,
    cut=pu.cut_U + pu.cut_matched,
    descr='(number of clusters related to at least one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of signal and background clusters.',
    check='Purity should be above 0 in all bins.',
    isShifter=True)


pu.plotRegions(
    name='ClusterizationPurity_V',
    title='Fraction of V-side signal clusters (purity)',
    x_label='SVD regions',
    y_label='Purity',
    granules=pu.granulesLayersTypes,
    tree=treeC,
    expr='strip_dir',
    cutALL=pu.cut_V,
    cut=pu.cut_V + pu.cut_matched,
    descr='(number of clusters related to at least one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of signal and background clusters.',
    check='Purity should be above 0 in all bins.',
    isShifter=True)
