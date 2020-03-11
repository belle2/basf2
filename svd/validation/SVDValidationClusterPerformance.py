# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTree.root</input>
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

# open the files with generated and reconstructed information for clusters
input = R.TFile.Open("../SVDValidationTTree.root")
input2 = R.TFile.Open("../SVDValidationTTreeTrueHit.root")
input3 = R.TFile.Open("../SVDValidationTTreeCluster.root")

tree = input.Get("tree")
tree2 = input2.Get("tree")
tree3 = input3.Get("tree")

histsCP = R.TFile.Open("SVDClusterPerformance.root", "recreate")

ploter(
    name='UPositionResolution_size1',
    title='Cluster position resolution for one strip on U side',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position resolution (cm)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_position - truehit_position',
    cut=cut_Usize1,
    descr='position resolution = (reconstructed position of the Cluster) - (position of the Truehit); for one strip on U side',
    check='peak around 0',
    isShifter=False)

ploter(
    name='VPositionResolution_size1',
    title='Cluster position resolution for one strip in V side',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position resolution (cm)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_position - truehit_position',
    cut=cut_Vsize1,
    descr='position resolution = (reconstructed position of the Cluster) - (position of the Truehit); for one strip on V side',
    check='peak around 0',
    isShifter=False)

ploter(
    name='UPositionResolution_size2',
    title='Cluster position resolution for two strips on U side',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position resolution (cm)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_position - truehit_position',
    cut=cut_Usize2,
    descr='position resolution = (reconstructed position of the Cluster) - (position of the Truehit); for two strips on U side',
    check='peak around 0',
    isShifter=False)

ploter(
    name='VPositionResolution_size2',
    title='Cluster position resolution for two strips on V side',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position resolution (cm)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_position - truehit_position',
    cut=cut_Vsize2,
    descr='position resolution = (reconstructed position of the Cluster) - (position of the Truehit); for two strips on V side',
    check='peak around 0',
    isShifter=False)

ploter(
    name='UPositionResolution_size3plus',
    title='Cluster position resolution for 3 or more strips on U side',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position resolution (cm)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_position - truehit_position',
    cut=cut_Usize3plus,
    descr='position resolution = (reconstructed position of the Cluster) - (position of the Truehit); for >2 strips on U side',
    check='peak around 0',
    isShifter=False)

ploter(
    name='VPositionResolution_size3plus',
    title='Cluster position resolution for 3 or more strips on V side',
    nbins=100,
    xmin=-0.01,
    xmax=0.01,
    x_label='Cluster position resolution (cm)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_position - truehit_position',
    cut=cut_Vsize3plus,
    descr='position resolution = (reconstructed position of the Cluster) - (position of the Truehit); for >2 strips on V side',
    check='peak around 0',
    isShifter=False)

ploter(
    name='UPositionPull_size1',
    title='Cluster position pull for one strip on U side',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_Usize1,
    descr='Pull: (cluster_position - truehit_position)/cluster_positionSigma; for one strip on U side',
    check='peak around 0 with RMS less than 2',
    isShifter=False)


ploter(
    name='VPositionPull_size1',
    title='Cluster position pull for one strip on V side',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_Vsize1,
    descr='Pull: (cluster_position - truehit_position)/cluster_positionSigma; for one strip on V side',
    check='peak around 0 with RMS less than 2',
    isShifter=False)


ploter(
    name='UPositionPull_size2',
    title='Cluster position pull for two strip on U side',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_Usize2,
    descr='Pull: (cluster_position - truehit_position)/cluster_positionSigma; for two strips on U side',
    check='peak around 0 with RMS less than 2',
    isShifter=False)

ploter(
    name='VPositionPull_size2',
    title='Cluster position pull for two strip on V side',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_Vsize2,
    descr='Pull: (cluster_position - truehit_position)/cluster_positionSigma; for two strips on V side',
    check='peak around 0 with RMS less than 2',
    isShifter=False)


