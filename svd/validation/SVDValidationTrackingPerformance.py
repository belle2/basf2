# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTreeRecoTrack.root</input>
    <output>TrackingPerformance.root</output>
    <description>
    Validation plots related to tracking performance.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

from plotUtils import *

# open the files with simulated and reconstructed events data
input = R.TFile.Open("../SVDValidationTTreeRecoTrack.root")

tree = input.Get("tree")

histsTP = R.TFile.Open("TrackingPerformance.root", "recreate")

ploter(
    name='U_V_time_difference',
    title='U-V time difference',
    nbins=50,
    xmin=-5,
    xmax=5,
    x_label='Cluster time difference (ns)',
    y_label='counts',
    granules=granulesLayersTypes,
    tree=tree,
    expr='cluster_uvTimeDiff',
    cut='',
    descr='time difference between opposite sides of clusters belonging to the same layer',
    check='peak around 0',
    isShifter=True)
