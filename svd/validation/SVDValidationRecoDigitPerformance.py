# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTreeRecoDigit.root</input>
    <output>SVDRecoDigitPerformance.root</output>
    <description>
    Validation plots related to RecoDigit performance.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

from plotUtils import *

inputRD = R.TFile.Open("../SVDValidationTTreeRecoDigit.root")

treeRD = inputRD.Get("tree")

histsRDP = R.TFile.Open("SVDRecoDigitPerformance.root", "recreate")

ploter(
    name='TimeResolution',
    title='Resolution of the reconstructed time of a single strip',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='RecoDigit time resolution (ns)',
    y_label='counts',
    granules=gD,
    tree=treeRD,
    expr='recodigit_time - truehit_time',
    cut='',
    descr='Definition: (reconstructed time of the RecoDigit) - (time of the Truehit);\
    for all RecoDigit related to one TrueHit.',
    check='Distribution between -40 and 20 ns.',
    isShifter=True)

ploter(
    name='StripCharge',
    title='Charge of the strip',
    nbins=50,
    xmin=0,
    xmax=120000,
    x_label='Strip charge (# of electrons)',
    y_label='counts',
    granules=gD,
    tree=treeRD,
    expr='recodigit_charge',
    cut='',
    descr='Strip charge for all RecoDigits related to one TrueHit.',
    check='Distribution peak around 15-30 ke.',
    isShifter=False)
