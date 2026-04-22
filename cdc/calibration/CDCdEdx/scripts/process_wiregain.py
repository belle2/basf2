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
from matplotlib.backends.backend_pdf import PdfPages
ROOT.gROOT.SetBatch(True)


def remove_nowg(df):
    # Inner layers: index < 2240 and wiregain != 0
    il = df.loc[(df.index < 2240) & (df['wiregain'] != 0)].copy()

    # Outer layers: index >= 2240 and wiregain != 0
    ol = df.loc[(df.index >= 2240) & (df['wiregain'] != 0)].copy()

    return il, ol


def sep_inout(df):
    il = df[df.index < 14].copy()
    ol = df[df.index >= 14].copy()
    return il, ol


def fetch_wiregain_pair(cal, exp, run, database_file, gt):
    """
    Fetch previous and new wiregain payloads once for a given exp/run.
    """
    prev_data = None
    new_data = None

    try:
        cal.setGlobalTag(gt)
        prev_data = cal.getwiregain(exp, run)
    finally:
        cal.setGlobalTag("")

    try:
        cal.setTestingPayload(database_file)
        new_data = cal.getwiregain(exp, run)
    finally:
        cal.setTestingPayload("")

    return prev_data, new_data


def process_wiregain_pair(prev_data, new_data, exp, run):
    """Process wiregain for a given exp-run pair."""

    # Extract wiregain vectors
    prev_wg = prev_data.wiregain if prev_data else []
    new_wg = new_data.wiregain if new_data else []

    # Skip empty payloads
    if not prev_wg or not new_wg:
        print(f"[WARNING] Empty wiregain data for exp={exp}, run={run}. Skipping.")
        return None, None, None, None

    # Convert to DataFrames
    df_prev = pd.DataFrame([[x] for x in prev_wg], columns=['wiregain'])
    df_new = pd.DataFrame([[x] for x in new_wg], columns=['wiregain'])

    # Split into inner and outer layers
    il_prev, ol_prev = remove_nowg(df_prev)
    il_new, ol_new = remove_nowg(df_new)

    return il_prev, ol_prev, il_new, ol_new


def process_layermean(prev_data, new_data, exp, run):

    # Also add layer mean plots
    if prev_data is None or new_data is None:
        print(f"[WARNING] Missing layermean data for exp={exp}, run={run}. Skipping.")
        return pd.Series(dtype=float), pd.Series(dtype=float)

    prev_layermean = list(prev_data.layermean) if hasattr(prev_data, "layermean") else []
    new_layermean = list(new_data.layermean) if hasattr(new_data, "layermean") else []

    if len(prev_layermean) == 0 or len(new_layermean) == 0:
        print(f"[WARNING] Empty layermean data for exp={exp}, run={run}. Skipping.")
        return pd.Series(dtype=float), pd.Series(dtype=float)

    df_prev_lm = pd.DataFrame(prev_layermean, columns=["layergain"])
    df_new_lm = pd.DataFrame(new_layermean, columns=["layergain"])
    il_lm_prev, ol_lm_prev = sep_inout(df_prev_lm)
    il_lm_new, ol_lm_new = sep_inout(df_new_lm)

    return il_lm_prev, ol_lm_prev, il_lm_new, ol_lm_new


