##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Implements cosgain correction"""

import os
import ROOT
import pandas as pd
import matplotlib.pyplot as plt
import process_cosgain as cg


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


def getRunGain(gtpath):
    os.makedirs("plots/constant", exist_ok=True)
    database_file = os.path.join(gtpath, "database.txt")
    calib_key = "dbstore/CDCDedxRunGain"

    exp_run_dict = parse_database(database_file, calib_key)
    rows = []

    for exp, run_rev_list in exp_run_dict.items():
        for run, rev in run_rev_list:
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
    df = pd.DataFrame(rows)
    if df.empty:
        print("[WARNING] No data to process.")
        return df

    # Plotting per experiment
    for exp, df_exp in df.groupby("exp"):

        gains = df_exp["gain"].tolist()
        average = compute_block_averages(gains, nblocks=30)
        df_exp["run"] = df_exp["run"].astype(str)

        sorted_gains = sorted(gains)
        y_min, y_max = min(sorted_gains) - 0.04, max(sorted_gains) + 0.04
        spacing = max(1, len(gains) // 10)

        cg.hist(y_min=y_min, y_max=y_max, xlabel="Run", ylabel="RunGain constant", space=spacing)
        plt.plot(df_exp["run"], df_exp["gain"], '*', rasterized=True, label=f"exp{exp}")
        plt.plot(df_exp["run"], average, '-', rasterized=True, label="Block Avg (full range in 30 nblocks)")
        plt.legend(fontsize=15)
        plt.tight_layout()
        plt.savefig(f"plots/constant/rungain_exp{exp}.png")
        plt.close()

    return df
