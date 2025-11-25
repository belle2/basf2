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

    # Parse the database to get exp -> [runs] mapping
    exp_run_dict = cg.parse_database(database_file, 'dbstore/CDCDedx1DCell')

    # Process each exp-run pair
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

            # Extract vectors
            prev_inner = prev_data.inner1D if prev_data else []
            new_inner = new_data.inner1D if new_data else []
            prev_outer = prev_data.outer1D if prev_data else []
            new_outer = new_data.outer1D if new_data else []
            enta = new_data.Enta if new_data else []

            # Convert to DataFrames
            df_prev_in = pd.DataFrame([[x] for x in prev_inner], columns=['oned'])
            df_new_in = pd.DataFrame([[x] for x in new_inner], columns=['oned'])
            df_prev_out = pd.DataFrame([[x] for x in prev_outer], columns=['oned'])
            df_new_out = pd.DataFrame([[x] for x in new_outer], columns=['oned'])
            df_enta = pd.DataFrame([[x] for x in enta], columns=['enta'])

            # Create a PDF file to save the plots
            pdf_path = f'plots/constant/onedgain_e{exp}_r{run}.pdf'
            with PdfPages(pdf_path) as pdf:
                # Create 2x2 subplots
                fig, ax = plt.subplots(2, 2, figsize=(20, 12))

                # Top-left: Inner 1D
                cg.hist(0.85, 1.12, xlabel="entrance angle", ylabel="Inner 1D constants", ax=ax[0, 0])
                ax[0, 0].plot(df_enta['enta'], df_new_in['oned'], '-', rasterized=True, label="inner 1D (new)")
                ax[0, 0].plot(df_enta['enta'], df_prev_in['oned'], '-', rasterized=True, label="inner 1D (prev)")
                ax[0, 0].legend(fontsize=15)

                # Top-right: Outer 1D
                cg.hist(0.7, 1.12, xlabel="entrance angle", ylabel="Outer 1D constants", ax=ax[0, 1])
                ax[0, 1].plot(df_enta['enta'], df_new_out['oned'], '-', rasterized=True, label="outer 1D (new)")
                ax[0, 1].plot(df_enta['enta'], df_prev_out['oned'], '-', rasterized=True, label="outer 1D (prev)")
                ax[0, 1].legend(fontsize=15)

                # Bottom-left: Inner ratio
                cg.hist(0.94, 1.06, xlabel="entrance angle", ylabel="Ratio (new/prev)", ax=ax[1, 0])
                ratio_inner = df_new_in['oned'] / df_prev_in['oned']
                ax[1, 0].plot(df_enta['enta'], ratio_inner, '-', rasterized=True, label="inner ratio")
                ax[1, 0].legend(fontsize=15)

                # Bottom-right: Outer ratio
                cg.hist(0.96, 1.04, xlabel="entrance angle", ylabel="Ratio (new/prev)", ax=ax[1, 1])
                ratio_outer = df_new_out['oned'] / df_prev_out['oned']
                ax[1, 1].plot(df_enta['enta'], ratio_outer, '-', rasterized=True, label="outer ratio")
                ax[1, 1].legend(fontsize=15)

                fig.suptitle(f"OneD Gain Calibration - Experiment {exp}", fontsize=20)
                plt.tight_layout()
                pdf.savefig(fig)
                plt.close()
