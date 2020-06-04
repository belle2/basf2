# !/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from plotUtils import *

inputTH = R.TFile.Open("../SVDValidationTTreeTrueHit.root")
inputC = R.TFile.Open("../SVDValidationTTreeCluster.root")

treeTH = inputTH.Get("tree")
treeC = inputC.Get("tree")

histsCP = R.TFile.Open("SVDClusterPerformance.root", "recreate")


ploter(
    name='ClusterTime',
    title='Cluster time',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_clsTime',
    cut=cut_oneTH,
    descr='Reconstructed time of the cluster for all clusters related to one TrueHit.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=True)


ploter(
    name='PositionResidual',
    title='Cluster position residual',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=cut_oneTH,
    descr='Definition: (reconstructed position of the cluster) - (position of the TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=True)


ploter(
    name='PositionResidual_size1',
    title='Cluster position residual for one strip',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=cut_oneTH,
    descr='Definition: (reconstructed position of the cluster) - (position of the TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


ploter(
    name='PositionResidual_size2',
    title='Cluster position residual for two strips',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=cut_oneTH,
    descr='Definition: (reconstructed position of the cluster) - (position of the TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


ploter(
    name='PositionResidual_size3plus',
    title='Cluster position residual for 3 or more strips',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position residual (cm)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_position - truehit_position',
    cut=cut_oneTH,
    descr='Definition: (reconstructed position of the cluster) - (position of the TrueHit).\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


ploter(
    name='PositionPull',
    title='Cluster position pull',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_oneTH,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=True)


ploter(
    name='PositionPull_size1',
    title='Cluster position pull for one strip',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_oneTH,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=False)


ploter(
    name='PositionPull_size2',
    title='Cluster position pull for two strip',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_oneTH,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=False)


ploter(
    name='PositionPull_size3plus',
    title='Cluster position pull for 3 or more strips',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_oneTH,
    descr='Definition: (cluster_position - truehit_position)/cluster_positionSigma.\
    Distribution for signal clusters.',
    check='Distribution peaks around 0 with RMS less than 2.0.',
    isShifter=False)


ploter(
    name='TimeResolution',
    title='Cluster time resolution',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time resolution (ns)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_clsTime - truehit_time',
    cut=cut_oneTH,
    descr='Definition: (reconstructed time of the cluster) - (time of the TrueHit)\
    for all signal clusters.',
    check='Distribution peak around 0.',
    isShifter=True)


ploter(
    name='ClusterCharge',
    title='Cluster charge',
    nbins=50,
    xmin=0,
    xmax=120000,
    x_label='Cluster charge (# of electrons)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_charge',
    cut=cut_oneTH,
    descr='Reconstructed charge of the cluster related to one TrueHit.\
    Distribution for signal clusters.',
    check='Distribution peaks around 20-40 ke.',
    isShifter=True)

ploter(
    name='ClusterSN',
    title='Cluster Signal/Noise ratio',
    nbins=121,
    xmin=-0.5,
    xmax=120.5,
    x_label='Cluster charge (# of electrons)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_snr',
    cut=cut_oneTH,
    descr='Signal/Noise ratio of the cluster related to one TrueHit. Distribution for signal clusters.',
    check='Distribution peaks around 20.',
    isShifter=True)

ploter(
    name='InterstripPosition',
    title='Interstrip position',
    nbins=50,
    xmin=0.0,
    xmax=1.0,
    x_label='Interstrip Position',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_interstripPosition',
    cut=cut_oneTH,
    descr='Definition: (cluster_position % strip_pitch / strip_pitch).\
    Distribution for signal clusters.',
    check='',
    isShifter=False)

ploter(
    name='ClusterSize',
    title='Cluster size',
    nbins=9,
    xmin=0.5,
    xmax=9.5,
    x_label='Cluster size (# of strips in cluster)',
    y_label='counts',
    granules=gD,
    tree=treeC,
    expr='cluster_size',
    cut=cut_oneTH,
    descr='Number of strips in the Cluster related to one TrueHit. Distribution for signal clusters.',
    check='Distribution peaks in range 2-3.',
    isShifter=False)

plotRegions(
    name='ClusterizationEfficiency_U',
    title='Efficiency of clusterization for U side',
    x_label='SVD regions',
    y_label='Efficiency',
    granules=granulesLayersTypes,
    tree=treeTH,
    expr='strip_dir',
    cutALL=cut_noV,
    cut=cut_U,
    descr='Definition: (number of clusters related to TrueHit) / (number of Truehits)',
    check='Efficiency should be close to 1 in all bins.',
    isShifter=True)


plotRegions(
    name='ClusterizationEfficiency_V',
    title='Efficiency of clusterization for V side',
    x_label='SVD regions',
    y_label='Efficiency',
    granules=granulesLayersTypes,
    tree=treeTH,
    expr='strip_dir',
    cutALL=cut_noU,
    cut=cut_V,
    descr='Definition: (number of clusters related to TrueHit) / (number of Truehits).',
    check='Efficiency should be close to 1 in all bins.',
    isShifter=True)


plotRegions(
    name='ClusterizationPurity_U',
    title='Purity of clusterization for U side',
    x_label='SVD regions',
    y_label='Purity',
    granules=granulesLayersTypes,
    tree=treeC,
    expr='strip_dir',
    cutALL=cut_noV,
    cut=cut_oneTH,
    descr='(number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of signal and background clusters.',
    check='Purity should be above 0 in all bins.',
    isShifter=True)


plotRegions(
    name='ClusterizationPurity_V',
    title='Purity of clusterization for V side',
    x_label='SVD regions',
    y_label='Purity',
    granules=granulesLayersTypes,
    tree=treeC,
    expr='strip_dir',
    cutALL=cut_noU,
    cut=cut_oneTH,
    descr='(number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of signal and background clusters.',
    check='Purity should be above 0 in all bins.',
    isShifter=True)
