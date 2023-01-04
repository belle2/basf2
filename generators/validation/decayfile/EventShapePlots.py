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
    <input>MCvalidation.root</input>
    <output>EventShapePlots.root</output>
    <contact>Frank Meier; frank.meier@belle2.org</contact>
    <description>Comparing event shape variables</description>
</header>
"""

import ROOT


def PlottingHistos(var):
    ''' Plotting function'''

    hist = rdf.Histo1D((var, var, 25, range_dic[var][0], range_dic[var][1]), var)
    hist.SetTitle(f";{axis_dic[var]}; Events")
    hist.GetListOfFunctions().Add(ROOT.TNamed('Description', axis_dic[var]))
    hist.GetListOfFunctions().Add(ROOT.TNamed('Check', 'Shape should not change drastically.'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@belle2.org'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    hist.Write()


if __name__ == '__main__':

    # load the root file into RDataFrame
    rdf = ROOT.RDataFrame("EventShape", "MCvalidation.root")

    # define the variables to plot
    colnames = rdf.GetColumnNames()
    all_list = [str(x) for x in colnames if x[0] != "_"]

    # define dictionaries for the ranges and axis-labels
    range_dic = {'foxWolframR1': [0, 0.15],
                 'foxWolframR2': [0, 0.4],
                 'foxWolframR3': [0, 0.25],
                 'foxWolframR4': [0, 0.3],
                 'cleoConeThrust0': [0, 2.5],
                 'cleoConeThrust1': [0, 5],
                 'cleoConeThrust2': [0, 6],
                 'cleoConeThrust3': [0, 6],
                 'cleoConeThrust4': [0, 6],
                 'cleoConeThrust5': [0, 6],
                 'cleoConeThrust6': [0, 5],
                 'cleoConeThrust7': [0, 4],
                 'cleoConeThrust8': [0, 2],
                 'sphericity': [0, 1],
                 'aplanarity': [0, 0.5],
                 'thrust': [0.5, 0.9],
                 'thrustAxisCosTheta': [0, 1],
                 'harmonicMomentThrust0': [0.5, 1.5],
                 'harmonicMomentThrust1': [-0.4, 0.4],
                 'harmonicMomentThrust2': [0, 0.8],
                 'harmonicMomentThrust3': [-0.4, 0.4],
                 'harmonicMomentThrust4': [-0.3, 0.5],
                 }

    axis_dic = {'foxWolframR1': 'R_{1}',
                'foxWolframR2': 'R_{2}',
                'foxWolframR3': 'R_{3}',
                'foxWolframR4': 'R_{4}',
                'cleoConeThrust0': "Cleo Cone 0",
                'cleoConeThrust1': "Cleo Cone 1",
                'cleoConeThrust2': "Cleo Cone 2",
                'cleoConeThrust3': "Cleo Cone 3",
                'cleoConeThrust4': "Cleo Cone 4",
                'cleoConeThrust5': "Cleo Cone 5",
                'cleoConeThrust6': "Cleo Cone 6",
                'cleoConeThrust7': "Cleo Cone 7",
                'cleoConeThrust8': "Cleo Cone 8",
                'sphericity': "Sphericity",
                'aplanarity': "Aplanarity",
                'thrust': "Thrust",
                'thrustAxisCosTheta': "ThrustAxisCosTheta",
                'harmonicMomentThrust0': "harmonicMomentThrust0",
                'harmonicMomentThrust1': "harmonicMomentThrust1",
                'harmonicMomentThrust2': "harmonicMomentThrust2",
                'harmonicMomentThrust3': "harmonicMomentThrust3",
                'harmonicMomentThrust4': "harmonicMomentThrust4"
                }

    outputFile = ROOT.TFile("EventShapePlots.root", "RECREATE")
    ROOT.gROOT.SetBatch(True)
    ROOT.gROOT.SetStyle("BELLE2")
    ROOT.gROOT.ForceStyle()

    # plot the histograms
    for var in all_list:
        PlottingHistos(var)

    outputFile.Close()
