#!/usr/bin/env python
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
    <input>MCvalidation.root</input>
    <description>Comparing event shape variables</description>
</header>
"""

from root_pandas import read_root
import matplotlib.pyplot as plt
# import numpy as np

plt.rcParams['axes.prop_cycle'] = plt.cycler(color=["#7fcdbb", "#081d58"])


def PlottingCompHistos(var):
    ''' Plotting function to create two histograms and the corresponding residuals '''
    # get unnormalised bin counts
    raw, _, _ = plt.hist(file[var], bins=bins, range=range_dic[var])
    # raw2, outbins, _ = plt.hist(old[var], bins=bins, range=range_dic[var], histtype='step')
    plt.close()

    # plot normalised event shape histograms
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(5, 5), dpi=300, sharex=True, gridspec_kw={"height_ratios": [3.5, 1]})
    count1, _, _ = ax1.hist(file[var], bins=bins, range=range_dic[var], density=True)
    # count2, outbins, _ = ax1.hist(old[var], bins=bins, range=range_dic[var], histtype='step', density=True)

    # # calculate ratios
    # bin_centers = outbins[:-1] + np.diff(outbins) / 2
    # ratio = count1/count2
    # err = np.sqrt((1/raw)+(1/raw2))

    # # plot residuals
    # ax2.errorbar(x=bin_centers, y=ratio, yerr=err, ls='', marker='_',  markersize='10', color="#4575b4")
    # ax2.axhline(1.0, alpha=0.3)

    # # add labels and legend
    # ax1.legend()
    # ax2.set_xlabel(axis_dic[var])
    # ax2.set_ylabel(r'$\dfrac{New}{Old}$')
    ax1.set_ylabel("Norm. Entries/Bin")
    # ax2.set_ylim(0.9, 1.1)

    # save histograms
    fig.tight_layout()
    plt.savefig(str(var) + '.png')
    plt.close()


if __name__ == '__main__':

    # load in the two root files
    file = read_root("MCvalidation.root", key="EventShape")

    bins = 25

    # define list of variables to plot
    FWM_list = ['foxWolframR1', 'foxWolframR2', 'foxWolframR3', 'foxWolframR4']
    HM_list = [
        'harmonicMomentThrust0',
        'harmonicMomentThrust1',
        'harmonicMomentThrust2',
        'harmonicMomentThrust3',
        'harmonicMomentThrust4']
    oth_list = ['thrustAxisCosTheta', 'thrust', 'aplanarity', 'sphericity']
    cc_list = [
        'cleoConeThrust0',
        'cleoConeThrust1',
        'cleoConeThrust2',
        'cleoConeThrust3',
        'cleoConeThrust4',
        'cleoConeThrust5',
        'cleoConeThrust6',
        'cleoConeThrust7',
        'cleoConeThrust8']
    all_list = FWM_list + HM_list + oth_list + cc_list

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

    axis_dic = {'foxWolframR1': '$R_{1}$',
                'foxWolframR2': '$R_{2}$',
                'foxWolframR3': '$R_{3}$',
                'foxWolframR4': '$R_{4}$',
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

    # plot the variables
    for var in all_list:
        PlottingCompHistos(var)
