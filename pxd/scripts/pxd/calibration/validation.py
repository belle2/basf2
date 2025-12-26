#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""\
This module contains classes for plotting validation results.
"""
from pxd.utils.plots import plot_in_module_efficiency, plot_efficiency_vs_run, \
                            plot_module_efficiencies_in_DHHs, plot_efficiency_map, plot_ip_resolutions
import uproot
import ROOT
import numpy as np


def analyse_root_file(file_name, tree_name="tree"):
    """
    Function to analyse the ROOT file generated from validation algorithm
    Parameters:
      file_name (str): name of the ROOT file
      tree_name (str): name of the TTree
    """
    # Get TTree
    tfile = ROOT.TFile.Open(file_name)
    tree = tfile.Get(tree_name)

    # Read DataFrame - MINIMAL FIX: Flatten data before passing to plotting functions
    branches = ['exp', 'run', 'pxdid', 'uBin', 'vBin', 'nTrackPoints', 'nSelTrackPoints', 'nSelTrackClusters', 'nTrackClusters']

    # Try different tree names
    if tree_name == "tree":
        try:
            df = uproot.open(file_name)["tree;3"].arrays(branches, library='pd')
        except BaseException:
            df = uproot.open(file_name)["tree;2"].arrays(branches, library='pd')
    else:
        df = uproot.open(file_name)[tree_name].arrays(branches, library='pd')

    # MINIMAL FIX: Check if df has Awkward Arrays and flatten them
    if len(df) > 0:
        # Check if we have Awkward Extension Arrays
        import awkward as ak
        import pandas as pd

        # If columns contain jagged arrays, flatten them
        if any(hasattr(df[col].iloc[0], '__len__') for col in df.columns if col not in ['exp', 'run']):
            print("DEBUG: Flattening jagged arrays...")

            # Extract lengths from pxdid (assumed jagged)
            jagged_lengths = ak.num(df['pxdid'].values)

            # Create flat arrays
            flat_data = {}

            # Handle scalar columns (exp, run)
            for col in ['exp', 'run']:
                if col in df.columns:
                    scalar_vals = ak.to_numpy(df[col].values)
                    flat_data[col] = np.repeat(scalar_vals, ak.to_numpy(jagged_lengths))

            # Handle jagged columns
            for col in df.columns:
                if col not in ['exp', 'run']:
                    try:
                        flat_data[col] = ak.to_numpy(ak.flatten(df[col].values))
                    except BaseException:
                        flat_data[col] = df[col].values

            # Create new DataFrame
            df = pd.DataFrame(flat_data)

    experiment_number = df.exp.min()
    run_range = (df.run.min(), df.run.max())
    prefix = f"e{experiment_number:05}_r{run_range[0]:04}-{run_range[1]:04}_"

    # Calculate efficiencies for all regions
    df.calculate_eff(num="nTrackClusters", den="nTrackPoints", output_var="eff")
    df.calculate_eff(num="nSelTrackClusters", den="nSelTrackPoints", output_var="eff_sel")

    plot_in_module_efficiency(
        df=df,
        pxdid=1052,
        y="eff_sel",
        x="run",
        cut="eff_sel>0&eff_sel_err_low<0.01",
        yerr_low="eff_sel_err_low",
        yerr_up="eff_sel_err_up",
        save_to=prefix + "efficiency_in_selected_regions_H1052.png")

    # Calculate efficiency of the entire PXD
    df_sum_all = df.groupby(['exp', 'run'])[["nTrackPoints", "nSelTrackPoints",
                                             "nTrackClusters", "nSelTrackClusters"]].sum().reset_index()
    df_sum_all.calculate_eff(num="nTrackClusters", den="nTrackPoints", output_var="eff")
    df_sum_all.calculate_eff(num="nSelTrackClusters", den="nSelTrackPoints", output_var="eff_sel")

    plot_efficiency_vs_run(df=df_sum_all, max_err=0.01, save_to=prefix + "pxd_efficiency_vs_run.png")

    # Calculate efficiencies of modules
    df_module_sum = df.groupby(['exp', 'run', 'pxdid'])[["nTrackPoints", "nSelTrackPoints",
                                                         "nTrackClusters", "nSelTrackClusters"]].sum().reset_index()
    # FIX: Replace deprecated np.int with int
    df_module_sum["pxdid"] = df_module_sum["pxdid"].astype(int)
    df_module_sum.calculate_eff(num="nTrackClusters", den="nTrackPoints", output_var="eff")
    df_module_sum.calculate_eff(num="nSelTrackClusters", den="nSelTrackPoints", output_var="eff_sel")

    plot_module_efficiencies_in_DHHs(df=df_module_sum, save_to=prefix + "efficiency_vs_run.png")

    # 2d efficiency map
    c1 = ROOT.TCanvas("c1", "c1", 900, 600)
    ROOT.gStyle.SetPalette(ROOT.kDarkRainBow)
    plot_efficiency_map(
        tree=tree,
        exp_runs=[0, 9999],
        num_name="hTrackClustersLayer1",
        den_name="hTrackPointsLayer1",
        save_to=prefix + "efficiency_l1.png",
        canvas=c1)
    plot_efficiency_map(
        tree=tree,
        exp_runs=[0, 9999],
        num_name="hTrackClustersLayer2",
        den_name="hTrackPointsLayer2",
        title="Efficiency of layer 2",
        save_to=prefix + "efficiency_l2.png",
        canvas=c1)
    # ip resolutions
    plot_ip_resolutions(tree=tree, save_to=prefix + "pxd_resolutions_vs_run.png")


if __name__ == "__main__":
    import sys
    file_name = sys.argv[1]
    ROOT.gROOT.SetBatch(True)
    analyse_root_file(file_name)
