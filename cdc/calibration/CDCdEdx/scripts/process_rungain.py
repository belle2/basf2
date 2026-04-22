##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Implements rungain correction"""

import os
import ROOT
import pandas as pd
import matplotlib.pyplot as plt
import process_cosgain as cg
from ROOT.Belle2 import CDCDedxValidationAlgorithm
from matplotlib.backends.backend_pdf import PdfPages


def compute_block_averages(values, nblocks):
    block_size = max(1, len(values) // nblocks)
    averages = []
    for i in range(0, len(values), block_size):
        block = values[i:i + block_size]
        avg = sum(block) / len(block)
        averages.extend([avg] * len(block))
    return averages[:len(values)]


def parse_database(database_file, calib_key):
    """
    Parses a calibration database file and returns:
    {exp: [(run, rev), ...]}
    """
    exp_run_dict = {}

    with open(database_file) as f:
        for line in f:
            if line.startswith(calib_key):
                try:
                    parts = line.strip().split()
                    if len(parts) != 3:
                        continue
                    _, rev, iov_str = parts
                    exp, run = map(int, iov_str.split(',')[:2])

                    exp_run_dict.setdefault(exp, []).append((run, rev))
                except Exception as e:
                    print(f"[WARNING] Skipping line: {line.strip()} | Error: {e}")
    return exp_run_dict


def getRunGain(gtpath, gt):
    os.makedirs("plots/constant", exist_ok=True)
    database_file = os.path.join(gtpath, "database.txt")
    calib_key = "dbstore/CDCDedxRunGain"

    exp_run_dict = parse_database(database_file, calib_key)
    rows = []
    prev = []

    for exp, run_rev_list in exp_run_dict.items():
        for run, rev in run_rev_list:
            cal = CDCDedxValidationAlgorithm()
            cal.setGlobalTag(gt)
            prev_data = cal.getrungain(exp, run)
            cal.setGlobalTag("")
            if prev_data is not None:
                prev.append({"exp": exp, "run": run, "gain": prev_data})

            root_file = os.path.join(gtpath, f"dbstore_CDCDedxRunGain_rev_{rev}.root")
            file = ROOT.TFile.Open(root_file, "READ")
            if file and not file.IsZombie():
                rgain = file.Get("CDCDedxRunGain")
                if rgain:
                    gain = rgain.getRunGain()
                    rows.append({"exp": exp, "run": run, "gain": gain})
            else:
                print(f"[ERROR] Failed to open file or file is zombie: {root_file}")

    # Create DataFrame
    df_prev = pd.DataFrame(prev)
    df = pd.DataFrame(rows)
    if df.empty:
        print("[WARNING] No data to process.")
        return df

    for exp, df_exp in df.groupby("exp"):
        df_prev_exp = df_prev[df_prev["exp"] == exp]

        gains = df_exp["gain"].tolist()
        average = compute_block_averages(gains, nblocks=30)
        df_exp["run"] = df_exp["run"].astype(str)

        # Sort runs for matching
        df_exp_sorted = df_exp.sort_values("run")
        df_prev_exp_sorted = df_prev_exp.sort_values("run")

        # Compute ratio (new/prev)
        if len(df_prev_exp_sorted) == len(df_exp_sorted):
            ratio = df_exp_sorted["gain"].values / df_prev_exp_sorted["gain"].values
        else:
            print(f"[WARNING] Exp {exp}: mismatch in number of runs, skipping ratio plot.")
            ratio = None

        sorted_gains = sorted(gains)
        y_min, y_max = min(sorted_gains) - 0.04, max(sorted_gains) + 0.04
        spacing = max(1, len(gains) // 10)

        # Open PDF for this experiment
        with PdfPages(f"plots/constant/rungain_exp{exp}.pdf") as pdf:
            fig, ax = plt.subplots(1, 2, figsize=(20, 6))

            # Left plot: RunGain constants
            cg.hist(y_min=y_min, y_max=y_max, xlabel="Run", ylabel="RunGain constant", space=spacing, ax=ax[0])
            ax[0].plot(df_exp["run"], df_exp["gain"], '*', rasterized=True, label=f"exp{exp}")
            ax[0].plot(df_exp["run"], average, '-', rasterized=True, label="Block Avg (30 blocks)")
            ax[0].legend(fontsize=12)

            # Right plot: Ratio (if available)
            if ratio is not None:
                cg.hist(0.99, 1.01, xlabel="Run", ylabel="Gain Ratio (new/prev)", space=spacing, ax=ax[1])
                ax[1].plot(df_exp_sorted["run"], ratio, '*', rasterized=True, label="New / Prev")
                ax[1].axhline(1.0, color='gray', linestyle='--')
                ax[1].legend(fontsize=12)
            else:
                ax[1].text(0.5, 0.5, "Ratio unavailable", ha="center", va="center", fontsize=15)
                ax[1].set_axis_off()

            fig.suptitle(f"RunGain Calibration - Experiment {exp}", fontsize=20)
            plt.tight_layout()
            pdf.savefig(fig)
            plt.close()

    return df
