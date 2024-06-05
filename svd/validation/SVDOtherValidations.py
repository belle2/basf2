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
    xmin=0,
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
