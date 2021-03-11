# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTreeSimhit.root</input>
    <output>SVDOtherValidations.root</output>
    <description>
    Additional validation plots not related to other categories.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

import ROOT as R
import plotUtils as pu

inputSim = R.TFile.Open("../SVDValidationTTreeSimhit.root")

treeSim = inputSim.Get("tree")

histsOV = R.TFile.Open("SVDOtherValidations.root", "recreate")

pu.plotter(
    name='dEdxForSimhits',
    title='dE/dx for SimHits',
    nbins=100,
    xmin=1,
    xmax=10,
    x_label='dE/dx (MeV/cm)',
    y_label='counts',
    granules=pu.gD2,
    tree=treeSim,
    expr='simhit_dEdx*1000',
    cut='',
    descr='Simulated dE/dx for SimHits.',
    check='Distribution should peak around 2.8 MeV/cm.',
    isShifter=False)
