##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Implements cosgain correction
'''

import pandas as pd
import matplotlib.pyplot as plt
import ROOT
from ROOT.Belle2 import CDCDedxValidationAlgorithm
ROOT.gROOT.SetBatch(True)


def hist(y_min=None, y_max=None, x_min=None, x_max=None, xlabel="", ylabel="", space=None, fs1=20, fs2=8, font=18, rota=30):
    import matplotlib.ticker as ticker
    figs, axes = plt.subplots(1, 1, figsize=(fs1, fs2))

    if y_min or y_max is not None:
        axes.set_ylim(y_min, y_max)
    if x_min or x_max is not None:
        axes.set_xlim(x_min, x_max)

    plt.xlabel(xlabel, fontsize=font)
    plt.ylabel(ylabel, fontsize=font)
    plt.xticks(fontsize=font, rotation=rota)
    plt.yticks(fontsize=font)
    if space is not None:
        axes.xaxis.set_major_locator(ticker.MultipleLocator(space))
    plt.grid()


def parse_database(database_file, calib_key):
    """Parse the database file and return exp -> [runs] mapping."""
    exp_run_dict = {}
    previous_exp = -666

    with open(database_file) as f:
        for line in f:
            if line.startswith(calib_key):
                try:
                    _, _, iov_str = line.strip().split(' ')
                    exp, run = map(int, iov_str.split(',')[:2])
                    if exp != previous_exp:
                        exp_run_dict[exp] = [run]
                        previous_exp = exp
                    else:
                        exp_run_dict[exp].append(run)
                except Exception as e:
                    print(f"[WARNING] Skipping line: {line.strip()} due to error: {e}")
                    continue
    return exp_run_dict


def process_cosgain(ccpath, gt):
    """Main function to process wiregain data and generate plots."""
    import os
    os.makedirs('plots/constant', exist_ok=True)

    database_file = f'{ccpath}/database.txt'

    # Parse the database to get exp -> [runs] mapping
    exp_run_dict = parse_database(database_file, 'dbstore/CDCDedxCosineCor')

    # Process each exp-run pair
    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            print(f"[INFO] Processing exp={exp}, run={run}")
            cal = CDCDedxValidationAlgorithm()
            cal.setGlobalTag(gt)
            prev_data = cal.getcosgain(exp, run)
            cal.setGlobalTag("")
            cal.setTestingPayload(database_file)
            new_data = cal.getcosgain(exp, run)
            cal.setTestingPayload("")

            # Extract wiregain vectors
            prev_cc = prev_data.cosgain if prev_data else []
            new_cc = new_data.cosgain if new_data else []
            costh = new_data.costh if new_data else []

            # Skip empty payloads
            if not prev_cc or not new_cc:
                print(f"[WARNING] Empty cosgain data for exp={exp}, run={run}. Skipping.")
                return None, None

            # Convert to DataFrames
            df_prev = pd.DataFrame([[x] for x in prev_cc], columns=['cosgain'])
            df_new = pd.DataFrame([[x] for x in new_cc], columns=['cosgain'])
            df_costh = pd.DataFrame([[x] for x in costh], columns=['costh'])

            # Cosine gain
            hist(0.7, 1.3, xlabel=r"$cos\theta$", ylabel="dE/dx avg. mean ($e^{+}e^{-}$)", fs1=12, fs2=8)
            plt.plot(df_costh['costh'], df_new['cosgain'], '*', rasterized=True, label="costh (new)")
            plt.plot(df_costh['costh'], df_prev['cosgain'], '*', rasterized=True, label="costh (prev)")
            plt.legend(fontsize=25)
            plt.tight_layout()
            plt.savefig(f'plots/constant/cosgain_e{exp}_r{run}.png')
            plt.close()

            # Cosine difference gain
            hist(-0.05, 0.05, xlabel=r"$cos\theta$", ylabel="Gain Difference (new - prev)", fs1=12, fs2=8)
            plt.plot(df_costh['costh'], df_new['cosgain']-df_prev['cosgain'], '*', rasterized=True, label="diff(new-old)")
            plt.legend(fontsize=25)
            plt.tight_layout()
            plt.savefig(f'plots/constant/cosgain_diff_e{exp}_r{run}.png')
            plt.close()
