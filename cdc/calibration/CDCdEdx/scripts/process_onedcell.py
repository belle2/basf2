##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Implements 1D correction"""

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import ROOT
from ROOT.Belle2 import CDCDedxValidationAlgorithm
import process_cosgain as cg
ROOT.gROOT.SetBatch(True)


def process_onedgain(onedpath, gt):
    """Main function to process 1D cell gain data and generate plots."""
    import os
    os.makedirs('plots/constant', exist_ok=True)

    database_file = f'{onedpath}/database.txt'
    exp_run_dict = cg.parse_database(database_file, 'dbstore/CDCDedx1DCell')

    group_labels = ["SL0", "SL1", "SL2-8"]

    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            print(f"[INFO] Processing exp={exp}, run={run}")

            cal = CDCDedxValidationAlgorithm()
            cal.setGlobalTag(gt)
            prev_data = cal.getonedgain(exp, run)

            cal.setGlobalTag("")
            cal.setTestingPayload(database_file)
            new_data = cal.getonedgain(exp, run)
            cal.setTestingPayload("")

            prev_corr = prev_data.oneDcorr if prev_data else [[], [], []]
            new_corr = new_data.oneDcorr if new_data else [[], [], []]
            enta = new_data.enta if new_data else []

            enta = list(enta) if enta is not None else []
            df_enta = pd.DataFrame(enta, columns=['enta'])

            pdf_path = f'plots/constant/onedgain_e{exp}_r{run}.pdf'
            with PdfPages(pdf_path) as pdf:
                fig, ax = plt.subplots(3, 2, figsize=(18, 16))

                for igroup, glabel in enumerate(group_labels):
                    prev_vals = prev_corr[igroup] if len(prev_corr) > igroup else []
                    new_vals = new_corr[igroup] if len(new_corr) > igroup else []

                    df_prev = pd.DataFrame([[x] for x in prev_vals], columns=['oned'])
                    df_new = pd.DataFrame([[x] for x in new_vals], columns=['oned'])

                    # constants
                    cg.hist(0.7, 1.5, xlabel="entrance angle", ylabel=f"{glabel} constants", ax=ax[igroup, 0])
                    ax[igroup, 0].plot(df_enta['enta'], df_new['oned'], '-', rasterized=True, label=f"{glabel} (new)")
                    ax[igroup, 0].plot(df_enta['enta'], df_prev['oned'], '-', rasterized=True, label=f"{glabel} (prev)")
                    ax[igroup, 0].legend(fontsize=12)

                    # ratio
                    cg.hist(0.9, 1.4, xlabel="entrance angle",  ylabel="Ratio (new/prev)", ax=ax[igroup, 1])
                    if len(df_prev) > 0 and len(df_new) > 0:
                        ratio = df_new['oned'] / df_prev['oned']
                        ax[igroup, 1].plot(df_enta['enta'], ratio, '-', rasterized=True, label=f"{glabel} ratio")
                    ax[igroup, 1].legend(fontsize=12)

                fig.suptitle(f"OneD Gain Calibration - Experiment {exp}, Run {run}", fontsize=20)
                plt.tight_layout()
                pdf.savefig(fig)
                plt.close(fig)
