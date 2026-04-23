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
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from matplotlib.backends.backend_pdf import PdfPages
import shutil
import basf2
import process_wiregain as pw
import process_cosgain as pc
import process_onedcell as oned
import process_rungain as rg

from prompt import ValidationSettings

settings = ValidationSettings(name="CDC dedx",
                              description=__doc__,
                              download_files=[],
                              expert_config={
                                "GT": "data_prompt_rel09",
                                  })


def save_to_pdf(pdf, fig):
    fig.tight_layout()
    pdf.savefig(fig)
    plt.close(fig)


def read_txt(filepath, columns, sep=r"\s+"):
    if not os.path.exists(filepath):
        basf2.B2ERROR(f"File not found: {filepath}")
        return None
    return pd.read_csv(filepath, sep=sep, header=None, names=columns)


def make_pdf_path(prefix, suffix):
    pdf_path = os.path.join("plots", "validation", f"{prefix}_{suffix}.pdf")
    os.makedirs(os.path.dirname(pdf_path), exist_ok=True)
    return pdf_path


def get_positive_minmax(series):
    positive = series[series > 0]
    ymin = positive.min() if not positive.empty else series.min()
    ymax = series.max()
    return ymin, ymax


def rungain_validation(path, suffix):
    val_path = os.path.join(path, "plots", "run", f"dedx_vs_run_{suffix}.txt")
    df = read_txt(val_path, ["run", "mean", "mean_err", "reso", "reso_err"])
    if df is None:
        return

    df['run'] = df['run'].astype(str)

    pdf_path = make_pdf_path("dedx_vs_run", suffix)

    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(1, 2, figsize=(20, 6))
        n = len(df)
        space = max(10, min(50, int(200 / max(n, 1))))

        # Mean plot
        ymin, ymax = get_positive_minmax(df['mean'])
        pc.hist(y_min=ymin-0.02, y_max=ymax+0.02, xlabel="Run range", ylabel="dE/dx mean", space=space, ax=ax[0])
        ax[0].errorbar(df['run'], df['mean'], yerr=df['mean_err'], fmt='*', markersize=8, rasterized=True, label='Bhabha mean')
        ax[0].legend(fontsize=12)
        ax[0].set_title('dE/dx Mean vs Run', fontsize=14)

        # Reso plot
        ymin, ymax = get_positive_minmax(df['reso'])
        pc.hist(y_min=ymin-0.01, y_max=ymax+0.01, xlabel="Run range", ylabel="dE/dx reso", space=space, ax=ax[1])
        ax[1].errorbar(df['run'], df['reso'], yerr=df['reso_err'], fmt='*', markersize=8, rasterized=True, label='Bhabha reso')
        ax[1].legend(fontsize=12)
        ax[1].set_title('dE/dx Resolution vs Run', fontsize=14)

        fig.suptitle("dE/dx vs Run", fontsize=20)
        save_to_pdf(pdf, fig)


