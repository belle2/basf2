##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Implements wirgain correction"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import ROOT
from ROOT.Belle2 import CDCDedxValidationAlgorithm
import process_cosgain as cg

ROOT.gROOT.SetBatch(True)


def remove_nowg(df):
    # Inner layers: index < 1280 and wiregain != 0
    il = df.loc[(df.index < 1280) & (df['wiregain'] != 0)].copy()

    # Outer layers: index >= 1280 and wiregain != 0
    ol = df.loc[(df.index >= 1280) & (df['wiregain'] != 0)].copy()

    return il, ol


def process_wiregain_pair(cal, exp, run, database_file, gt):
    """Process wiregain for a given exp-run pair."""

    cal.setGlobalTag(gt)
    prev_data = cal.getwiregain(exp, run)
    cal.setGlobalTag("")
    cal.setTestingPayload(database_file)
    new_data = cal.getwiregain(exp, run)
    cal.setTestingPayload("")

    # Extract wiregain vectors
    prev_wg = prev_data.wiregain if prev_data else []
    new_wg = new_data.wiregain if new_data else []

    # Skip empty payloads
    if not prev_wg or not new_wg:
        print(f"[WARNING] Empty wiregain data for exp={exp}, run={run}. Skipping.")
        return None, None

    # Convert to DataFrames
    df_prev = pd.DataFrame([[x] for x in prev_wg], columns=['wiregain'])
    df_new = pd.DataFrame([[x] for x in new_wg], columns=['wiregain'])

    # Split into inner and outer layers
    il_prev, ol_prev = remove_nowg(df_prev)
    il_new, ol_new = remove_nowg(df_new)

    return il_prev, ol_prev, il_new, ol_new


def plot_wiregain_overlay(il_prev, ol_prev, il_new, ol_new, exp, run):
    """Plot wiregain overlay of previous and new payloads."""
    colors = {
        "il_prev": "#8c564b",  # brown
        "il_new": "#ff69b4",   # hot pink
        "ol_prev": "#17becf",  # cyan
        "ol_new": "#2ca02c"    # green
    }
    markers = {
        "il_prev": "o", "il_new": "s",
        "ol_prev": "^", "ol_new": "D"
    }

    cg.hist(0., 4, xlabel="#wire", ylabel="wg constant", space=1000)
    plt.plot(il_prev['wiregain'], marker=markers["il_prev"], color=colors["il_prev"],
             linestyle='None', markersize=5, alpha=0.7, label="Inner layer (prev)")
    plt.plot(il_new['wiregain'], marker=markers["il_new"], color=colors["il_new"],
             linestyle='None', markersize=5, alpha=0.7, label="Inner layer (new)")
    plt.plot(ol_prev['wiregain'], marker=markers["ol_prev"], color=colors["ol_prev"],
             linestyle='None', markersize=5, alpha=0.7, label="Outer layer (prev)")
    plt.plot(ol_new['wiregain'], marker=markers["ol_new"], color=colors["ol_new"],
             linestyle='None', markersize=5, alpha=0.7, label="Outer layer (new)")
    plt.legend(fontsize=20)
    plt.tight_layout()
    plt.savefig(f'plots/constant/wiregain_e{exp}_r{run}.png')
    plt.close()


def plot_wiregain_ratio(il_new, il_prev, ol_new, ol_prev, exp, run):
    """Plot the ratio of new to old wiregains."""
    il_r = il_new['wiregain'] / il_prev['wiregain']
    ol_r = ol_new['wiregain'] / ol_prev['wiregain']
    il_r, ol_r = il_r.dropna(), ol_r.dropna()

    cg.hist(0.5, 1.5, xlabel="#wire", ylabel="wiregain ratio", space=1000)
    plt.plot(il_r, '*', rasterized=True, label='Inner Layer')
    plt.plot(ol_r, '*', rasterized=True, label='Outer Layer')
    plt.legend(fontsize=20)
    plt.tight_layout()
    plt.savefig(f'plots/constant/wiregain_ratio_e{exp}_r{run}.png')
    plt.clf()


