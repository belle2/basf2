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
    <description>Comparing generated particle multiplicities</description>
</header>
"""

import uproot
import matplotlib.pyplot as plt
# import numpy as np

plt.rcParams['axes.prop_cycle'] = plt.cycler(color=["#7fcdbb", "#081d58"])


def PlottingCompHistos(particle):
    ''' Plotting function to compare generated particle multiplicities between two MC samples'''

    nbins = int(range_dic[particle][1] - (range_dic[particle][0]))

    # get unnormalised bin counts
    raw, _, _ = plt.hist(file[var], bins=nbins, range=range_dic[particle])
    # raw2, outbins, _ = plt.hist(old[var], bins=nbins, range=range_dic[particle], histtype='step')
    plt.close()

    # plot normalised particle multiplicities histograms
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(5, 5), dpi=300, sharex=True, gridspec_kw={"height_ratios": [3.5, 1]})
    count1, _, _ = ax1.hist(file[var], bins=nbins, range=range_dic[particle], density=True)
    # count2, outbins, _ = ax1.hist(old[var], bins=nbins, range=range_dic[particle], histtype='step', density=True)

    # # calculate ratios
    # bin_centers = outbins[:-1] + np.diff(outbins) / 2
    # ratio = count1/count2
    # err = np.sqrt((1/raw)+(1/raw2))

    # # plot residuals
    # ax2.errorbar(x=bin_centers, y=ratio, yerr=err, ls='', marker='_',  markersize='10', color="#4575b4")
    # ax2.axhline(1.0, alpha=0.3)

    # add labels and legend
    # ax1.legend()
    # ax2.set_xlabel(axis_dic[particle])
    # ax2.set_ylabel(r'$\dfrac{New}{Old}$')
    ax1.set_ylabel("Norm. Entries/Bin")
    # ax2.set_ylim(0.9, 1.1)

    # save histograms
    fig.tight_layout()
    plt.savefig(str(var) + '.png')


if __name__ == '__main__':

    # load the root files
    file = uproot.open("MCvalidation.root:Multiplicities").array(library='pd')

    # define the variables to plot
    all_list = [
        'nPIp',
        'nPI0',
        'nETA',
        'nETAprim',
        'nPHI',
        'nRHOp',
        'nRHO0',
        'nKp',
        'nKL0',
        'nKS0',
        'nKstar0',
        'nKstarp',
        'nDp',
        'nD0',
        'nJPSI',
        'nELECTRON',
        'nENEUTRINO',
        'nMUON',
        'nMNEUTRINO',
        'nTAUON',
        'nTNEUTRINO',
        'nPHOTON']

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

    axis_dic = {'nPIp': r'$\pi^+$',
                'nPI0': r'$\pi^0$',
                'nETA': r'$\eta$',
                'nETAprim': r"$\eta'$",
                'nPHI': r'$\phi$',
                'nRHOp': r'$\rho^0$',
                'nRHO0': r'$\rho^+$',
                'nKp': r'$K^+$',
                'nKL0': r'$K_L^0$',
                'nKS0': r'$K_S^0$',
                'nKstar0': r'$K^{*,0}$',
                'nKstarp': r'$K^{*,+}$',
                'nDp': r'$D^+$',
                'nD0': r'$D^0$',
                'nJPSI': r'$J/\psi$',
                'nELECTRON': r'$e^-$',
                'nENEUTRINO': r'$\nu_e^-$',
                'nMUON': r'$\mu^-$',
                'nMNEUTRINO': r'$\nu_{\mu}^-$',
                'nTAUON': r'$\tau^-$',
                'nTNEUTRINO': r'$\nu_{\tau}^-$',
                'nPHOTON': r'$\gamma$'}

    # plot the histograms
    for var in all_list:
        PlottingCompHistos(var)
