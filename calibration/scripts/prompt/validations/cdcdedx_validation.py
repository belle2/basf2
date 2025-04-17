##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Validation plots for CDC dedx calibration.
'''

import sys
import os
import json
import logging
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import shutil

import process_wiregain as pw
import process_cosgain as pc
import process_onedcell as oned
import process_rungain as rg

from prompt import ValidationSettings

# Set up logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

settings = ValidationSettings(name="CDC dedx calibration",
                              description=__doc__,
                              download_files=[],
                              expert_config={
                                "GT": "data_prompt_rel08",
                                  })


def save_plot(filename):
    """Saves the plot with tight layout."""
    plt.tight_layout()
    plt.savefig(filename)


def rungain_validation(path, suffix):
    try:
        val_path = os.path.join(path, "plots", "run", f"dedx_vs_run_{suffix}.txt")
        df = pd.read_csv(val_path, sep=" ", header=None, names=["run", "mean", "mean_err", "reso", "reso_err"])
    except FileNotFoundError:
        logger.error(f"File {val_path} not found!")
        return

    df['run'] = df['run'].astype(str)

    # mean
    ymin = df[df['mean'] > 0]['mean'].min()
    ymax = df['mean'].max()
    pc.hist(y_min=ymin-0.02, y_max=ymax+0.02, xlabel="run range", ylabel="dE/dx mean", space=30)
    plt.errorbar(df['run'], df['mean'], yerr=df['mean_err'], fmt='*', markersize=10, rasterized=True, label='bhabha mean')
    plt.legend(fontsize=17)
    save_plot(f'plots/validation/dedx_mean_vs_run_{suffix}.png')

    # reso
    ymin = df[df['reso'] > 0]['reso'].min()
    ymax = df['reso'].max()
    pc.hist(y_min=ymin-0.01, y_max=ymax+0.01, xlabel="run range", ylabel="dE/dx reso", space=30)
    plt.errorbar(df['run'], df['reso'], yerr=df['reso_err'], fmt='*', markersize=10, rasterized=True, label='bhabha reso')
    plt.legend(fontsize=17)
    save_plot(f'plots/validation/dedx_reso_vs_run_{suffix}.png')


def wiregain_validation(path, suffix):
    try:
        val_path = os.path.join(path, "plots", "wire", f"dedx_mean_wire_{suffix}.txt")
        df = pd.read_csv(val_path, sep=" ", header=None, names=["wire", "mean"])
    except FileNotFoundError:
        logger.error(f"File not found in {val_path}")
        return

    ymin = df['mean'].min()
    ymax = df['mean'].max()
    pc.hist(x_min=0, y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Wire", ylabel="dE/dx mean", space=1000)
    plt.plot(df['wire'], df['mean'], '*', markersize=5, rasterized=True)
    save_plot(f'plots/validation/dedx_mean_vs_wire_{suffix}.png')

    try:
        val_path = os.path.join(path, "plots", "wire", f"dedx_mean_layer_{suffix}.txt")
        df = pd.read_csv(val_path, sep=" ", header=None, names=["layer", "mean"])
    except FileNotFoundError:
        logger.error(f"File not found in {val_path}")
        return

    ymin = df['mean'].min()
    ymax = df['mean'].max()
    pc.hist(x_min=0, x_max=56, y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Layer", ylabel="dE/dx mean", space=1)
    plt.plot(df['layer'], df['mean'], '*', markersize=10, rasterized=True)
    save_plot(f'plots/validation/dedx_mean_vs_layer_{suffix}.png')


def cosgain_validation(path, suffix):
    try:
        val_path_el = os.path.join(path, "plots", "costh", f"dedx_vs_cos_electrons_{suffix}.txt")
        val_path_po = os.path.join(path, "plots", "costh", f"dedx_vs_cos_positrons_{suffix}.txt")
        df_el = pd.read_csv(val_path_el, sep=" ", header=None, names=["cos", "mean", "mean_err", "reso", "reso_err"])
        df_po = pd.read_csv(val_path_po, sep=" ", header=None, names=["cos", "mean", "mean_err", "reso", "reso_err"])
    except FileNotFoundError:
        logger.error(f"Cosine data files not found in {path}/plots/costh/")
        return

    # Ensure both dataframes are sorted by 'cos' so addition is element-wise correct
    df_el = df_el.sort_values(by='cos').reset_index(drop=True)
    df_po = df_po.sort_values(by='cos').reset_index(drop=True)

    # New DataFrame with summed means
    mean_avg = (df_el['mean'] + df_po['mean']) / 2
    err_avg = err_avg = 0.5 * np.sqrt(df_el['mean_err']**2 + df_po['mean_err']**2)
    df_sum = pd.DataFrame({'cos': df_el['cos'],  'mean_sum': mean_avg, 'err_avg': err_avg})

    # mean
    ymin = df_el[df_el['mean'] > 0]['mean'].min()
    ymax = df_el['mean'].max()
    pc.hist(x_min=-1.0, x_max=1.0, y_min=ymin-0.02, y_max=ymax+0.02, xlabel=r"cos#theta", ylabel="dE/dx mean", space=0.1)
    plt.errorbar(df_el['cos'], df_el['mean'], yerr=df_el['mean_err'], fmt='*', markersize=10, rasterized=True, label='electron')
    plt.errorbar(df_po['cos'], df_po['mean'], yerr=df_po['mean_err'], fmt='*', markersize=10, rasterized=True, label='positrons')
    plt.errorbar(df_sum['cos'], df_sum['mean_sum'], yerr=df_sum['err_avg'], fmt='*',
                 markersize=10, rasterized=True, label=r'avergae of e^{+} and e^{-}')
    plt.legend(fontsize=17)
    save_plot(f'plots/validation/dedx_mean_vs_cosine_{suffix}.png')

    # reso
    ymin = df_el[df_el['reso'] > 0]['reso'].min()
    ymax = df_el['reso'].max()
    pc.hist(x_min=-1.0, x_max=1.0, y_min=ymin-0.01, y_max=ymax+0.01, xlabel=r"cos#theta", ylabel="dE/dx reso", space=0.1)
    plt.errorbar(df_el['cos'], df_el['reso'], yerr=df_el['reso_err'], fmt='*', markersize=10, rasterized=True, label='electron')
    plt.errorbar(df_po['cos'], df_po['reso'], yerr=df_po['reso_err'], fmt='*', markersize=10, rasterized=True, label='positrons')
    plt.legend(fontsize=17)
    save_plot(f'plots/validation/dedx_reso_vs_cosine_{suffix}.png')


def injection_validation(path, suffix):
    try:
        val_path_ler = os.path.join(path, "plots", "injection", f"dedx_vs_inj_ler_{suffix}.txt")
        val_path_her = os.path.join(path, "plots", "injection", f"dedx_vs_inj_her_{suffix}.txt")
        df_ler = pd.read_csv(val_path_ler, sep=" ", header=None, names=["var", "bin", "mean", "mean_err", "reso", "reso_err"])
        df_her = pd.read_csv(val_path_her, sep=" ", header=None, names=["var", "bin", "mean", "mean_err", "reso", "reso_err"])
    except FileNotFoundError:
        logger.error(f"Injection data files not found in {path}/plots/injection/")
        return

    df_ler['bin'] = df_ler['bin'].astype(str)

    ymin = df_ler[df_ler['mean'] > 0]['mean'].min()
    ymax = df_ler['mean'].max()
    pc.hist(y_min=ymin-0.01, y_max=ymax+0.01, xlabel="injection time", ylabel="dE/dx mean", space=3)
    plt.errorbar(df_ler['bin'], df_ler['mean'], yerr=df_ler['mean_err'], fmt='*', markersize=10, rasterized=True, label='LER')
    plt.errorbar(df_her['bin'], df_her['mean'], yerr=df_her['mean_err'], fmt='*', markersize=10, rasterized=True, label='HER')
    plt.legend(fontsize=19)
    save_plot(f'plots/validation/dedx_mean_inj_{suffix}.png')


def mom_validation(path, suffix):
    try:
        val_path_el = os.path.join(path, "plots", "mom", f"dedx_vs_mom_acos_elec_{suffix}.txt")
        val_path_po = os.path.join(path, "plots", "mom", f"dedx_vs_mom_acos_pos_{suffix}.txt")
        df_el = pd.read_csv(val_path_el, sep=" ", header=None, names=["mom", "mean", "mean_err", "reso", "reso_err"])
        df_po = pd.read_csv(val_path_po, sep=" ", header=None, names=["mom", "mean", "mean_err", "reso", "reso_err"])
    except FileNotFoundError:
        logger.error(f"Momentum data files not found in {path}/plots/mom/")
        return

    ymin = df_el[df_el['mean'] > 0]['mean'].min()
    ymax = df_el['mean'].max()
    pc.hist(x_min=0, y_min=ymin-0.01, y_max=ymax+0.01, xlabel="Momentum", ylabel="dE/dx mean", space=1000)
    plt.errorbar(df_el['mom'], df_el['mean'], yerr=df_el['mean_err'], fmt='*', markersize=10, rasterized=True, label='electron')
    plt.errorbar(df_po['mom'], df_po['mean'], yerr=df_po['mean_err'], fmt='*', markersize=10, rasterized=True, label='positron')
    plt.legend(fontsize=17)
    save_plot(f'plots/validation/dedx_mean_vs_mom_{suffix}.png')


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    '''
    Makes validation plots
    :job_path: path to cdcdedx calibration output
    :input_data_path: path to the input files
    :requested_iov: required argument but not used
    :expert_config: required argument
    '''
    if not os.path.exists('plots/validation'):
        os.makedirs('plots/validation')

    if not os.path.exists('plots/constant'):
        os.makedirs('plots/constant')

    expert_config = json.loads(expert_config)
    GT = expert_config["GT"]

    logger.info("Starting validation...")

    logger.info("Processing run gain payloads...")
    gtpath = os.path.join(job_path, 'rungain2', 'outputdb')
    rg.getRunGain(gtpath)

    logger.info("Processing coscorr payloads...")
    ccpath = os.path.join(job_path, 'coscorr1', 'outputdb')
    pc.process_cosgain(ccpath, GT)

    logger.info("Processing wire gain payloads...")
    wgpath = os.path.join(job_path, 'wiregain0', 'outputdb')
    exp_run_dict = pw.process_wiregain(wgpath, GT)

    logger.info("Processing 1D gain payloads...")
    ccpath = os.path.join(job_path, 'onedcell0', 'outputdb')
    oned.process_onedgain(ccpath, GT)

    logger.info("Generating validation plots...")
    val_path = os.path.join(job_path, 'validation0', '0', 'algorithm_output')

    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            logger.info("Processing rungain validation plots...")
            suffix = f'e{exp}_r{run}'
            rungain_validation(val_path, suffix)

            logger.info("Processing wire gain validation plots...")
            wiregain_validation(val_path, suffix)

            logger.info("Processing cosine correction validation plots...")
            cosgain_validation(val_path, suffix)

            logger.info("Processing injection time validation plots...")
            injection_validation(val_path, suffix)

            logger.info("Processing momentum validation plots...")
            mom_validation(val_path, suffix)

            source_path = os.path.join(job_path, 'validation0', '0', 'algorithm_output', 'plots', 'costh')
            shutil.copy(source_path+f"/dedxpeaks_vs_cos_e{exp}_r{run}.pdf", 'plots/validation/')

            sou_path = os.path.join(job_path, 'validation0', '0', 'algorithm_output', 'plots', 'mom')
            shutil.copy(sou_path+f"/dedxpeaks_vs_mom_e{exp}_r{run}.pdf", 'plots/validation/')


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
