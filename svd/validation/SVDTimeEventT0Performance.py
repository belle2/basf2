# !/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <input>SVDValidationTTreeCluster.root</input>
    <output>SVDTimeEventT0Performance.root</output>
    <description>
    Validation plots related to EventT0 time.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

import ROOT as R

import plotUtils as pu

inputC = R.TFile.Open("../SVDValidationTTreeCluster.root")

treeC = inputC.Get("tree")

hists = R.TFile.Open("SVDTimeEventT0Performance.root", "recreate")


pu.plotter(
    name='ClusterTime_eventt0',
    title='Cluster time - EventT0',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='cluster_clsTime - eventt0_all',
    cut=pu.cut_matched,
    descr='Time difference between cluster time and EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='ClusterTime_eventt0_svd',
    title='Cluster time - EventT0_SVD',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='cluster_clsTime - eventt0_svd',
    cut=pu.cut_matched,
    descr='Time difference between cluster time and EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)

pu.plotter(
    name='ClusterTime_eventt0_top',
    title='Cluster time - EventT0_TOP',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='cluster_clsTime - eventt0_top',
    cut=pu.cut_matched,
    descr='Time difference between cluster time and EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='ClusterTime_eventt0_cdc',
    title='Cluster time - EventT0_CDC',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='cluster_clsTime - eventt0_cdc',
    cut=pu.cut_matched,
    descr='Time difference between cluster time and EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='ClusterTime_eventt0_ecl',
    title='Cluster time - EventT0_ECL',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='cluster_clsTime - eventt0_ecl',
    cut=pu.cut_matched,
    descr='Time difference between cluster time and EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)


pu.plotter(
    name='Eventt0_all',
    title='EventT0',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='eventt0_all',
    cut=pu.cut_matched,
    descr='EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)

pu.plotter(
    name='Eventt0_TOP',
    title='EventT0_TOP',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='eventt0_top',
    cut=pu.cut_matched,
    descr='TOP EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)

pu.plotter(
    name='Eventt0_CDC',
    title='EventT0_CDC',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='eventt0_cdc',
    cut=pu.cut_matched,
    descr='CDC EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)

pu.plotter(
    name='Eventt0_SVD',
    title='EventT0_SVD',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='eventt0_svd',
    cut=pu.cut_matched,
    descr='SVD EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)

pu.plotter(
    name='Eventt0_ECL',
    title='EventT0_ECL',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='Cluster time (ns)',
    y_label='counts',
    granules=pu.g_L3_V,
    tree=treeC,
    expr='eventt0_ecl',
    cut=pu.cut_matched,
    descr='ECL EventT0 time.\
    Distribution for signal clusters.',
    check='Distribution peak around 0.',
    isShifter=False)