ploter(
    name='UPositionPull_size3plus',
    title='Cluster position pull for 3 or more strips on U side',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_Usize3plus,
    descr='Pull: (cluster_position - truehit_position)/cluster_positionSigma; for 3 or more strips on U side',
    check='peak around 0 with RMS less than 2',
    isShifter=False)


ploter(
    name='VPositionPull_size3plus',
    title='Cluster position pull for 3 or more strips on V side',
    nbins=100,
    xmin=-5,
    xmax=5,
    x_label='Cluster position pull',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='(cluster_position - truehit_position)/cluster_positionSigma',
    cut=cut_Vsize3plus,
    descr='Pull: (cluster_position - truehit_position)/cluster_positionSigma; for 3 or more strips on V side',
    check='peak around 0 with RMS less than 2',
    isShifter=False)


ploter(
    name='timeResolution_U',
    title='Cluster time resolution on U side',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time resolution (ns)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_clsTime - truehit_time',
    cut=cut_U,
    descr='(reconstructed time of the Cluster) - (time of the Truehit) for all Clusters related to TrueHits',
    check='peak around 0',
    isShifter=True)


ploter(
    name='timeResolution_V',
    title='Cluster time resolution on V side',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time resolution (ns)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_clsTime - truehit_time',
    cut=cut_V,
    descr='(reconstructed time of the Cluster) - (time of the Truehit) for all Clusters related to TrueHits',
    check='peak around 0',
    isShifter=True)


plotRegions(
    name='ClusterizationEfficiency_U',
    title='Efficiency of clusterization for U side',
    x_label='SVD regions',
    y_label='Efficiency',
    granules=granulesLayersTypes,
    tree=tree3,
    expr='strip_dir',
    cutALL=cut_noV,
    cut=cut_U,
    descr='(number of clusters related to TrueHit) / (number of Truehits);\
          Evaluates the clusterizer efficiency. \
          This will depend on the clusterizer parameters cutSeed and cutAdjacent.',
    check='Should be close to 1 in all bins',
    isShifter=True)


plotRegions(
    name='ClusterizationEfficiency_V',
    title='Efficiency of clusterization for V side',
    x_label='SVD regions',
    y_label='Efficiency',
    granules=granulesLayersTypes,
    tree=tree3,
    expr='strip_dir',
    cutALL=cut_noU,
    cut=cut_V,
    descr='(number of clusters related to TrueHit) / (number of Truehits);\
          Evaluates the clusterizer efficiency. \
          This will depend on the clusterizer parameters cutSeed and cutAdjacent.',
    check='Should be close to 1 in all bins',
    isShifter=True)

plotRegions(
    name='ClusterizationPurity_U',
    title='Purity of clusterization for U side',
    x_label='SVD regions',
    y_label='Purity',
    granules=granulesLayersTypes,
    tree=tree3,
    expr='strip_dir',
    cutALL=cut_noV,
    cut=cut_oneTH,
    descr='(number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of clusters.',
    check='Should be close to 1 in all bins',
    isShifter=True)


plotRegions(
    name='ClusterizationPurity_V',
    title='Purity of clusterization for V side',
    x_label='SVD regions',
    y_label='Purity',
    granules=granulesLayersTypes,
    tree=tree3,
    expr='strip_dir',
    cutALL=cut_noU,
    cut=cut_oneTH,
    descr='(number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of clusters.',
    check='Should be close to 1 in all bins',
    isShifter=True)

# comment for above plot. It shows multiplicity of truehit related to one cluster
ploter(
    name='cluster_truehits_number',
    title='cluster_truehits_number',
    nbins=10,
    xmin=0,
    xmax=10,
    x_label='clusters_length',
    y_label='counts',
    granules=granulesLayersTypes,
    tree=tree3,
    expr='cluster_truehits_number',
    cut='',
    descr='Number of TrueHits related to one cluster. Signal cluster is associated to one TrueHit.',
    check='...',
    isShifter=True)