def wiregain_validation(path, suffix):

    val_path_gwire = os.path.join(path, "plots", "wire", f"dedx_mean_gwire_{suffix}.txt")
    val_path_bwire = os.path.join(path, "plots", "wire", f"dedx_mean_badwire_{suffix}.txt")
    val_path_layer = os.path.join(path, "plots", "wire", f"dedx_mean_layer_{suffix}.txt")

    df_gwire = read_txt(val_path_gwire, ["wire", "mean"])
    df_bwire = read_txt(val_path_bwire, ["wire", "mean"])
    df_layer = read_txt(val_path_layer, ["layer", "mean", "gmean"])

    if df_gwire is None or df_bwire is None or df_layer is None:
        return

    pdf_path = make_pdf_path("dedx_vs_wire_layer", suffix)

    with PdfPages(pdf_path) as pdf:
        fig, ax = plt.subplots(2, 2, figsize=(20, 12))

        ymin, ymax = get_positive_minmax(df_gwire['mean'])

        pc.hist(y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Wire", ylabel="dE/dx mean", space=1000, ax=ax[0, 0])
        ax[0, 0].plot(df_gwire['wire'], df_gwire['mean'], '*', markersize=5, rasterized=True)
        ax[0, 0].set_title('dE/dx Mean vs good Wire', fontsize=14)

        ymin, ymax = get_positive_minmax(df_bwire['mean'])

        pc.hist(y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Wire", ylabel="dE/dx mean", space=1000, ax=ax[1, 0])
        ax[1, 0].plot(df_bwire['wire'], df_bwire['mean'], '*', markersize=5, rasterized=True)
        ax[1, 0].set_title('dE/dx Mean vs bad Wire', fontsize=14)

        ymin, ymax = get_positive_minmax(df_layer['mean'])

        pc.hist(x_min=0, x_max=56, y_min=ymin-0.05, y_max=ymax+0.05, xlabel="Layer", ylabel="dE/dx mean", space=3, ax=ax[0, 1])
        ax[0, 1].plot(df_layer['layer'], df_layer['mean'], '*', markersize=10, rasterized=True)
        ax[0, 1].set_title('dE/dx Mean vs Layer', fontsize=14)

        ymin, ymax = get_positive_minmax(df_layer['gmean'])
        pc.hist(x_min=0, x_max=56, y_min=ymin-0.02, y_max=ymax+0.02, xlabel="Layer", ylabel="dE/dx mean", space=3, ax=ax[1, 1])
        ax[1, 1].plot(df_layer['layer'], df_layer['gmean'], '*', markersize=10, rasterized=True)
        ax[1, 1].set_title('dE/dx Mean vs Layer (good wires)', fontsize=14)

        fig.suptitle(f"dE/dx vs #wire {suffix}", fontsize=20)
        save_to_pdf(pdf, fig)


def cosgain_validation(path, suffix):
    val_path_el = os.path.join(path, "plots", "costh", f"dedx_vs_cos_electrons_{suffix}.txt")
    val_path_po = os.path.join(path, "plots", "costh", f"dedx_vs_cos_positrons_{suffix}.txt")

    df_el = read_txt(val_path_el, ["cos", "mean", "mean_err", "reso", "reso_err"])
    df_po = read_txt(val_path_po, ["cos", "mean", "mean_err", "reso", "reso_err"])

    if df_el is None or df_po is None:
        return

    # Ensure both dataframes are sorted by 'cos' so addition is element-wise correct
    df_el = df_el.sort_values(by='cos').reset_index(drop=True)
    df_po = df_po.sort_values(by='cos').reset_index(drop=True)

    # New DataFrame with summed means
    mean_avg = (df_el['mean'] + df_po['mean']) / 2
    err_avg = 0.5 * np.sqrt(df_el['mean_err']**2 + df_po['mean_err']**2)
    df_sum = pd.DataFrame({'cos': df_el['cos'],  'mean_sum': mean_avg, 'err_avg': err_avg})

    pdf_path = make_pdf_path("dedx_vs_cosine", suffix)

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
                       markersize=10, rasterized=True, label=r'average of e^{+} and e^{-}')
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

        fig.suptitle(fr"dE/dx vs cos$\theta$ {suffix}", fontsize=20)
        save_to_pdf(pdf, fig)


def injection_validation(path, suffix):

    cols = ["var", "bin", "mean", "mean_err", "reso", "reso_err"]
    # corrected files
    val_path_ler = os.path.join(path, "plots", "injection", f"dedx_vs_inj_ler_{suffix}.txt")
    val_path_her = os.path.join(path, "plots", "injection", f"dedx_vs_inj_her_{suffix}.txt")

    df_ler = read_txt(val_path_ler, cols)
    df_her = read_txt(val_path_her, cols)

    # no-correction files
    val_path_ler_nocor = os.path.join(path, "plots", "injection", f"dedx_vs_inj_nocor_ler_{suffix}.txt")
    val_path_her_nocor = os.path.join(path, "plots", "injection", f"dedx_vs_inj_nocor_her_{suffix}.txt")

    df_ler_nocor = read_txt(val_path_ler_nocor, cols)
    df_her_nocor = read_txt(val_path_her_nocor, cols)

    if df_ler is None or df_her is None or df_ler_nocor is None or df_her_nocor is None:
        return

    for df in [df_ler, df_her, df_ler_nocor, df_her_nocor]:
        df["bin"] = df["bin"].astype(str)

    pdf_path = make_pdf_path("dedx_mean_inj", suffix)

    with PdfPages(pdf_path) as pdf:

        fig, ax = plt.subplots(2, 1, figsize=(18, 10), sharex=True)

        ymin, ymax = get_positive_minmax(
            pd.concat([df_ler["mean"], df_her["mean"]])
        )

        pc.hist(y_min=ymin - 0.01, y_max=ymax + 0.01,
                xlabel="", ylabel="dE/dx mean",
                space=3, ax=ax[0])

        ax[0].errorbar(df_ler['bin'], df_ler['mean'], yerr=df_ler['mean_err'],
                       fmt='*', markersize=10, rasterized=True, label='LER')
        ax[0].errorbar(df_her['bin'], df_her['mean'], yerr=df_her['mean_err'],
                       fmt='*', markersize=10, rasterized=True, label='HER')

        ax[0].legend(fontsize=14)
        ax[0].set_title("Corrected", fontsize=16)

        all_means = pd.concat([
            df_ler["mean"], df_her["mean"],
            df_ler_nocor["mean"], df_her_nocor["mean"]
        ])

        positive_means = all_means[all_means > 0]
        ymin2 = positive_means.min() if not positive_means.empty else all_means.min()
        ymax2 = all_means.max()

        pc.hist(y_min=ymin2 - 0.01, y_max=ymax2 + 0.01,
                xlabel="injection time", ylabel="dE/dx mean",
                space=3, ax=ax[1])

        datasets = [
            ("LER corr",    df_ler,       "o"),
            ("HER corr",    df_her,       "s"),
            ("LER no corr", df_ler_nocor, "^"),
            ("HER no corr", df_her_nocor, "D"),
        ]

        for label, df, marker in datasets:
            ax[1].errorbar(df['bin'], df['mean'], yerr=df['mean_err'],
                           fmt=marker, markersize=6, rasterized=True, label=label)

        ax[1].legend(fontsize=12)
        ax[1].set_title("Corrected vs No correction", fontsize=16)

        fig.suptitle(f"dE/dx vs Injection time {suffix}", fontsize=20)

        plt.tight_layout(rect=[0, 0, 1, 0.96])  # important to avoid overlap
        save_to_pdf(pdf, fig)


def mom_validation(path, suffix):

    cos_labels = [
        "acos",
        "cos$\\theta > 0.0$",
        "cos$\\theta < 0.0$",
        "cos$\\theta \\leq -0.8$",
        "cos$\\theta > -0.8$ and $\\cos\\theta \\leq -0.6$",
        "cos$\\theta > -0.6$ and $\\cos\\theta \\leq -0.4$",
        "cos$\\theta > -0.4$ and $\\cos\\theta \\leq -0.2$",
        "cos$\\theta > -0.2$ and $\\cos\\theta \\leq 0$",
        "cos$\\theta > 0$ and $\\cos\\theta \\leq 0.2$",
        "cos$\\theta > 0.2$ and $\\cos\\theta \\leq 0.4$",
        "cos$\\theta > 0.4$ and $\\cos\\theta \\leq 0.6$",
        "cos$\\theta > 0.6$ and $\\cos\\theta \\leq 0.8$",
        "cos$\\theta > 0.8$"
    ]

    # Define output PDFs
    pdf_paths = {
        "low": make_pdf_path("dedx_vs_mom", suffix),
        "high": make_pdf_path("dedx_vs_mom", f"{suffix}_cosbins"),
    }

    with PdfPages(pdf_paths["low"]) as pdf_low, PdfPages(pdf_paths["high"]) as pdf_high:
        for i in range(13):
            cols = ["mom", "mean", "mean_err", "reso", "reso_err"]
            val_path_el = os.path.join(path, "plots", "mom", f"dedx_vs_mom_{i}_elec_{suffix}.txt")
            val_path_po = os.path.join(path, "plots", "mom", f"dedx_vs_mom_{i}_posi_{suffix}.txt")

            df_el = read_txt(val_path_el, cols)
            df_po = read_txt(val_path_po, cols)

            if df_el is None or df_po is None:
                continue

            df_el['mom'] *= -1  # flip electron momentum

            fig, ax = plt.subplots(2, 2, figsize=(20, 12))

            ymin, ymax = get_positive_minmax(df_el['mean'])

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
                if i == 3 and panel["df_col"] == "reso":
                    ymin, ymax = ax_i.get_ylim()
                    ax_i.set_ylim(ymin, ymax * 1.5)

            fig.suptitle(f"dE/dx vs Momentum ({cos_labels[i]}) {suffix}", fontsize=20)
            plt.tight_layout()

            # Save to correct PDF
            if i <= 2:
                save_to_pdf(pdf_low, fig)
            else:
                save_to_pdf(pdf_high, fig)


def oneDcell_validation(path, suffix):

    val_path_il = os.path.join(path, "plots", "oneD", f"dedx_vs_1D_IL_{suffix}.txt")
    val_path_ol = os.path.join(path, "plots", "oneD", f"dedx_vs_1D_OL_{suffix}.txt")

    df_il = read_txt(val_path_il, ["enta", "mean"])
    df_ol = read_txt(val_path_ol, ["enta", "mean"])

    if df_il is None or df_ol is None:
        return

    pdf_path = make_pdf_path("dedx_vs_enta", suffix)

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

        fig.suptitle(f"dE/dx vs entaRS {suffix}", fontsize=20)
        save_to_pdf(pdf, fig)


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    '''
    Makes validation plots
    :job_path: path to cdcdedx calibration output
    :input_data_path: path to the input files
    :requested_iov: required argument but not used
    :expert_config: required argument
    '''
    os.makedirs('plots/validation', exist_ok=True)
    os.makedirs('plots/constant', exist_ok=True)

    expert_config = json.loads(expert_config)
    GT = expert_config["GT"]

    basf2.B2INFO("Starting validation...")

    basf2.B2INFO("Processing run gain payloads...")
    gtpath = os.path.join(job_path, 'rungain2', 'outputdb')
    rg.getRunGain(gtpath, GT)

    basf2.B2INFO("Processing coscorr payloads...")
    ccpath = os.path.join(job_path, 'coscorr1', 'outputdb')
    pc.process_cosgain(ccpath, GT)

    basf2.B2INFO("Processing wire gain payloads...")
    wgpath = os.path.join(job_path, 'wiregain0', 'outputdb')
    exp_run_dict = pw.process_wiregain(wgpath, GT)

    basf2.B2INFO("Processing 1D gain payloads...")
    onedpath = os.path.join(job_path, 'onedcell0', 'outputdb')
    oned.process_onedgain(onedpath, GT)

    basf2.B2INFO("Generating validation plots...")
    val_path = os.path.join(job_path, 'validation0', '0', 'algorithm_output')

    validators = [
        ("rungain validation plots", rungain_validation),
        ("wire gain validation plots", wiregain_validation),
        ("cosine correction validation plots", cosgain_validation),
        ("injection time validation plots", injection_validation),
        ("momentum validation plots", mom_validation),
        ("1D validation plots", oneDcell_validation),
    ]

    for exp, run_list in exp_run_dict.items():
        for run in run_list:
            suffix = f"e{exp}_r{run}"
            for msg, func in validators:
                basf2.B2INFO(f"Processing {msg} for {suffix}...")
                func(val_path, suffix)

            source_path = os.path.join(job_path, 'validation0', '0', 'algorithm_output', 'plots')
            shutil.copy(source_path+f"/costh/dedxpeaks_vs_cos_{suffix}.pdf", 'plots/validation/')

            shutil.copy(source_path+f"/mom/dedxpeaks_vs_mom_{suffix}.pdf", 'plots/validation/')


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
