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
from ROOT.Belle2 import CDCDedxValidationAlgorithm
import process_cosgain as cg
from matplotlib.backends.backend_pdf import PdfPages


def remove_nowg(df):
    # Inner layers: index < 1280 and wiregain != 0
    il = df.loc[(df.index < 1280) & (df['wiregain'] != 0)].copy()

    # Outer layers: index >= 1280 and wiregain != 0
    ol = df.loc[(df.index >= 1280) & (df['wiregain'] != 0)].copy()

    return il, ol


def sep_inout(df):
    il = df[df.index < 8].copy()
    ol = df[df.index >= 8].copy()
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


def process_layermean(cal, exp, run, database_file, gt):

    # Also add layer mean plots
    cal.setGlobalTag(gt)
    prev_data = cal.getwiregain(exp, run)
    cal.setGlobalTag("")
    cal.setTestingPayload(database_file)
    new_data = cal.getwiregain(exp, run)
    cal.setTestingPayload("")

    df_prev_lm = pd.DataFrame([[x] for x in prev_data.layermean], columns=['layergain'])
    df_new_lm = pd.DataFrame([[x] for x in new_data.layermean], columns=['layergain'])

    il_lm_prev, ol_lm_prev = sep_inout(df_prev_lm)
    il_lm_new, ol_lm_new = sep_inout(df_new_lm)

    il_ratio = (il_lm_new['layergain'] / il_lm_prev['layergain']).dropna()
    ol_ratio = (ol_lm_new['layergain'] / ol_lm_prev['layergain']).dropna()

    return il_ratio, ol_ratio


