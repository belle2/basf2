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
    <description>Comparing generated kaon multiplicities, optionally split by charge and originating B meson flavor</description>
</header>
"""

from root_pandas import read_root
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.gridspec as gridspec
# import matplotlib.ticker as ticker

plt.rcParams['axes.prop_cycle'] = plt.cycler(color=["#7fcdbb", "#081d58"])


def PlottingCompHistos(particle, varlp, varlm, legend):
    ''' Function to plot histograms of particle multiplicities'''

    nbins = int(range_dic[particle][1] - (range_dic[particle][0]))

    # create figure and axes
    plt.figure(figsize=(6, 5), dpi=200)
    gs = gridspec.GridSpec(2, 2,  height_ratios=[4, 1])
    gs.update(hspace=0.1, wspace=0)

    # create subplots
    ax1 = plt.subplot(gs[0, 1])
    ax2 = plt.subplot(gs[0, 0], sharey=ax1)
    # ax1b = plt.subplot(gs[1, 1], sharex=ax1)
    # ax2b = plt.subplot(gs[1, 0], sharex=ax2, sharey=ax1b)

    # make axes that overlap invisible
    plt.setp(ax1.get_yticklabels(), visible=False)
    # plt.setp(ax1b.get_yticklabels(), visible=False)
    plt.setp(ax2.get_xticklabels(), visible=False)
    # plt.setp(ax2b.get_xticklabels(), visible=False)

    # create histograms
    count = 0
    for varnp, varnm, leg in zip(varlp, varlm, legend):
        # get unnormalised bin counts
        raw_p, _ = np.histogram(file[varnp], bins=nbins, range=range_dic[particle])
        # raw_p2, outbins = np.histogram(old[varnp], bins=nbins, range=range_dic[particle])

        raw_m, _ = np.histogram(file[varnm], bins=nbins, range=range_dic[particle])
        # raw_m2, outbins = np.histogram(old[varnm], bins=nbins, range=range_dic[particle])

        # plot normalised multiplicity histograms
        count1_p, _, _ = ax1.hist(file[varnp], bins=nbins, ls=lines[count], color=colors[count],
                                  range=range_dic[particle], density=True, histtype='step')
        # count2_p, outbins, _ = ax1.hist(old[varnp], ls=lines[count+1], bins=nbins, color=colors[count],
        #                                 range=range_dic[particle], histtype='step',  density=True)

        count1_m, _, _ = ax2.hist(file[varnm], bins=nbins, ls=lines[count], color=colors[count],
                                  range=range_dic[particle], density=True, histtype='step')
        # count2_m, outbins, _ = ax2.hist(old[varnm], ls=lines[count+1], color=colors[count], bins=nbins,
        #                                 range=range_dic[particle], histtype='step', fill=False, density=True)

        # # calculate ratios
        # bin_centers = outbins[:-1] + np.diff(outbins) / 2
        # ratio_p = count1_p/count2_p
        # err_p = np.sqrt((1/raw_p)+(1/raw_p2))
        # ratio_m = count1_m/count2_m
        # err_m = np.sqrt((1/raw_m)+(1/raw_m2))

        # # plot residuals
        # ax1b.errorbar(x=bin_centers, y=ratio_p, yerr=err_p,
        #               color=colors[count],
        #               marker=markers[2*count],
        #               ls='',
        #               markersize=markers[2*count+1])

        # ax2b.errorbar(x=bin_centers, y=ratio_m, yerr=err_m,
        #               color=colors[count],
        #               ls='',
        #               marker=markers[2*count],
        #               markersize=markers[2*count+1])

        count += 1

    # set axis limits and invert anti-particle x-axis
    ax1.set_xlim(range_dic[particle][0] - 0.5, range_dic[particle][1] + 0.5)
    ax2.set_xlim(range_dic[particle][0] - 0.5, range_dic[particle][1] + 0.5)
    ax2.invert_xaxis()

    # # set a tick frequency equal to 1
    # ax1b.xaxis.set_major_locator(ticker.MultipleLocator(1.0))
    # ax2b.xaxis.set_major_locator(ticker.MultipleLocator(1.0))

    # # draw separating line and set residuals axis
    # ax1b.axhline(1.0, alpha=0.3)
    # ax2b.axhline(1.0, alpha=0.3)
    # ax1b.set_ylim(0.9, 1.1)

    # # add some labels and titles
    # ax1b.set_xlabel(axisp_dic[particle])
    # ax2b.set_xlabel(axism_dic[particle])
    # ax2b.set_ylabel(r'$\dfrac{\mathrm{New}}{\mathrm{Old}}$')
    ax2.set_ylabel("Norm. Entries/Bin")
    # ax1.legend(frameon=False, fontsize='xx-small')

    ax2.annotate(
        f"{B}", (0.02, 0.98), xytext=(4, -4), xycoords='axes fraction',
        textcoords='offset points',
        fontweight='bold', ha='left', va='top'
    )

    # save the plots
    plt.subplots_adjust(hspace=0.2)
    if len(varlp) > 1:
        plt.savefig(str(B) + str(particle) + '_split.png')
    else:
        plt.savefig(str(B) + str(particle) + '.png')


if __name__ == '__main__':

    # load in the root files
    file = read_root("MCvalidation.root", key="Split")

    B = 'charged'

    # define axis-label and range dictionaries
    axism_dic = {'Kpm': '$\\# K^{-}$',
                 'K0': '$\\# \\overline{K^{0}}$'
                 }

    axisp_dic = {'Kpm': '$\\# K^{+}$',
                 'K0': '$\\# K^{0}$'
                 }

    range_dic = {'Kpm': [-0.5, 4.5],
                 'K0': [-0.5, 3.5]
                 }

    # define settings for the histograms
    lines = ['-', (0, (5, 1)), '-']
    markers = ["v", 3,
               "^", 3]
    colors = ["#081d58", "#7fcdbb"]

    # Plot the histograms
    PlottingCompHistos("Kpm", ["gen_Kp"], ["gen_Km"], [" from both B"])
    PlottingCompHistos("K0", ["gen_K0"], ["gen_antiK0"], [" from both B"])

    if B == 'charged':
        PlottingCompHistos("K0", ["gen_K0_Bp", "gen_K0_Bm"], ["gen_antiK0_Bp", "gen_antiK0_Bm", ], [" from Bp", " from Bm"])
    else:
        PlottingCompHistos(
            "K0", [
                "gen_K0_B0", "gen_K0_antiB0"], [
                "gen_antiK0_B0", "gen_antiK0_antiB0", ], [
                " from B0", " from antiB0"])

    if B == 'charged':
        PlottingCompHistos("Kpm", ["gen_Kp_Bp", "gen_Kp_Bm"], ["gen_Km_Bp", "gen_Km_Bm", ], [" from Bp", " from Bm"])
    else:
        PlottingCompHistos("Kpm", ["gen_Kp_B0", "gen_Kp_antiB0"], ["gen_Km_B0", "gen_Km_antiB0", ], [" from B0", " from antiB0"])
