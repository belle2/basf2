# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTree.root</input>
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

# open the files with simulated and reconstructed events data
input = R.TFile.Open("../SVDValidationTTree.root")

tree = input.Get("tree")

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
    cut='(cluster_size==1)&&(strip_dir==0)',
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
    cut='(cluster_size==1)&&(strip_dir==1)',
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
    cut='(cluster_size==2)&&(strip_dir==0)',
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
    cut='(cluster_size==2)&&(strip_dir==1)',
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
    cut='(cluster_size>2)&&(strip_dir==0)',
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
    cut='(cluster_size>2)&&(strip_dir==1)',
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
    cut='(cluster_size==1)&&(strip_dir==0)',
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
    cut='(cluster_size==1)&&(strip_dir==1)',
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
    cut='(cluster_size==2)&&(strip_dir==0)',
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
    cut='(cluster_size==2)&&(strip_dir==1)',
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
    cut='(cluster_size>2)&&(strip_dir==0)',
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
    cut='(cluster_size>2)&&(strip_dir==1)',
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
    cut='(strip_dir==0)',
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
    cut='(strip_dir==1)',
    descr='(reconstructed time of the Cluster) - (time of the Truehit) for all Clusters related to TrueHits',
    check='peak around 0',
    isShifter=True)
