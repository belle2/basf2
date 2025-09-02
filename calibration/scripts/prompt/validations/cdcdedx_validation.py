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
from matplotlib.backends.backend_pdf import PdfPages
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

    pdf_path = os.path.join("plots", "validation", f"dedx_vs_run_{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)

    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(1, 2, figsize=(20, 6))

        # Mean plot
        ymin = df[df['mean'] > 0]['mean'].min()
        ymax = df['mean'].max()
        pc.hist(y_min=ymin-0.02, y_max=ymax+0.02, xlabel="Run range", ylabel="dE/dx mean", space=30, ax=ax[0])
        ax[0].errorbar(df['run'], df['mean'], yerr=df['mean_err'], fmt='*', markersize=8, rasterized=True, label='Bhabha mean')
        ax[0].legend(fontsize=12)
        ax[0].set_title('dE/dx Mean vs Run', fontsize=14)

        # Reso plot
        ymin = df[df['reso'] > 0]['reso'].min()
        ymax = df['reso'].max()
        pc.hist(y_min=ymin-0.01, y_max=ymax+0.01, xlabel="Run range", ylabel="dE/dx reso", space=30, ax=ax[1])
        ax[1].errorbar(df['run'], df['reso'], yerr=df['reso_err'], fmt='*', markersize=8, rasterized=True, label='Bhabha reso')
        ax[1].legend(fontsize=12)
        ax[1].set_title('dE/dx Resolution vs Run', fontsize=14)

        fig.suptitle("dE/dx vs Run", fontsize=20)
        plt.tight_layout()
        pdf.savefig(fig)
        plt.close()

    print(f"Saved combined PDF: {pdf_path}")


def wiregain_validation(path, suffix):
    pdf_path = os.path.join("plots", "validation", f"dedx_vs_wire_layer_{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)

    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(2, 2, figsize=(20, 12))

        # Wire plot
        try:
            val_path_gwire = os.path.join(path, "plots", "wire", f"dedx_mean_gwire_{suffix}.txt")
            df_gwire = pd.read_csv(val_path_gwire, sep=" ", header=None, names=["wire", "mean"])
            val_path_bwire = os.path.join(path, "plots", "wire", f"dedx_mean_badwire_{suffix}.txt")
            df_bwire = pd.read_csv(val_path_bwire, sep=" ", header=None, names=["wire", "mean"])
        except FileNotFoundError:
            print(f"File not found: {val_path_gwire}")
            return

        ymin = df_gwire['mean'].min()
        ymax = df_gwire['mean'].max()

        pc.hist(y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Wire", ylabel="dE/dx mean", space=1000, ax=ax[0, 0])
        ax[0, 0].plot(df_gwire['wire'], df_gwire['mean'], '*', markersize=5, rasterized=True)
        ax[0, 0].set_title('dE/dx Mean vs good Wire', fontsize=14)

        ymin = df_bwire['mean'].min()
        ymax = df_bwire['mean'].max()
        pc.hist(y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Wire", ylabel="dE/dx mean", space=1000, ax=ax[1, 0])
        ax[1, 0].plot(df_bwire['wire'], df_bwire['mean'], '*', markersize=5, rasterized=True)
        ax[1, 0].set_title('dE/dx Mean vs bad Wire', fontsize=14)

        # Layer plot
        try:
            val_path_layer = os.path.join(path, "plots", "wire", f"dedx_mean_layer_{suffix}.txt")
            df_layer = pd.read_csv(val_path_layer, sep=" ", header=None, names=["layer", "mean", "gmean"])
        except FileNotFoundError:
            print(f"File not found: {val_path_layer}")
            return

        ymin = df_layer['mean'].min()
        ymax = df_layer['mean'].max()

        pc.hist(x_min=0, x_max=56, y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Layer", ylabel="dE/dx mean", space=3, ax=ax[0, 1])
        ax[0, 1].plot(df_layer['layer'], df_layer['mean'], '*', markersize=10, rasterized=True)
        ax[0, 1].set_title('dE/dx Mean vs Layer', fontsize=14)

        ymin = df_layer['gmean'].min()
        ymax = df_layer['gmean'].max()
        pc.hist(x_min=0, x_max=56, y_min=ymin-0.02, y_max=ymax+0.02, xlabel="Layer", ylabel="dE/dx mean", space=3, ax=ax[1, 1])
        ax[1, 1].plot(df_layer['layer'], df_layer['gmean'], '*', markersize=10, rasterized=True)
        ax[1, 1].set_title('dE/dx Mean vs Layer (good wires)', fontsize=14)

        fig.suptitle("dE/dx vs #wire", fontsize=20)
        plt.tight_layout()
        pdf.savefig(fig)
        plt.close()

    print(f"Saved combined PDF: {pdf_path}")


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

    pdf_path = os.path.join("plots", "validation", f"dedx_vs_cosine_{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)

    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(1, 2, figsize=(20, 6))  # Two plots side-by-side
        # mean
        pc.hist(x_min=-1.0, x_max=1.0, y_min=0.96, y_max=1.03, xlabel=r"cos#theta", ylabel="dE/dx mean", space=0.1, ax=ax[0])
        ax[0].errorbar(
            df_el['cos'],
            df_el['mean'],
            yerr=df_el['mean_err'],
            fmt='*',
            markersize=10,
            rasterized=True,
            label='electron')
        ax[0].errorbar(
            df_po['cos'],
            df_po['mean'],
            yerr=df_po['mean_err'],
            fmt='*',
            markersize=10,
            rasterized=True,
            label='positrons')
        ax[0].errorbar(df_sum['cos'], df_sum['mean_sum'], yerr=df_sum['err_avg'], fmt='*',
                       markersize=10, rasterized=True, label=r'avergae of e^{+} and e^{-}')
        ax[0].legend(fontsize=17)
        ax[0].set_title('dE/dx Mean vs cosine', fontsize=14)

        # reso
        pc.hist(x_min=-1.0, x_max=1.0, y_min=0.04, y_max=0.13, xlabel=r"cos#theta", ylabel="dE/dx reso", space=0.1, ax=ax[1])
        ax[1].errorbar(
            df_el['cos'],
            df_el['reso'],
            yerr=df_el['reso_err'],
            fmt='*',
            markersize=10,
            rasterized=True,
            label='electron')
        ax[1].errorbar(
            df_po['cos'],
            df_po['reso'],
            yerr=df_po['reso_err'],
            fmt='*',
            markersize=10,
            rasterized=True,
            label='positrons')
        ax[1].legend(fontsize=17)
        ax[1].set_title('dE/dx Resolution vs cosine', fontsize=14)

        fig.suptitle(r"dE/dx vs cos$\theta$", fontsize=20)
        plt.tight_layout()
        pdf.savefig(fig)
        plt.close()


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

    pdf_path = os.path.join("plots", "validation", f"dedx_mean_inj_{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)

    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(1, 1, figsize=(20, 6))

        ymin = df_ler[df_ler['mean'] > 0]['mean'].min()
        ymax = df_ler['mean'].max()

        pc.hist(y_min=ymin-0.01, y_max=ymax+0.01, xlabel="injection time", ylabel="dE/dx mean", space=3, ax=ax)
        ax.errorbar(df_ler['bin'], df_ler['mean'], yerr=df_ler['mean_err'], fmt='*', markersize=10, rasterized=True, label='LER')
        ax.errorbar(df_her['bin'], df_her['mean'], yerr=df_her['mean_err'], fmt='*', markersize=10, rasterized=True, label='HER')
        ax.legend(fontsize=19)

        fig.suptitle("dE/dx vs Injection time", fontsize=20)
        plt.tight_layout()
        pdf.savefig(fig)
        plt.close()


def mom_validation(path, suffix, modes=("acos", "negCosth", "posCosth"), prefix=""):
    pdf_path = os.path.join("plots", "validation", f"dedx_vs_mom_{prefix}{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)

    with PdfPages(pdf_path) as pdf:
        for mode in modes:
            try:
                val_path_el = os.path.join(path, "plots", "mom", f"dedx_vs_mom_{prefix}{mode}_elec_{suffix}.txt")
                val_path_po = os.path.join(path, "plots", "mom", f"dedx_vs_mom_{prefix}{mode}_posi_{suffix}.txt")

                df_el = pd.read_csv(val_path_el, sep=" ", header=None,
                                    names=["mom", "mean", "mean_err", "reso", "reso_err"])
                df_po = pd.read_csv(val_path_po, sep=" ", header=None,
                                    names=["mom", "mean", "mean_err", "reso", "reso_err"])
            except FileNotFoundError:
                logger.error(f"Momentum data files not found for mode={mode} in {path}/plots/mom/")
                continue

            df_el['mom'] *= -1

            fig, ax = plt.subplots(2, 2, figsize=(20, 12))

            ymin = df_el[df_el['mean'] > 0]['mean'].min()
            ymax = df_el['mean'].max()

            panels = [
                {"xlim": (-7, 7), "ylim": (ymin-0.01, ymax+0.01),
                 "ylabel": "dE/dx mean", "df_col": "mean", "err_col": "mean_err",
                 "title": "dE/dx Mean vs momentum"},
                {"xlim": (-7, 7), "ylim": (0.04, 0.1),
                 "ylabel": "dE/dx reso", "df_col": "reso", "err_col": "reso_err",
                 "title": "dE/dx resolution vs momentum"},
                {"xlim": (-3, 3), "ylim": (ymin-0.01, ymax+0.01),
                 "ylabel": "dE/dx mean", "df_col": "mean", "err_col": "mean_err",
                 "title": "dE/dx Mean vs momentum (zoomed)"},
                {"xlim": (-3, 3), "ylim": (0.04, 0.1),
                 "ylabel": "dE/dx reso", "df_col": "reso", "err_col": "reso_err",
                 "title": "dE/dx resolution vs momentum (zoomed)"},
            ]

            for ax_i, panel in zip(ax.flat, panels):
                pc.hist(x_min=panel["xlim"][0], x_max=panel["xlim"][1],
                        y_min=panel["ylim"][0], y_max=panel["ylim"][1],
                        xlabel="Momentum", ylabel=panel["ylabel"],
                        space=1, ax=ax_i)

                ax_i.errorbar(df_el['mom'], df_el[panel["df_col"]],
                              yerr=df_el[panel["err_col"]],
                              fmt='*', markersize=10, rasterized=True, label='electron')
                ax_i.errorbar(df_po['mom'], df_po[panel["df_col"]],
                              yerr=df_po[panel["err_col"]],
                              fmt='*', markersize=10, rasterized=True, label='positron')
                ax_i.legend(fontsize=17)
                ax_i.set_title(panel["title"], fontsize=14)

            fig.suptitle(f"dE/dx vs Momentum ({prefix}{mode})", fontsize=20)
            plt.tight_layout()
            pdf.savefig(fig)
            plt.close()


def oneDcell_validation(path, suffix):
    try:
        val_path_il = os.path.join(path, "plots", "oneD", f"dedx_vs_1D_IL_{suffix}.txt")
        val_path_ol = os.path.join(path, "plots", "oneD", f"dedx_vs_1D_OL_{suffix}.txt")
        df_il = pd.read_csv(val_path_il, sep=" ", header=None, names=["enta", "mean"])
        df_ol = pd.read_csv(val_path_ol, sep=" ", header=None, names=["enta", "mean"])
    except FileNotFoundError:
        logger.error(f"1D data files not found in {path}/plots/oneD/")
        return

    pdf_path = os.path.join("plots", "validation", f"dedx_vs_enta_{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)
    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(2, 2, figsize=(20, 12))

        pc.hist(x_min=-1.5, x_max=1.5, y_min=0.9, y_max=1.07, xlabel=r"entaRS", ylabel="dE/dx mean", space=0.3, ax=ax[0, 0])
        ax[0, 0].plot(df_il['enta'], df_il['mean'], '-', markersize=10, rasterized=True, label='IL')
        ax[0, 0].legend(fontsize=17)
        ax[0, 0].set_title('dE/dx Mean vs entaRS (IL)', fontsize=14)

        pc.hist(x_min=-1.5, x_max=1.5, y_min=0.9, y_max=1.05, xlabel=r"entaRS", ylabel="dE/dx mean", space=0.3, ax=ax[0, 1])
        ax[0, 1].plot(df_ol['enta'], df_ol['mean'], '-', markersize=10, rasterized=True, label='OL')
        ax[0, 1].legend(fontsize=17)
        ax[0, 1].set_title('dE/dx Mean vs entaRS (OL)', fontsize=14)

        pc.hist(x_min=-0.2, x_max=0.2, y_min=0.9, y_max=1.07, xlabel=r"entaRS", ylabel="dE/dx mean", space=0.02, ax=ax[1, 0])
        ax[1, 0].plot(df_il['enta'], df_il['mean'], '-', markersize=10, rasterized=True, label='IL')
        ax[1, 0].legend(fontsize=17)
        ax[1, 0].set_title('dE/dx Mean vs entaRS (IL) zoom', fontsize=14)

        pc.hist(x_min=-0.2, x_max=0.2, y_min=0.9, y_max=1.05, xlabel=r"entaRS", ylabel="dE/dx mean", space=0.02, ax=ax[1, 1])
        ax[1, 1].plot(df_ol['enta'], df_ol['mean'], '-', markersize=10, rasterized=True, label='OL')
        ax[1, 1].legend(fontsize=17)
        ax[1, 1].set_title('dE/dx Mean vs entaRS (OL) zoom', fontsize=14)

        fig.suptitle("dE/dx vs entaRS", fontsize=20)
        plt.tight_layout()
        pdf.savefig(fig)
        plt.close()


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
    rg.getRunGain(gtpath, GT)

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

            logger.info("Processing momentum validation plots in cos bins...")
            mom_validation(val_path, suffix, prefix="inCos_")
            logger.info("Processing 1D validation plots...")
            oneDcell_validation(val_path, suffix)

            source_path = os.path.join(job_path, 'validation0', '0', 'algorithm_output', 'plots')
            shutil.copy(source_path+f"/costh/dedxpeaks_vs_cos_e{exp}_r{run}.pdf", 'plots/validation/')

            shutil.copy(source_path+f"/mom/dedxpeaks_vs_mom_e{exp}_r{run}.pdf", 'plots/validation/')


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
