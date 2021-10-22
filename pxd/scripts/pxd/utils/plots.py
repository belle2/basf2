#!/usr/bin/python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""\
Utilities for plotting
"""
from pxd.utils import getH1Sigma68WithError
import basf2 as b2
import ROOT
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


def root_style():
    """
    ROOT style
    Author: maiko.takahashi@desy.de
    """
    # root style option

    # canvas
    ROOT.gStyle.SetCanvasBorderMode(0)
    ROOT.gStyle.SetCanvasBorderSize(10)
    ROOT.gStyle.SetCanvasColor(0)
    ROOT.gStyle.SetCanvasDefH(450)
    ROOT.gStyle.SetCanvasDefW(500)
    ROOT.gStyle.SetCanvasDefX(10)
    ROOT.gStyle.SetCanvasDefY(10)
    # pad
    ROOT.gStyle.SetPadBorderMode(0)
    ROOT.gStyle.SetPadBorderSize(10)
    ROOT.gStyle.SetPadColor(0)
    ROOT.gStyle.SetPadBottomMargin(0.16)
    ROOT.gStyle.SetPadTopMargin(0.10)
    ROOT.gStyle.SetPadLeftMargin(0.17)
    ROOT.gStyle.SetPadRightMargin(0.19)
    ROOT.gStyle.SetPadGridX(1)
    ROOT.gStyle.SetPadGridY(1)
    ROOT.gStyle.SetGridStyle(2)
    ROOT.gStyle.SetGridColor(ROOT.kGray + 1)
    # frame
    ROOT.gStyle.SetFrameFillStyle(0)
    ROOT.gStyle.SetFrameFillColor(0)
    ROOT.gStyle.SetFrameLineColor(1)
    ROOT.gStyle.SetFrameLineStyle(0)
    ROOT.gStyle.SetFrameLineWidth(2)
    ROOT.gStyle.SetFrameBorderMode(0)
    ROOT.gStyle.SetFrameBorderSize(10)
    # axis
    ROOT.gStyle.SetLabelFont(42, "xyz")
    ROOT.gStyle.SetLabelOffset(0.015, "xyz")
    ROOT.gStyle.SetLabelSize(0.048, "xyz")
    ROOT.gStyle.SetNdivisions(505, "xyz")
    ROOT.gStyle.SetTitleFont(42, "xyz")
    ROOT.gStyle.SetTitleSize(0.050, "xyz")
    ROOT.gStyle.SetTitleOffset(1.3, "x")
    ROOT.gStyle.SetTitleOffset(1.2, "y")
    ROOT.gStyle.SetTitleOffset(1.4, "z")
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)
    # title
    ROOT.gStyle.SetTitleBorderSize(0)
    ROOT.gStyle.SetTitleFillColor(10)
    ROOT.gStyle.SetTitleAlign(12)
    ROOT.gStyle.SetTitleFontSize(0.045)
    ROOT.gStyle.SetTitleX(0.560)
    ROOT.gStyle.SetTitleY(0.860)
    ROOT.gStyle.SetTitleFont(42, "")
    # stat
    ROOT.gStyle.SetStatBorderSize(0)
    ROOT.gStyle.SetStatColor(10)
    ROOT.gStyle.SetStatFont(42)
    ROOT.gStyle.SetStatX(0.94)
    ROOT.gStyle.SetStatY(0.91)
    # histo style
    ROOT.gStyle.SetOptTitle(0)
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetHistLineWidth(3)
    ROOT.TH2.SetDefaultSumw2()


def df_plot_errorbar(df, x, y, yerr_low, yerr_up, ax=None, *args, **kwargs):
    """
    errorbar extention for pandas.DataFrame
    Parameters:
      df: pandas.DataFrame
      x (str): column used as the x-axis
      y (str): column used as the y-axis
      yerr_low (str): column of the lower error of value y
      yerr_up (str): column of the upper error of value y

      ax: matplotlib.pyplot.axis
      args: positional arguments
      kwargs: named arguments
    """
    if ax is None:
        ax = plt.gca()
    array_x = df[x].to_numpy()
    array_y = df[y].to_numpy()
    yerr = [df[yerr_low].to_numpy(), df[yerr_up].to_numpy()]
    default_kwargs = {"fmt": "o", "label": y, "alpha": 0.7}
    kwargs = {**default_kwargs, **kwargs}

    ax.errorbar(x=array_x, y=array_y, yerr=yerr, *args, **kwargs)
    ax.legend()
    ax.set_xlabel(x)
    ax.set_ylabel(y)


# Extend pandas.DataFrame
if hasattr(pd.DataFrame, "errorbar"):
    b2.B2WARNING("DataFrame.errorbar has already been defined. Replacing it!")
pd.DataFrame.errorbar = df_plot_errorbar


def plot_ip_resolutions(tree, cut="0<z0_err<1 & 0<d0_err<1", figsize=(16, 6), save_to=""):
    """
    Helper function to plot ip resolutions as a function of run number.
    Parameters:
      tree: a ROOT.TTree which contains run, hD0, hZ0 from PXD calibration validation.
      figsize (tuple): figsize for matplotlib pyplot
      save_to (str): Target file name to save a figure when it's not empty.
    Return:
      pandas.DataFrame with resolutions
    """
    plt.figure(figsize=figsize)
    ip_res_dic = {"run": [], "d0": [], "d0_err": [], "z0": [], "z0_err": []}
    for i in range(tree.GetEntries()):
        tree.GetEntry(i)
        # exp = getattr(tree, "exp")
        run = getattr(tree, "run")
        hD0 = getattr(tree, "hD0")
        hZ0 = getattr(tree, "hZ0")
        res_d0, res_d0_err = np.array(getH1Sigma68WithError(hD0, 1000)) / 2 * 1e4  # cm to um
        res_z0, res_z0_err = np.array(getH1Sigma68WithError(hZ0, 1000)) / 2 * 1e4  # cm to um
        #  print(run,res_d0, res_z0)
        ip_res_dic["run"].append(run)
        ip_res_dic["d0"].append(res_d0)
        ip_res_dic["d0_err"].append(res_d0_err)
        ip_res_dic["z0"].append(res_z0)
        ip_res_dic["z0_err"].append(res_z0_err)
    df_res = pd.DataFrame(ip_res_dic)
    df_res.query(cut).errorbar(y="z0", x="run", yerr_low="z0_err", yerr_up="z0_err")
    df_res.query(cut).errorbar(y="d0", x="run", yerr_low="d0_err", yerr_up="d0_err")
    plt.ylabel(r"$\sigma_{68}$ " + r"[$\mathrm{\mu m}$]")
    plt.savefig(save_to)
    return df_res


def plot_efficiency_map(tree, exp_runs=[], num_name="hTrackClustersLayer1", den_name="hTrackPointsLayer1",
                        title="Efficiency of layer 1", canvas=None, save_to=""):
    """
    Helper function to plot efficiency map of modules for a given layer.
    Color map can be set by ROOT.gStyle.SetPalette(ROOT.kDarkRainBow)
    Parameters:
      tree: ROOT.TTree
      exp_runs (list): list of (experiment_number, run_number) or list of run_number
      num_name (str): histogram name of the ROOT.TH2 used for numerator
      den_name (str): histogram name of the ROOT.TH2 used for denominator
      title (str): title of the plot
      canvas (ROOT.TCanvas): TCanvas for plotting
      save_to (str): File name to save the figure
    Return:
      the efficiency map (ROOT.TH2)
    """
    if canvas is None:
        canvas = ROOT.TCanvas("c1", "c1", 900, 600)
    use_exp_run_tuple = True
    if not exp_runs:
        return
    elif not isinstance(exp_runs[0], tuple):
        exp_runs = sorted(exp_runs)
        use_exp_run_tuple = False
    count = 0
    h_num, h_den = None, None
    for i_evt in range(tree.GetEntries()):
        tree.GetEntry(i_evt)
        exp = getattr(tree, "exp")
        run = getattr(tree, "run")
        if not use_exp_run_tuple:
            if run < exp_runs[0] or run > exp_runs[-1]:
                continue
        elif (exp, run) not in exp_runs:
            continue
        if count == 0:
            h_num = getattr(tree, num_name).Clone()
            h_den = getattr(tree, den_name).Clone()
        else:
            h_num.Add(getattr(tree, num_name))
            h_den.Add(getattr(tree, den_name))
        count += 1
    if count:
        h_num.Divide(h_den)
        h_num.SetTitle(title)
        h_num.SetStats(False)
        h_num.Draw("colz")
        canvas.Draw()
        if save_to:
            canvas.Print(save_to)
    return h_num


def plot_in_module_efficiency(df, pxdid=1052, figsize=(12, 16), alpha=0.7, save_to="",
                              y="eff_sel", x="run", cut="eff_sel>0&eff_sel_err_low<0.01",
                              yerr_low="eff_sel_err_low", yerr_up="eff_sel_err_up"):
    """
    Helper function to plot efficiencies of 4 x 6 ASIC regions in one module
    Parameters:
      df (pandas.DataFrame): pandas DataFrame for plotting
      pxdid (int): pxdid, e.g., 1052
      figsize (tuple): figsize for matplotlib pyplot
      alpha (float): alpha color
      save_to (str): File name to save the figure
      y (str): column of the efficiency
      x (str): column of the run
      cut (str): additional cut
      yerr_low (str): column of the lower error
      yerr_up (str): column of the upper error
    """

    fig, axes = plt.subplots(4, 1, sharex=False, sharey=True, figsize=figsize)
    for uBin in range(4):
        ax = axes[uBin]
        for vBin in range(6):
            df.query(cut + "&" + f"pxdid=={pxdid}&uBin=={uBin}&vBin=={vBin}").errorbar(
                y=y, x=x, yerr_low=yerr_low, yerr_up=yerr_up, ax=ax, label=f"{pxdid},u{uBin}v{vBin}", alpha=0.7)
        ax.legend(bbox_to_anchor=(1, 1), loc="upper left")
        ymin, ymax = ax.get_ylim()
        ax.set_ylim(ymin, 1.)
    if save_to:
        fig.savefig(save_to, bbox_inches="tight")


def plot_efficiency_vs_run(
        df,
        eff_var="eff",
        eff_sel_var="eff_sel",
        max_err=0.01,
        figsize=(
            12,
            6),
        save_to="pxd_efficiency_vs_run.png"):
    """
    Helper function to plot efficiency vs run
    Parameters:
      df (pandas.DataFrame): pandas DataFrame for plotting
      eff_var (str): column of the efficiency
      eff_sel_var (str): column of the efficiency of selected regions
      max_err (float): the maximum error used to clean up points with large errors
      figsize (tuple): figsize for matplotlib pyplot
      save_to (str): file name to save the figure
    """

    plt.figure(figsize=figsize)
    df.query(f"{eff_var}_err_low<{max_err}").errorbar(
        y=eff_var,
        x="run",
        yerr_low=f"{eff_var}_err_low",
        yerr_up=f"{eff_var}_err_up",
        alpha=0.7,
        label="All regions")
    df.query(f"{eff_sel_var}_err_low<{max_err}").errorbar(
        y=eff_sel_var,
        x="run",
        yerr_low=f"{eff_sel_var}_err_low",
        yerr_up=f"{eff_sel_var}_err_up",
        alpha=0.7,
        label="Excluding hot/dead regions")
    plt.ylabel("PXD efficiency")
    ymin, ymax = plt.ylim()
    plt.ylim(ymin, 1.0)
    if save_to:
        plt.savefig(save_to)


def plot_module_efficiencies_in_DHHs(df, eff_var="eff_sel", phase="early_phase3", figsize=(12, 6), save_to="efficiency_vs_run.png"):
    """
    Helper function to plot efficiencies of modules in a DHH.
    Parameters:
      df (pandas.DataFrame): pandas DataFrame for plotting
      eff_var (str): column of the efficiency
      phase (str): belle2 phase, the default is earyly_phase3 with half PXD
      figsize (tuple): figsize for matplotlib pyplot
      save_to (str): file name to save the figure
    """

    dhh_modules_dic = {}
    if phase == "early_phase3":
        dhh_modules_dic = {
            "H30": [1012, 1022, 1052, 1082, 2042],
            "H40": [1042, 1062, 1072, 2052],
            "H50": [1041, 1051, 1061, 1071, 2051],
            "H60": [1011, 1021, 1031, 1081, 2041],
        }
    for dhh, pxdid_list in dhh_modules_dic.items():
        plt.figure(figsize=figsize)
        for pxdid in pxdid_list:
            df.query(f"{eff_var}>0&pxdid=={pxdid}&{eff_var}_err_low<0.01").errorbar(
                y=eff_var, x="run", yerr_low=f"{eff_var}_err_low", yerr_up=f"{eff_var}_err_up", label=f"{pxdid}", alpha=0.7)
            plt.title(dhh + " efficiency")
            ymin, ymax = plt.ylim()
            plt.ylim(ymin, 1.0)
            if save_to:
                plt.savefig(dhh + "_" + save_to)


if __name__ == '__main__':
    root_style()