def plot_wiregain_diff(il_r, ol_r, exp, run):
    """Plot the difference (Δ) of wiregain ratios."""
    cg.hist(x_min=0.7, x_max=1.5, xlabel=r"$\Delta$ wiregains", ylabel="Normalized count", fs1=10, fs2=6)

    scale1 = 1 / il_r.shape[0] if il_r.shape[0] > 0 else 1
    scale2 = 1 / ol_r.shape[0] if ol_r.shape[0] > 0 else 1

    counts1, bins1 = np.histogram(il_r, bins=200)
    plt.hist(bins1[:-1], bins=200, histtype='step', linewidth=2.5,
             weights=counts1 * scale1, label=r'$\Delta$ gain IL')

    counts2, bins2 = np.histogram(ol_r, bins=200)
    plt.hist(bins2[:-1], bins=200, histtype='step', linewidth=2.5,
             weights=counts2 * scale2, label=r'$\Delta$ gain OL')

    plt.legend(fontsize=20)
    plt.tight_layout()
    plt.savefig(f'plots/constant/wiregain_hist_e{exp}_r{run}.png')
    plt.close()


def sep_inout(df):
    il = df[df.index < 8].copy()
    ol = df[df.index >= 8].copy()
    return il, ol


def process_layermean(cal, exp, run, database_file, gt):
    """Process wiregain for a given exp-run pair."""
    cal.setGlobalTag(gt)
    prev_data = cal.getwiregain(exp, run)
    cal.setGlobalTag("")
    cal.setTestingPayload(database_file)
    new_data = cal.getwiregain(exp, run)
    cal.setTestingPayload("")

    # Extract layermean vectors
    prev_lm = prev_data.layermean if prev_data else []
    new_lm = new_data.layermean if new_data else []

    # Skip empty payloads
    if not prev_lm or not new_lm:
        print(f"[WARNING] Empty layermean data for exp={exp}, run={run}. Skipping.")
        return None, None

    # Convert to DataFrames
    df_prev = pd.DataFrame([[x] for x in prev_lm], columns=['layergain'])
    df_new = pd.DataFrame([[x] for x in new_lm], columns=['layergain'])

    # Split into inner and outer layers
    il_prev, ol_prev = sep_inout(df_prev)
    il_new, ol_new = sep_inout(df_new)

    il_ratio = il_new['layergain'].values
    ol_ratio = ol_new['layergain'].values

    cg.hist(0.9, 1.1, xlabel="#layer", ylabel="layer average", space=5, fs1=10, fs2=6)
    plt.plot(range(len(il_ratio)), il_ratio, '*', label="Inner Layers", rasterized=True)
    plt.plot(range(len(ol_ratio)), ol_ratio, '*', label="Outer Layers", rasterized=True)

    plt.legend(fontsize=25)
    plt.savefig(f'plots/constant/layergain_hist_e{exp}_r{run}.png')


def process_wiregain(wgpath, gt):
    """Main function to process wiregain data and generate plots."""
    import os
    os.makedirs('plots/constant', exist_ok=True)

    database_file = f'{wgpath}/database.txt'

    # Parse the database to get exp -> [runs] mapping
    exp_run_dict = cg.parse_database(database_file, "dbstore/CDCDedxWireGain")

    # Process each exp-run pair
    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            print(f"[INFO] Processing exp={exp}, run={run}")
            cal = CDCDedxValidationAlgorithm()

            # Process wiregain for the exp-run pair
            il_prev, ol_prev, il_new, ol_new = process_wiregain_pair(cal, exp, run, database_file, gt)

            if il_prev is None or ol_prev is None or il_new is None or ol_new is None:
                continue

            # Plot wiregain overlay
            plot_wiregain_overlay(il_prev, ol_prev, il_new, ol_new, exp, run)

            # Plot wiregain ratio
            plot_wiregain_ratio(il_new, il_prev, ol_new, ol_prev, exp, run)

            # Plot difference in wiregains (Δ)
            plot_wiregain_diff(il_new['wiregain'] / il_prev['wiregain'], ol_new['wiregain'] / ol_prev['wiregain'], exp, run)
            # process layermean
            process_layermean(cal, exp, run, database_file, gt)
    return exp_run_dict
