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
import matplotlib.ticker as ticker
# import ROOT
from ROOT.Belle2 import CDCDedxValidationAlgorithm
from matplotlib.backends.backend_pdf import PdfPages

# ROOT.gROOT.SetBatch(True)


def hist(y_min=None, y_max=None, x_min=None, x_max=None,
         xlabel="", ylabel="", space=None, fx=1, fy=1,
         fs1=20, fs2=8, font=18, rota=30, ax=None):
    """
    Configure histogram-style plot appearance.

    If `ax` is None, applies to the current pyplot figure.
    If `ax` is provided, applies to that axis (for subplots).
    """
    if ax is None:
        fig, ax = plt.subplots(fx, fy, figsize=(fs1, fs2))
    else:
        fig = ax.get_figure()

    if y_min is not None or y_max is not None:
        ax.set_ylim(y_min, y_max)
    if x_min is not None or x_max is not None:
        ax.set_xlim(x_min, x_max)

    ax.set_xlabel(xlabel, fontsize=font)
    ax.set_ylabel(ylabel, fontsize=font)
    ax.tick_params(axis='x', labelsize=font, rotation=rota)
    ax.tick_params(axis='y', labelsize=font)

    if space is not None:
        ax.xaxis.set_major_locator(ticker.MultipleLocator(space))

    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.3f'))
    ax.grid()

    return fig, ax


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

            pdf_path = f'plots/constant/cosgain_e{exp}_r{run}.pdf'
            with PdfPages(pdf_path) as pdf:
                fig, axes = plt.subplots(1, 2, figsize=(20, 6))

                # Left plot
                hist(0.7, 1.3, xlabel=r"$\cos\theta$", ylabel="dE/dx avg. mean ($e^{+}e^{-}$)", ax=axes[0])
                axes[0].plot(df_costh['costh'], df_new['cosgain'], '*', label='new')
                axes[0].plot(df_costh['costh'], df_prev['cosgain'], '*', label='prev')
                axes[0].legend()

                # Right plot
                hist(0.99, 1.01, xlabel=r"$\cos\theta$", ylabel="Gain Ratio (new/prev)", ax=axes[1])
                axes[1].plot(df_costh['costh'], df_new['cosgain']/df_prev['cosgain'], '*', label='ratio')
                axes[1].legend()

                fig.suptitle(f"CosGain Calibration - Experiment {exp}", fontsize=20)
                fig.tight_layout()
                pdf.savefig(fig)
                plt.close(fig)