def plot_wiregain_all(il_prev, ol_prev, il_new, ol_new, exp, run, pdf):
    """Plot overlay, ratio and difference into a single PDF page."""

    fig, ax = plt.subplots(2, 2, figsize=(20, 12))

    # 1.raw wiregain plots
    raw_panels = [
        (ax[0, 0], il_prev, "wiregain", "Inner previous Wiregain", "o", "Inner (prev)", 0., 4, 100),
        (ax[0, 1], ol_prev, "wiregain", "Outer previous Wiregain", "^", "Outer (prev)", 0., 3, 1000),
        (ax[1, 0], il_new,  "wiregain", "Inner new Wiregain",      "s", "Inner (new)",  0., 4, 100),
        (ax[1, 1], ol_new,  "wiregain", "Outer new Wiregain",      "D", "Outer (new)",  0., 3, 1000),
    ]

    for axi, df, col, title, marker, label, ymin, ymax, space in raw_panels:
        axi.set_title(title)
        cg.hist(ymin, ymax, xlabel="#wire", ylabel="wg constant", space=space, ax=axi)
        axi.plot(df[col], marker, label=label, markersize=5, alpha=0.7)
        axi.legend(fontsize=12)

    fig.suptitle(f"WireGain Calibration - Experiment {exp}, Run {run}", fontsize=20)
    plt.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)

    fig, ax = plt.subplots(2, 2, figsize=(20, 12))

    # # 2. Ratio (top-right)
    il_r = (il_new["wiregain"] / il_prev["wiregain"]).dropna()
    ol_r = (ol_new["wiregain"] / ol_prev["wiregain"]).dropna()

    ratio_panels = [
        (ax[0, 0], il_r, "Inner Wiregain Ratio",        0.0, 1.5, 100,  "Inner Layer"),
        (ax[0, 1], ol_r, "Outer Wiregain Ratio",        0.5, 2.0, 1000, "Outer Layer"),
        (ax[1, 0], il_r, "Inner Wiregain Ratio (zoom)", 0.3, 0.7, 100,  "Inner Layer"),
        (ax[1, 1], ol_r, "Outer Wiregain Ratio (zoom)", 0.92, 1.2, 1000, "Outer Layer"),
    ]

    for axi, series, title, ymin, ymax, space, label in ratio_panels:
        axi.set_title(title)
        cg.hist(ymin, ymax, xlabel="#wire", ylabel="wiregain ratio", space=space, ax=axi)
        axi.plot(series, "*", rasterized=True, label=label)
        axi.legend(fontsize=12)

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)

    fig, ax = plt.subplots(1, 2, figsize=(20, 6))

    # 5. Ratio Histogram  (bottom -left)
    # 5. Ratio Histogram (filled)
    ax[0].set_title("Wiregain Ratio histo")
    cg.hist(x_min=0.0, x_max=2.0, xlabel=r"$\Delta$ wiregains", ylabel="Normalized count", fs1=10, fs2=6, ax=ax[0])

    scale1 = 1 / il_r.shape[0] if il_r.shape[0] > 0 else 1
    scale2 = 1 / ol_r.shape[0] if ol_r.shape[0] > 0 else 1

    ax[0].hist(il_r, bins=200, weights=np.ones_like(il_r) * scale1, histtype='stepfilled', alpha=0.5, label='Inner layer')

    ax[0].hist(ol_r, bins=200, weights=np.ones_like(ol_r) * scale2, histtype='stepfilled', alpha=0.5, label='Outer Layer')

    ax[0].legend(fontsize=12)

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)


def plot_layergain(il_lm_prev, ol_lm_prev, il_lm_new, ol_lm_new, exp, run, pdf):
    """Compact combined plot with color separation."""

    fig, ax = plt.subplots(1, 2, figsize=(20, 6))

    data = [
        ("Inner prev", il_lm_prev, "blue",   "o"),
        ("Inner new",  il_lm_new,  "cyan",   "o"),
        ("Outer prev", ol_lm_prev, "red",    "s"),
        ("Outer new",  ol_lm_new,  "orange", "s"),
    ]

    # --- Overlay ---
    cg.hist(0.2, 1.9, xlabel="#layer", ylabel="layer gain", fs1=10, fs2=6, space=2, ax=ax[0])
    ax[0].set_title("Layer Mean")

    for label, df, color, marker in data:
        ax[0].plot(df["layergain"], marker,  color=color, label=f"{label}")

    ax[0].legend(fontsize=10)

    # --- Ratio ---
    cg.hist(0.0, 2.4, xlabel="#layer", ylabel="ratio", fs1=10, fs2=6, space=2, ax=ax[1])
    ax[1].set_title("Layer Ratio")

    ratios = [
        ("Inner", il_lm_new, il_lm_prev, "blue", "o"),
        ("Outer", ol_lm_new, ol_lm_prev, "red",  "s"),
    ]
    for label, prev_df, new_df, color, marker in ratios:
        ratio = (new_df["layergain"] / prev_df["layergain"]).dropna()
        ax[1].plot(ratio, marker, color=color, label=f"{label}")

    ax[1].legend(fontsize=10)

    fig.suptitle(f"Exp {exp}, Run {run}", fontsize=16)

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
            try:
                prev_data, new_data = fetch_wiregain_pair(cal, exp, run, database_file, gt)
            except Exception as e:
                print(f"[ERROR] Failed to fetch wiregain for exp={exp}, run={run}: {e}")
                continue

            il_prev, ol_prev, il_new, ol_new = process_wiregain_pair(prev_data, new_data, exp, run)

            if il_prev is None or ol_prev is None or il_new is None or ol_new is None:
                continue

            il_lm_prev, ol_lm_prev, il_lm_new, ol_lm_new = process_layermean(prev_data, new_data, exp, run)

            pdf_path = f'plots/constant/wiregain_e{exp}_r{run}.pdf'
            with PdfPages(pdf_path) as pdf:
                plot_wiregain_all(il_prev, ol_prev, il_new, ol_new, exp, run, pdf)
                plot_layergain(il_lm_prev, ol_lm_prev, il_lm_new, ol_lm_new, exp, run, pdf)

    return exp_run_dict
