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
import ROOT
from ROOT.Belle2 import CDCDedxValidationAlgorithm
import process_cosgain as cg
ROOT.gROOT.SetBatch(True)


def process_onedgain(onedpath, gt):
    """Main function to process wiregain data and generate plots."""
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

            # Plot gain
            cg.hist(0.8, 1.12, xlabel="entrance angle", ylabel="Inner 1D constants")
            plt.plot(df_enta['enta'], df_new_in['oned'], '-', rasterized=True, label="inner 1D (new)")
            plt.plot(df_enta['enta'], df_prev_in['oned'], '-', rasterized=True, label="inner 1D (prev)")
            plt.legend(fontsize=25)
            plt.tight_layout()
            plt.savefig(f'plots/constant/onedgain_inner_e{exp}_r{run}.png')
            plt.close()

            cg.hist(0.6, 1.12, xlabel="entrance angle", ylabel="Outer 1D constants")
            plt.plot(df_enta['enta'], df_new_out['oned'], '-', rasterized=True, label="outer 1D (new)")
            plt.plot(df_enta['enta'], df_prev_out['oned'], '-', rasterized=True, label="outer 1D (prev)")
            plt.legend(fontsize=25)
            plt.tight_layout()
            plt.savefig(f'plots/constant/onedgain_outer_e{exp}_r{run}.png')
            plt.close()
