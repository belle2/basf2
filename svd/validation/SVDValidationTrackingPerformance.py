# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTreeRecoTrack.root</input>
    <output>SVDTrackingPerformance.root</output>
    <description>
    Validation plots related to tracking performance.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

from plotUtils import *

input = R.TFile.Open("../SVDValidationTTreeRecoTrack.root")

tree = input.Get("tree")

histsTP = R.TFile.Open("SVDTrackingPerformance.root", "recreate")


ploter(
    name='cluster_UVTimeDiff',
    title='U-V time difference',
    nbins=80,
    xmin=-20,
    xmax=20,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='cluster_UVTimeDiff',
    cut=cut_oneTH,
    descr='time difference between opposite sides of clusters belonging to the same layer',
    check='peak around 0',
    isShifter=True)


ploter(
    name='cluster_UUTimeDiff',
    title='U-U time difference',
    nbins=80,
    xmin=-20,
    xmax=20,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=granulesTD,
    tree=tree,
    expr='cluster_UUTimeDiff',
    cut=cut_U+cut_oneTH,
    descr='time difference between clusters belonging to the neighbour layers',
    check='peak around 0',
    isShifter=True)

ploter(
    name='cluster_VVTimeDiff',
    title='V-V time difference',
    nbins=80,
    xmin=-20,
    xmax=20,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=granulesTD,
    tree=tree,
    expr='cluster_VVTimeDiff',
    cut=cut_V+cut_oneTH,
    descr='time difference between clusters belonging to the neighbour layers',
    check='peak around 0',
    isShifter=True)


plotRegions(
    name='ClusterizationPurity_U',
    title='Purity of clusters from tracks for U side',
    x_label='SVD regions',
    y_label='Purity',
    granules=granulesLayersTypes,
    tree=tree,
    expr='strip_dir',
    cutALL=cut_noV,
    cut=cut_oneTH,
    descr='(number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of clusters.',
    check='Should be close to 1 in all bins',
    isShifter=True)


plotRegions(
    name='ClusterizationPurity_V',
    title='Purity of clusters from tracks for V side',
    x_label='SVD regions',
    y_label='Purity',
    granules=granulesLayersTypes,
    tree=tree,
    expr='strip_dir',
    cutALL=cut_noU,
    cut=cut_oneTH,
    descr='(number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of clusters.',
    check='Should be close to 1 in all bins',
    isShifter=True)

ploter(
    name='clusters_number',
    title='Number of clusters in one track',
    nbins=12,
    xmin=0,
    xmax=13,
    x_label='Number of clusters in one track',
    y_label='counts',
    granules=granulesL3456,
    tree=tree,
    expr='clusters_number',
    cut=cut_U,
    descr='Number of clusters in one track.',
    check='Maximum is expected for 8, i.e. each cluster for one of 4 layers,\
    separately for U and V side',
    isShifter=True)

ploter(
    name='cluster_truehits_number_U',
    title='cluster_truehits_number for U side',
    nbins=12,
    xmin=0,
    xmax=13,
    x_label='Number of truehit for one clusters',
    y_label='counts',
    granules=granulesLayersTypes,
    tree=tree,
    expr='cluster_truehits_number',
    cut=cut_noV,
    descr='Multiplicity of TrueHits related to one cluster.',
    check='Signal cluster is associated to one TrueHit.',
    isShifter=False)

ploter(
    name='cluster_truehits_number_V',
    title='cluster_truehits_number for V side',
    nbins=12,
    xmin=0,
    xmax=13,
    x_label='Number of truehit for one clusters',
    y_label='counts',
    granules=granulesLayersTypes,
    tree=tree,
    expr='cluster_truehits_number',
    cut=cut_noU,
    descr='Multiplicity of TrueHits related to one cluster.',
    check='Signal cluster is associated to one TrueHit.',
    isShifter=False)
