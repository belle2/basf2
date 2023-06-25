#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <input>MCvalidationCharged.root</input>
    <output>MultiplicityPlotsCharged.root</output>
    <contact>Frank Meier; frank.meier@belle2.org</contact>
    <description>Comparing generated particle multiplicities</description>
</header>
"""

import ROOT


def PlottingHistos(particle):
    ''' Plotting function'''

    nbins = int(range_dic[particle][1] - (range_dic[particle][0]))

    hist = rdf.Histo1D((var, var, nbins, range_dic[particle][0], range_dic[particle][1]), var)
    hist.SetTitle(f";{axis_dic[particle]}; Events")
    hist.GetListOfFunctions().Add(ROOT.TNamed('Description', f'{axis_dic[particle]} multiplicity'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('Check', 'Shape should not change drastically.'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@belle2.org'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    hist.Write()


if __name__ == '__main__':

    # load the root file into RDataFrame
    rdf = ROOT.RDataFrame("Multiplicities", "MCvalidationCharged.root")

    # define the variables to plot
    colnames = rdf.GetColumnNames()
    all_list = [str(x) for x in colnames if x[0] == "n"]
    all_list.remove("nB0")
    all_list.remove("nBp")

    # define dictionaries for the axis-labels and the ranges
    range_dic = {'nPIp': [-0.5, 25.5],
                 'nPI0': [-0.5, 20.5],
                 'nETA': [-0.5, 4.5],
                 'nETAprim': [-0.5, 2.5],
                 'nPHI': [-0.5, 3.5],
                 'nRHOp': [-0.5, 6.5],
                 'nRHO0': [-0.5, 4.5],
                 'nKp': [-0.5, 6.5],
                 'nKL0': [-0.5, 5.5],
                 'nKS0': [-0.5, 5.5],
                 'nKstar0': [-0.5, 4.5],
                 'nKstarp': [-0.5, 4.5],
                 'nDp': [-0.5, 3.5],
                 'nD0': [-0.5, 4.5],
                 'nJPSI': [-0.5, 1.5],
                 'nELECTRON': [-0.5, 6.5],
                 'nENEUTRINO': [-0.5, 4.5],
                 'nMUON': [-0.5, 4.5],
                 'nMNEUTRINO': [-0.5, 4.5],
                 'nTAUON': [-0.5, 2.5],
                 'nTNEUTRINO': [-0.5, 4.5],
                 'nPHOTON': [-0.5, 35.5]
                 }

    axis_dic = {'nPIp': '#pi^{+}',
                'nPI0': '#pi^{0}',
                'nETA': '#eta',
                'nETAprim': "#eta'",
                'nPHI': '#phi',
                'nRHOp': '#rho^{0}',
                'nRHO0': '#rho^{+}',
                'nKp': 'K^{+}',
                'nKL0': 'K_{L}^{0}',
                'nKS0': 'K_{S}^{0}',
                'nKstar0': 'K^{*,0}',
                'nKstarp': 'K^{*,+}',
                'nDp': 'D^{+}',
                'nD0': 'D^{0}',
                'nJPSI': 'J/#psi',
                'nELECTRON': 'e^{#minus}',
                'nENEUTRINO': '#nu_{e}^{#minus}',
                'nMUON': '#mu^{#minus}',
                'nMNEUTRINO': '#nu_{#mu}^{#minus}',
                'nTAUON': '#tau^{#minus}',
                'nTNEUTRINO': '#nu_{#tau}^{#minus}',
                'nPHOTON': '#gamma'}

    outputFile = ROOT.TFile("MultiplicityPlotsCharged.root", "RECREATE")
    ROOT.gROOT.SetBatch(True)
    ROOT.gROOT.SetStyle("BELLE2")
    ROOT.gROOT.ForceStyle()

    # plot the histograms
    for var in all_list:
        PlottingHistos(var)

    outputFile.Close()
