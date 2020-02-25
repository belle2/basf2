# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTreeTrack.root</input>
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
input = R.TFile.Open("../SVDValidationTTreeTrack.root")

tree = input.Get("tree")

histsTP = R.TFile.Open("TrackingPerformance.root", "recreate")
