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
    <input>SVDValidationTTreeRecoTrack.root</input>
    <input>SVDValidationTTreeSpacePoint.root</input>
    <output>SVDTrackingPerformance.root</output>
    <description>
    Validation plots related to tracking performance.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

import ROOT as R
import plotUtils as pu

inputRT = R.TFile.Open("../SVDValidationTTreeRecoTrack.root")
inputSP = R.TFile.Open("../SVDValidationTTreeSpacePoint.root")

treeRT = inputRT.Get("tree")
treeSP = inputSP.Get("tree")

histsTP = R.TFile.Open("SVDTrackingPerformance.root", "recreate")


pu.plotter(
    name='SpacePointTime_U',
    title='SpacePoint time on U side',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='SP time (ns)',
    y_label='counts',
    granules=pu.gD2,
    tree=treeSP,
    expr='time_u',
    cut='',
    descr='Time of the U cluster which belong to Space Point. Distribution for all clusters: signal + background.',
    check='Distribution peak around 0.',
    isShifter=True)


pu.plotter(
    name='SpacePointTime_V',
    title='SpacePoint time on V side',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='SP time (ns)',
    y_label='counts',
    granules=pu.gD2,
    tree=treeSP,
    expr='time_v',
    cut='',
    descr='Time of the V cluster which belong to SP. Distribution for all clusters: signal + background.',
    check='Distribution peak around 0 for signal.',
    isShifter=True)


pu.plotter(
    name='cluster_UVTimeDiff',
    title='U-V time difference',
    nbins=80,
    xmin=-20,
    xmax=20,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=pu.gD2,
    tree=treeRT,
    expr='cluster_UVTimeDiff',
    cut=pu.cut_oneTH,
    descr='Time difference between opposite sides of clusters belonging to the same layer.\
    Distribution for signal clusters.',
    check='Distribution peak around 0 for signal.',
    isShifter=True)


pu.plotter(
    name='cluster_UUTimeDiff',
    title='U-U time difference',
    nbins=80,
    xmin=-20,
    xmax=20,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=pu.granulesTD,
    tree=treeRT,
    expr='cluster_UUTimeDiff',
    cut=pu.cut_U+pu.cut_oneTH,
    descr='Time difference between clusters belonging to the neighbour layers.\
    Distribution for signal clusters.',
    check='Distributions peak around 0.',
    isShifter=True)


pu.plotter(
    name='cluster_VVTimeDiff',
    title='V-V time difference',
    nbins=80,
    xmin=-20,
    xmax=20,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=pu.granulesTD,
    tree=treeRT,
    expr='cluster_VVTimeDiff',
    cut=pu.cut_V+pu.cut_oneTH,
    descr='Time difference between clusters belonging to the neighbour layers.\
    Distribution for signal clusters.',
    check='Distributions peak around 0.',
    isShifter=True)


pu.plotRegions(
    name='ClusterizationPurity_U',
    title='Purity of clusters from tracks for U side',
    x_label='SVD regions',
    y_label='Purity',
    granules=pu.granulesLayersTypes,
    tree=treeRT,
    expr='strip_dir',
    cutALL=pu.cut_noV,
    cut=pu.cut_oneTH,
    descr='Definition: (number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of signal and background clusters.',
    check='Should be close to 1 in all bins',
    isShifter=True)


pu.plotRegions(
    name='ClusterizationPurity_V',
    title='Purity of clusters from tracks for V side',
    x_label='SVD regions',
    y_label='Purity',
    granules=pu.granulesLayersTypes,
    tree=treeRT,
    expr='strip_dir',
    cutALL=pu.cut_noU,
    cut=pu.cut_oneTH,
    descr='Definition: (number of clusters related to one TrueHit) / (number of clusters).\
    Evaluates the fraction of signal cluster over the total number of signal and background clusters.',
    check='Should be close to 1 in all bins.',
    isShifter=True)


pu.plotter(
    name='clusters_number',
    title='Number of clusters in one track',
    nbins=12,
    xmin=0,
    xmax=13,
    x_label='Number of clusters in one track',
    y_label='counts',
    granules=pu.granulesL3456,
    tree=treeRT,
    expr='clusters_number',
    cut=pu.cut_U,
    descr='Number of all clusters (signal + background) in one track.',
    check='Maximum is expected for 8, i.e. each cluster for one of 4 layers,\
    separately for U and V side.',
    isShifter=True)