def plot_wiregain_all(il_prev, ol_prev, il_new, ol_new, exp, run, pdf, il_lay, ol_lay):
    """Plot overlay, ratio and difference into a single PDF page."""

    fig, ax = plt.subplots(2, 2, figsize=(20, 12))

    # 1. Wiregain overlay (top-left)
    ax[0, 0].set_title("Inner previous Wiregain")
    cg.hist(0., 4, xlabel="#wire", ylabel="wg constant", space=100, ax=ax[0, 0])
    ax[0, 0].plot(il_prev['wiregain'], 'o', label="Inner (prev)", markersize=5, alpha=0.7)
    ax[0, 0].legend(fontsize=12)

    ax[0, 1].set_title("Outer previous Wiregain")
    cg.hist(0., 3, xlabel="#wire", ylabel="wg constant", space=1000, ax=ax[0, 1])
    ax[0, 1].plot(ol_prev['wiregain'], '^', label="Outer (prev)", markersize=5, alpha=0.7)
    ax[0, 1].legend(fontsize=12)

    ax[1, 0].set_title("Inner new Wiregain")
    cg.hist(0., 4, xlabel="#wire", ylabel="wg constant", space=100, ax=ax[1, 0])
    ax[1, 0].plot(il_new['wiregain'], 's', label="Inner (new)", markersize=5, alpha=0.7)
    ax[1, 0].legend(fontsize=12)

    ax[1, 1].set_title("Outer new Wiregain")
    cg.hist(0., 3, xlabel="#wire", ylabel="wg constant", space=1000, ax=ax[1, 1])
    ax[1, 1].plot(ol_new['wiregain'], 'D', label="Outer (new)", markersize=5, alpha=0.7)
    ax[1, 1].legend(fontsize=12)

    fig.suptitle(f"WireGain Calibration - Experiment {exp}", fontsize=20)
    plt.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)

    fig, ax = plt.subplots(2, 2, figsize=(20, 12))

    # # 2. Ratio (top-right)
    ax[0, 0].set_title("Inner Wiregain Ratio")
    il_r = (il_new['wiregain'] / il_prev['wiregain']).dropna()
    ol_r = (ol_new['wiregain'] / ol_prev['wiregain']).dropna()
    cg.hist(0.7, 1.5, xlabel="#wire", ylabel="wiregain ratio", space=100, ax=ax[0, 0])
    ax[0, 0].plot(il_r, '*', rasterized=True, label='Inner Layer')
    ax[0, 0].legend(fontsize=12)

    ax[0, 1].set_title("Outer Wiregain Ratio")
    il_r = (il_new['wiregain'] / il_prev['wiregain']).dropna()
    ol_r = (ol_new['wiregain'] / ol_prev['wiregain']).dropna()
    cg.hist(0.7, 1.5, xlabel="#wire", ylabel="wiregain ratio", space=1000, ax=ax[0, 1])
    ax[0, 1].plot(ol_r, '*', rasterized=True, label='Outer Layer')
    ax[0, 1].legend(fontsize=12)

    # 4. Ratio (middle-right)
    ax[1, 0].set_title("Inner Wiregain Ratio (zoom)")
    cg.hist(0.92, 1.05, xlabel="#wire", ylabel="wiregain ratio", space=100, ax=ax[1, 0])
    ax[1, 0].plot(il_r, '*', rasterized=True, label='Inner Layer')
    ax[1, 0].legend(fontsize=12)

    ax[1, 1].set_title("Outer Wiregain Ratio (zoom)")
    cg.hist(0.92, 1.05, xlabel="#wire", ylabel="wiregain ratio", space=1000, ax=ax[1, 1])
    ax[1, 1].plot(ol_r, '*', rasterized=True, label='Outer Layer')
    ax[1, 1].legend(fontsize=12)

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)

    fig, ax = plt.subplots(1, 2, figsize=(20, 6))

    # 5. Ratio Histogram  (bottom -left)
    ax[0].set_title("Wiregain Ratio histo")
    cg.hist(x_min=0.7, x_max=1.5, xlabel=r"$\Delta$ wiregains", ylabel="Normalized count", fs1=10, fs2=6, ax=ax[0])

    scale1 = 1 / il_r.shape[0] if il_r.shape[0] > 0 else 1
    scale2 = 1 / ol_r.shape[0] if ol_r.shape[0] > 0 else 1

    counts1, bins1 = np.histogram(il_r, bins=200)
    counts2, bins2 = np.histogram(ol_r, bins=200)

    ax[0].hist(bins1[:-1], bins=200, histtype='step', linewidth=2.5,
               weights=counts1 * scale1, label='Inner layer')
    ax[0].hist(bins2[:-1], bins=200, histtype='step', linewidth=2.5,
               weights=counts2 * scale2, label='Outer Layer')

    ax[0].legend(fontsize=12)

    # 6. Layer Gain Ratio (bottom-right)
    ax[1].set_title("Layer Gain Ratio")
    cg.hist(0.9, 1.1, xlabel="#layer", ylabel="layer average ratio", space=5, fs1=10, fs2=6, ax=ax[1])
    ax[1].plot(il_lay, '*', label="Inner Layers")
    ax[1].plot(ol_lay, '*', label="Outer Layers")
    ax[1].legend(fontsize=12)

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)


def process_wiregain(wgpath, gt):
    """Main function to process wiregain data and generate plots."""
    import os
    os.makedirs('plots/constant', exist_ok=True)
    database_file = f'{wgpath}/database.txt'

    exp_run_dict = cg.parse_database(database_file, "dbstore/CDCDedxWireGain")

    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            print(f"[INFO] Processing exp={exp}, run={run}")
            cal = CDCDedxValidationAlgorithm()
            il_prev, ol_prev, il_new, ol_new = process_wiregain_pair(cal, exp, run, database_file, gt)

            if il_prev is None or ol_prev is None or il_new is None or ol_new is None:
                continue

            il_lay, ol_lay = process_layermean(cal, exp, run, database_file, gt)

            pdf_path = f'plots/constant/wiregain_e{exp}_r{run}.pdf'
            with PdfPages(pdf_path) as pdf:
                plot_wiregain_all(il_prev, ol_prev, il_new, ol_new, exp, run, pdf, il_lay, ol_lay)

    return exp_run_dict
