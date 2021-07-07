#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# Visualize results from hyperparameter scan for tripletFit QE
# created by running the script tripletQE_scan.py
#####################################################################

import os
import ROOT
from pathlib import Path
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


def show_values(pc, fmt="%.3f", **kw):
    """Helper for drawing number on pcolor plot"""
    pc.update_scalarmappable()
    ax = pc.axes
    for p, color, value in zip(pc.get_paths(), pc.get_facecolors(), pc.get_array()):
        x, y = p.vertices[:-2, :].mean(0)
        if np.all(color[:3] > 0.5):
            color = (0.0, 0.0, 0.0)
        else:
            color = (1.0, 1.0, 1.0)
        ax.text(x, y, fmt % value, ha="center", va="center", color=color, **kw)


def plot_tables(data, fom='hit_efficiency'):
    """Helper to plot figure of merit vs. scan parameters"""
    table = pd.pivot_table(data, values=fom, index=['maxP'], columns=['factor'], aggfunc=np.sum)
    fig, ax = plt.subplots(figsize=(20, 10))
    pc = ax.pcolor(table)
    show_values(pc)
    ax.set_yticks(np.arange(0.5, len(table.index), 1))
    ax.set_xticks(np.arange(0.5, len(table.columns), 1))
    ax.set(xlabel='material budget factor', ylabel='maxPt', title='{} hyperparameter scan'.format(fom))
    ax.set_yticklabels(table.index)
    ax.set_xticklabels(table.columns)
    fig.savefig("{}_scan.pdf".format(fom))


def get_fom(path):
    inputfile = ROOT.TFile(str(path), 'READ')

    fomtree = inputfile.Get("vtx_comb_validation_FullReco_vxdtf2_overview_figures_of_merit")
    hfake = ROOT.TH1F("hfake", "", 10, 0, 1)
    fomtree.Draw("fake_rate>>+hfake", "", "goff")

    hclone = ROOT.TH1F("hclone", "", 10, 0, 1)
    fomtree.Draw("clone_rate>>+hclone", "", "goff")

    hfinding_efficiency = ROOT.TH1F("hfinding_efficiency", "", 10, 0, 1)
    fomtree.Draw("finding_efficiency>>+hfinding_efficiency", "", "goff")

    hhit_efficiency = ROOT.TH1F("hhit_efficiency", "", 10, 0, 1)
    fomtree.Draw("hit_efficiency>>+hhit_efficiency", "", "goff")

    return hfinding_efficiency.GetMean(), hhit_efficiency.GetMean(), hfake.GetMean(), hclone.GetMean()


if __name__ == "__main__":

    # Create a dictionary
    results = {}
    results['maxP'] = []
    results['factor'] = []
    results['finding_efficiency'] = []
    results['hit_efficiency'] = []
    results['fake_rate'] = []
    results['clone_rate'] = []

    for path in Path('git_hash=None').rglob('FoM_validation.root'):
        print(path)

        # Parse the hyper parameters from path
        ll = []
        for t in str(path).split("/"):
            for tt in t.split("="):
                try:
                    ll.append(float(tt))
                except ValueError:
                    pass

        # Extract figures of merit from root file
        finding_efficiency, hit_efficiency, fake_rate, clone_rate = get_fom(path)

        results['maxP'].append(ll[1])
        results['factor'].append(ll[0])
        results['finding_efficiency'].append(finding_efficiency)
        results['hit_efficiency'].append(hit_efficiency)
        results['fake_rate'].append(fake_rate)
        results['clone_rate'].append(clone_rate)

    results = pd.DataFrame(results)

    print('Maximum finding_efficency: ')
    print(results.iloc[results['finding_efficiency'].idxmax()])

    print('Maximum hit_efficiency: ')
    print(results.iloc[results['hit_efficiency'].idxmax()])

    print('Minimum fake rate: ')
    print(results.iloc[results['fake_rate'].idxmin()])

    print('Minimum clone rate: ')
    print(results.iloc[results['clone_rate'].idxmin()])

    plot_tables(results, fom='finding_efficiency')
    plot_tables(results, fom='hit_efficiency')
    plot_tables(results, fom='clone_rate')
    plot_tables(results, fom='fake_rate')
