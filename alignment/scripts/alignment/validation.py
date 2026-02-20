#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os
import ROOT
import re
import argparse
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit

import alignment.fit_dimuon_mass as fit_dimuon_mass
import glob

#: log level
log_level = 0
#: output file format for images
file_format = "pdf"
#: default output directory for plots
output_dirname = "validation_plots"

# Utility functions


class Unit:
    """Pair of display/residual unit strings and the conversion factor between them."""

    def __init__(self, name, dname, multiplier):
        """Store display unit, residual unit, and the conversion factor between them."""
        #: Unit name shown in plots.
        self.name = name
        #: Unit name used for residual axes.
        self.dname = dname
        #: Conversion factor from name to dname.
        self.convert = multiplier


#: second
s = Unit(" [s]", "s", 1)
#: centimeter and micrometer
cm = Unit(" [cm]", r" [$\mu$m]", 1e4)
#: radian and milliradian
rad = Unit(" [rad]", " [mrad]", 1e3)
#: scalars
unit = Unit(" [1]", r" [$10^{-3}$]", 1e3)
#: for curvature
inverse_cm = Unit(" [1/cm]", r" [1/cm $\cdot 10^{-4}$]", 1e4)
#: GeV
gev = Unit(" [GeV/c]", r" [GeV/c]", 1)


class GlobalVariable:
    """Metadata for a scalar observable stored in a single ROOT branch per event."""

    def __init__(self, name, latex, unit, plaintext):
        """Store metadata for a scalar variable (single branch per event)."""
        #: Branch name in the ROOT file.
        self.name = name
        #: LaTeX label used on plot axes.
        self.latex = latex
        #: Physical unit of the variable.
        self.unit = unit
        #: Plain-text name used in filenames and console output.
        self.plaintext = plaintext

    def getName(self):
        """Return the branch name as a single-element list."""
        return [self.name]


class TrackVariable:
    """Metadata for an observable with one ROOT branch per track (e.g. for cosmic two-track events)."""

    def __init__(self, name1, name2, latex, unit, plaintext):
        """Store metadata for a two-track variable (one branch per track)."""
        #: Branch name for the first track in the ROOT file.
        self.name1 = name1
        #: Branch name for the second track in the ROOT file.
        self.name2 = name2
        #: LaTeX label used on plot axes.
        self.latex = latex
        #: Physical unit of the variable.
        self.unit = unit
        #: Plain-text name used in filenames and console output.
        self.plaintext = plaintext

    def getName(self):
        """Return branch names for both tracks as a two-element list."""
        return [self.name1, self.name2]


def load_cosmic_data(filenames: list, selection: str):
    """Define cosmic-track variables, read ROOT files, and return filtered data arrays."""
    print("Loading cosmic data.")

    global run, time, d, z, phi, tanLambda, omega, pt

    run = GlobalVariable("run", "run", unit, "run")
    time = GlobalVariable("evtT0", r"t$_{0}$", s, "time")
    d = TrackVariable("D01", "D02", r"d$_{0}$", cm, "d")
    z = TrackVariable("Z01", "Z02", r"z$_{0}$", cm, "z")
    phi = TrackVariable("Phi01", "Phi02", r"$\Phi_{0}$", rad, "phi")
    tanLambda = TrackVariable(
        "tanLambda1",
        "tanLambda2",
        r"$\tan(\lambda$)",
        unit,
        "tanLambda")
    omega = TrackVariable("Omega1", "Omega2", r"$\omega$", inverse_cm, "omega")
    pt = TrackVariable("Pt1", "Pt2", r"$P_{t}$", gev, "pt")

    treename = "tree"

    variables_to_load = [run, time, d, z, phi, tanLambda, omega, pt]
    data = {}
    for file in filenames:
        print(f"Reading {file}")
        tfile = ROOT.TFile(file, "OPEN")
        dataframe = ROOT.RDataFrame(treename, tfile)
        filtered_dataframe = dataframe.Filter(selection)
        data[file] = filtered_dataframe.AsNumpy(
            columns=get_variable_names(variables_to_load))
        print(
            f"Number of events after applying selection in {file[:-5]}"
            f" is: {len(data[file][get_variable_names(variables_to_load)[0]])}")
    return data


def load_dimuon_data(filenames: list, selection: str, frame: str):
    """Define dimuon variables for the requested reference frame, read ROOT files, and return filtered data arrays."""
    print("Loading dimuon data.")

    global run, time, d, z, phi, tanLambda, omega, pt, InvM

    run = GlobalVariable("__run__", "run", unit, "run")
    time = GlobalVariable("eventTimeSeconds", r"t$_{0}$", s, "time")

    if frame == "lab":
        # variables for ee -> mumu events
        d = TrackVariable(
            "daughter__bo0__cm__spd0__bc",
            "daughter__bo1__cm__spd0__bc",
            r"d$_{0}$",
            cm,
            "d")
        z = TrackVariable(
            "daughter__bo0__cm__spz0__bc",
            "daughter__bo1__cm__spz0__bc",
            r"z$_{0}$",
            cm,
            "z")
        phi = TrackVariable(
            "daughter__bo0__cm__spphi0__bc",
            "daughter__bo1__cm__spphi0__bc",
            r"$\Phi_{0}$",
            rad,
            "phi")
        tanLambda = TrackVariable(
            "daughter__bo0__cm__sptanLambda__bc",
            "daughter__bo1__cm__sptanLambda__bc",
            r"tan($\lambda$)",
            unit,
            "tanLambda")
        omega = TrackVariable(
            "daughter__bo0__cm__spomega__bc",
            "daughter__bo1__cm__spomega__bc",
            r"$\omega$",
            inverse_cm,
            "omega")
        pt = TrackVariable(
            "daughter__bo0__cm__sppt__bc",
            "daughter__bo1__cm__sppt__bc",
            r"$P_{t}$",
            gev,
            "pt")
    elif frame == "cms":
        # variables for ee -> mumu eventsa - CMS frame
        d = TrackVariable(
            "useCMSFrame__bodaughter__bo0__cm__spd0__bc__bc",
            "useCMSFrame__bodaughter__bo1__cm__spd0__bc__bc",
            r"d$_{0}$",
            cm,
            "d")
        z = TrackVariable(
            "useCMSFrame__bodaughter__bo0__cm__spz0__bc__bc",
            "useCMSFrame__bodaughter__bo1__cm__spz0__bc__bc",
            r"z$_{0}$",
            cm,
            "z")
        phi = TrackVariable(
            "useCMSFrame__bodaughter__bo0__cm__spphi0__bc__bc",
            "useCMSFrame__bodaughter__bo1__cm__spphi0__bc__bc",
            r"$\Phi_{0}$",
            rad,
            "phi")
        tanLambda = TrackVariable(
            "useCMSFrame__bodaughter__bo0__cm__sptanLambda__bc__bc",
            "useCMSFrame__bodaughter__bo1__cm__sptanLambda__bc__bc",
            r"tan($\lambda$)",
            unit,
            "tanLambda")
        omega = TrackVariable(
            "useCMSFrame__bodaughter__bo0__cm__spomega__bc__bc",
            "useCMSFrame__bodaughter__bo1__cm__spomega__bc__bc",
            r"$\omega$",
            inverse_cm,
            "omega")
        pt = TrackVariable(
            "useCMSFrame__bodaughter__bo0__cm__sppt__bc__bc",
            "useCMSFrame__bodaughter__bo1__cm__sppt__bc__bc",
            r"$P_{t}$",
            gev,
            "pt")
    elif frame == "rest":
        # variables for ee -> mumu events - rest frame
        d = TrackVariable(
            "useRestFrame__bodaughter__bo0__cm__spd0__bc__bc",
            "useRestFrame__bodaughter__bo1__cm__spd0__bc__bc",
            r"d$_{0}$",
            cm,
            "d")
        z = TrackVariable(
            "useRestFrame__bodaughter__bo0__cm__spz0__bc__bc",
            "useRestFrame__bodaughter__bo1__cm__spz0__bc__bc",
            r"z$_{0}$",
            cm,
            "z")
        phi = TrackVariable(
            "useRestFrame__bodaughter__bo0__cm__spphi0__bc__bc",
            "useRestFrame__bodaughter__bo1__cm__spphi0__bc__bc",
            r"$\Phi_{0}$",
            rad,
            "phi")
        tanLambda = TrackVariable(
            "useRestFrame__bodaughter__bo0__cm__sptanLambda__bc__bc",
            "useRestFrame__bodaughter__bo1__cm__sptanLambda__bc__bc",
            r"tan($\lambda$)",
            unit,
            "tanLambda")
        omega = TrackVariable(
            "useRestFrame__bodaughter__bo0__cm__spomega__bc__bc",
            "useRestFrame__bodaughter__bo1__cm__spomega__bc__bc",
            r"$\omega$",
            inverse_cm,
            "omega")
        pt = TrackVariable(
            "useRestFrame__bodaughter__bo0__cm__sppt__bc__bc",
            "useRestFrame__bodaughter__bo1__cm__sppt__bc__bc",
            r"$P_{t}$",
            gev,
            "pt")

    InvM = GlobalVariable("InvM", r"$M_{inv}$", gev, "InvM")

    treename = "variables"

    variables_to_load = [run, time, d, z, phi, tanLambda, omega, pt, InvM]
    data = {}
    for file in filenames:
        print(f"Reading {file[:-5]}")
        tfile = ROOT.TFile(file, "OPEN")
        dataframe = ROOT.RDataFrame(treename, tfile)
        filtered_dataframe = dataframe.Filter(selection)
        data[file] = filtered_dataframe.AsNumpy(
            columns=(get_variable_names(variables_to_load) + ["IPX", "IPY", "IPZ"]))
        print(
            f"Number of events after applying selection in {file[:-5]}"
            f" is: {len(data[file][get_variable_names(variables_to_load)[0]])}")

    fix_ip_location(data)
    return data


def fix_ip_location(data: dict):
    """Refit helix parameters relative to the per-event IP position for all dimuon events."""
    print("Recalculating IP location.")
    for file in data.keys():
        for i, _ in enumerate(data[file][time.name]):
            helix1 = ROOT.Belle2.Helix(data[file][d.name1][i],
                                       data[file][phi.name1][i],
                                       data[file][omega.name1][i],
                                       data[file][z.name1][i],
                                       data[file][tanLambda.name1][i])
            helix2 = ROOT.Belle2.Helix(data[file][d.name2][i],
                                       data[file][phi.name2][i],
                                       data[file][omega.name2][i],
                                       data[file][z.name2][i],
                                       data[file][tanLambda.name2][i])

            helix1.passiveMoveBy(
                data[file]["IPX"][i],
                data[file]["IPY"][i],
                data[file]["IPZ"][i])
            helix2.passiveMoveBy(
                data[file]["IPX"][i],
                data[file]["IPY"][i],
                data[file]["IPZ"][i])

            data[file][d.name1][i] = helix1.getD0()
            data[file][d.name2][i] = helix2.getD0()

            data[file][z.name1][i] = helix1.getZ0()
            data[file][z.name2][i] = helix2.getZ0()

            data[file][tanLambda.name1][i] = helix1.getTanLambda()
            data[file][tanLambda.name2][i] = helix2.getTanLambda()

            data[file][phi.name1][i] = helix1.getPhi0()
            data[file][phi.name2][i] = helix2.getPhi0()

            data[file][omega.name1][i] = helix1.getOmega()
            data[file][omega.name2][i] = helix2.getOmega()

            if i % 10000 == 0:
                print(
                    f"Processing event: {i} of {len(data[file][d.name1])}.",
                    end="\r",
                    flush=True)
        print(
            f"Processing event: {i+1} of {len(data[file][d.name1])}.",
            flush=True)


def get_variable_names(variables: list):
    """Flatten a list of Variable objects into a list of branch-name strings."""
    variable_names = []
    for variable in variables:
        var = variable.getName()
        variable_names.extend(var)
    return variable_names


def auto_range(
        data_list: list,
        percent: float,
        modify: float = 0,
        symmetric: bool = False):
    """Return an axis range covering the central `percent` of all datasets, optionally symmetrized."""
    low_percent = (100 - percent) / 2
    high_percent = 100 - low_percent
    low, high, median = (), (), ()
    for data in data_list:
        low += (np.percentile(data, low_percent),)
        high += (np.percentile(data, high_percent),)
        median += (np.median(data),)  # median is more stable than mean
    minimum = min(low)
    maximum = max(high)
    center = np.mean(median)
    if symmetric:
        if abs(center - minimum) > abs(center - maximum):
            maximum = center + center - minimum
        if abs(center - minimum) < abs(center - maximum):
            minimum = center + center - maximum

    # extend interval slightly to capture edges of the data more nicely
    minimum = minimum - (abs(maximum) - abs(minimum)) * modify
    maximum = maximum + (abs(maximum) - abs(minimum)) * modify

    return (minimum, maximum)


def normal_distribution(x: float, a: float, mu: float, sigma: float):
    """Evaluate a normalised Gaussian with amplitude a, mean mu, and width sigma at x."""
    return (a / (sigma * (2 * np.pi)**0.5)) * \
        np.exp(-((x - mu) / sigma)**2 / 2)


def to_bins(x, y, bins, x_limits):
    """Bin (x, y) data and compute per-bin median, sigma68, and their uncertainties."""
    # sigma68 is found using percentiles! - possibly not symetric around median
    x_edges = np.histogram_bin_edges(x, bins, x_limits)
    x_bin_halfwidth = [(x_edges[i] - x_edges[i - 1]) /
                       2 for i in range(1, len(x_edges))]
    # bin index for each element of x
    bin_numbers = np.digitize(x, x_edges) - 1
    y_medians, y_approx_uncert, halfwidth_of_sigma68, sigma68_approx_uncert, x_halfwidth, x_vals = [], [], [], [], [], []
    for i in range(len(x_edges) - 1):
        if y[bin_numbers == i].size >= 10:
            if log_level > 1:
                print(
                    f"Events in bin {x_edges[i]} to {x_edges[i+1]}: {y[bin_numbers == i].size}")
            # array of y means
            y_medians.append(float(np.median(y[bin_numbers == i])))
            y_approx_uncert.append(float(np.std(
                y[bin_numbers == i]) / len(y[bin_numbers == i])**0.5))  # approximation of y error
            halfwidth_of_sigma68.append((np.percentile(y[bin_numbers == i], 84) -
                                        np.percentile(y[bin_numbers == i], 16)) / 2)  # half the sigma68 interval
            # approximation of sigma68 error sig68/sqrt(N)
            sigma68_approx_uncert.append(
                halfwidth_of_sigma68[i] / len(y[bin_numbers == i])**0.5)
            x_halfwidth.append(x_bin_halfwidth[i])  # half the bin width
            # x value is in the middle of each bin
            x_vals.append(x_edges[i] + x_bin_halfwidth[i])
        else:
            y_medians.append(np.nan)
            y_approx_uncert.append(np.nan)
            halfwidth_of_sigma68.append(np.nan)
            sigma68_approx_uncert.append(np.nan)
            x_halfwidth.append(np.nan)
            x_vals.append(np.nan)
            if log_level:
                print(f"Warning: bin {x_edges[i]}-{x_edges[i+1]} is empty.")
    return x_vals, y_medians, x_halfwidth, y_approx_uncert, halfwidth_of_sigma68, sigma68_approx_uncert


def swap_tracks(data: dict, vars: list):
    """Swap the name1/name2 arrays for each TrackVariable in `vars` within the data dict."""
    for var in vars:
        if isinstance(var, TrackVariable):
            data[var.name1], data[var.name2] = data[var.name2], data[var.name1]
    return data


def get_filter(
        data: dict,
        cut: dict  # dictionary of tuples
):
    """Return a boolean mask selecting events that pass all (min, max) cuts in `cut`."""
    mask = np.ones_like(next(iter(data.values())), dtype=bool)
    for var in cut.keys():
        mask = np.logical_and(
            (mask, cut[var][0] < data[var], data[var] < cut[var][1]))
    return mask


def pseudomomentum(p, tan, power):
    """Compute muon pseudomomentum p*beta*sin(theta)^(power/2) from momentum and tan(lambda)."""
    return p * (1 + 1 / tan**2)**0.5 * 1 / (1 + 0.105**2 /
                                            (p**2 * (1 + tan**2)))**0.5 * 1 / ((1 + tan**2)**0.5)**power


# Plotting functions
def plot_histogram(
        data_list: list,
        data_labels: list,
        plot_filename: str,
        xlabel: str,
        range: int = 96,
        opacity: float = 0.5
):
    """Plot overlaid normalised histograms for each dataset and save to the output directory."""
    if isinstance(range, (int, float)):
        hist_ranges = auto_range(data_list, range, 0.1)
    else:
        hist_ranges = range

    plt.figure(figsize=(5, 4))
    plt.ticklabel_format(scilimits=(-2, 3), useMathText=True)
    plt.ylabel("Entries")
    plt.xlabel(xlabel)
    for i, data in enumerate(data_list):
        plt.hist(
            data,
            bins=30,
            range=hist_ranges,
            alpha=opacity,
            density=False,
            label=data_labels[i],
            rasterized=True)
        # Fit data and save separately for InvM variable
        if plot_filename == "InvM":
            result = fit_dimuon_mass.fit(
                data, savefig=f"{output_dirname}/dimuon_fit.{file_format}")
            print(result)
            plt.clf()

    plt.legend()
    plt.savefig(
        f"{output_dirname}/{plot_filename}.{file_format}",
        bbox_inches="tight")
    plt.close()


def plot_correlations(
        plot_type: str,
        x_data_list: list,
        y_data_list: list,
        x_data_labels: list,
        y_data_labels: list,
        data_labels: list,  # labels of data in case of multiple files
        nbins: int,  # number of bins on x axis
        ranges: tuple = (96, 90),  # (x limits, y limits)
        make_2D_hist: bool = True,
        figsize: tuple = (10.0, 7.5)
):
    """Plot a grid of per-bin median or resolution profiles and optionally matching 2D histograms."""
    fig, axs = plt.subplots(len(y_data_list[0]), len(
        x_data_list[0]), sharex="col", sharey="row")
    fig.set_size_inches(*figsize)
    if plot_type == 'median':
        fig.suptitle("Correlations")
    elif plot_type == 'resolution':
        fig.suptitle("Resolution relations")
    else:
        raise Exception(
            "Possible arguments for the 'plot_correlations' function are: 'median' or 'resolution'")
    fig.subplots_adjust(wspace=0.2, hspace=0.2)

    if isinstance(ranges[0], (int, float)):
        percentage = ranges[0]
        ranges = ({}, ranges[1])
        for var in x_data_list[0].keys():
            temp = []
            for i, _ in enumerate(x_data_list):
                temp.append(auto_range(
                    [x_data_list[i][var]], percentage, modify=0.1))
            ranges[0][var] = [min(temp, key=lambda x: x[0])[
                0], max(temp, key=lambda x: x[1])[1]]
    if isinstance(ranges[1], (int, float)):
        percentage = ranges[1]
        ranges = (ranges[0], {})
        for var in y_data_list[0].keys():
            temp = []
            for i, _ in enumerate(y_data_list):
                if plot_type == 'median':
                    temp.append(auto_range(
                        [y_data_list[i][var]], percentage, modify=0.1, symmetric=True))
                elif plot_type == 'resolution':
                    temp.append(
                        (0,
                         np.percentile(
                             y_data_list[i][var],
                             50 +
                             percentage /
                             2) -
                            np.percentile(
                             y_data_list[i][var],
                             50 -
                             percentage /
                             2)))
            ranges[1][var] = [min(temp, key=lambda x: x[0])[
                0], max(temp, key=lambda x: x[1])[1]]

    for y, vary in enumerate(y_data_list[0].keys()):
        for x, varx in enumerate(x_data_list[0].keys()):
            for i, _ in enumerate(x_data_list):

                if y == (len(y_data_list[0].keys()) - 1):
                    axs[y, x].set_xlabel(x_data_labels[x])
                if x == 0:
                    axs[y, x].set_ylabel(y_data_labels[y])

                if plot_type == 'median':
                    x_vals, y_vals, x_width, y_err, _, _ = to_bins(
                        x_data_list[i][varx], y_data_list[i][vary], nbins, ranges[0][varx])
                elif plot_type == 'resolution':
                    x_vals, _, x_width, _, y_vals, y_err = to_bins(
                        x_data_list[i][varx], y_data_list[i][vary], nbins, ranges[0][varx])

                # draw the histogram
                with np.errstate(invalid='ignore'):
                    axs[y, x].errorbar(
                        x_vals,
                        y_vals,
                        y_err,
                        x_width,
                        fmt=".",
                        label=data_labels[i]
                    )

            axs[y, x].set_xlim(ranges[0][varx][0], ranges[0][varx][1])
        axs[y, x].set_ylim(ranges[1][vary][0], ranges[1][vary][1])

    fig.legend(
        data_labels,
        loc="upper center",
        ncol=len(data_labels),
        bbox_to_anchor=(
            0.5,
            0.95))
    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/{plot_type}_correlations.{file_format}")
    plt.close()

    if make_2D_hist:
        for i, _ in enumerate(x_data_list):
            plot_correlations_2D_histograms(
                x_data_list[i],
                y_data_list[i],
                x_data_labels,
                y_data_labels,
                data_labels[i],
                nbins,
                ranges,
                figsize=figsize
            )


def plot_correlations_2D_histograms(
        x_data: dict,
        y_data: dict,
        x_data_labels: list,
        y_data_labels: list,
        data_label: str,
        nbins: int,
        ranges: tuple,
        figsize: tuple = (10.0, 7.5)
):
    """Plot a grid of 2D histograms showing correlations between all x/y variable pairs."""
    fig, axs = plt.subplots(len(y_data), len(
        x_data), sharex="col", sharey="row")
    fig.set_size_inches(*figsize)
    fig.suptitle(f"Correlations ({data_label})")
    fig.subplots_adjust(wspace=0.2, hspace=0.2)

    for y, vary in enumerate(y_data.keys()):
        for x, varx in enumerate(x_data.keys()):

            axs[y, x].set_xlim(ranges[0][varx])
            axs[y, x].set_ylim(ranges[1][vary])

            if y == (len(y_data.keys()) - 1):
                axs[y, x].set_xlabel(x_data_labels[x])
            if x == 0:
                axs[y, x].set_ylabel(y_data_labels[y])

            # draw the histogram
            with np.errstate(invalid='ignore'):
                axs[y, x].hist2d(x_data[varx], y_data[vary], [nbins, 10], [
                                 axs[y, x].get_xlim(), axs[y, x].get_ylim()], rasterized=True)

            axs[y, x].set_xlim(ranges[0][varx][0], ranges[0][varx][1])
        axs[y, x].set_ylim(ranges[1][vary][0], ranges[1][vary][1])

    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/Correlations_2dhist_{data_label}.{file_format}")
    plt.close()


def plot_2D_histogram(
        data: dict,
        label: str,
        bins: tuple,  # (x-axis bins, y-axis bins)
):
    """Plot a 2D histogram of track occupancy in the tan(lambda) vs phi detector plane."""
    plt.figure()
    plt.title("Detector map histogram")
    plt.hist2d(
        np.concatenate((data[tanLambda.name1], data[tanLambda.name2])),
        np.concatenate((data[phi.name1], data[phi.name2])),
        (bins[1], bins[0]),
        range=((-2, 3), (-np.pi, np.pi)),
        rasterized=True
    )
    plt.colorbar(label="Events")
    plt.ylabel(phi.latex + phi.unit.name)
    plt.xlabel(tanLambda.latex + tanLambda.unit.name)
    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/map_2dhist_{label}.{file_format}")
    plt.close()


def draw_map(
        map_type: str,
        data: dict,
        label: str,
        variable: TrackVariable,  # variable to plot
        bins: tuple,  # (x-axis bins, y-axis bins)
        vmin: float = None,  # minimum values to plot
        vmax: float = None  # maximum values to plot
):
    """Draw a median or resolution detector map for `variable` binned in tan(lambda) vs phi."""
    # bins can contain integers or lists
    if isinstance(bins[0], int):
        xdim = bins[0]
    else:
        xdim = len(bins[0]) - 1
    if isinstance(bins[1], int):
        ydim = bins[1]
    else:
        ydim = len(bins[1]) - 1
    map = np.zeros((xdim, ydim))

    plt.figure()
    if map_type == "median":
        plt.title(f"Median map ({label})")
    elif map_type == "resolution":
        plt.title(f"Resolution map ({label})")
    else:
        raise Exception(
            "Possible arguments for the 'draw_map' function are: 'median' or 'resolution'")

    x_bins = np.histogram_bin_edges(np.concatenate(
        (data[tanLambda.name1], data[tanLambda.name2])), bins[1], (-2, 3))
    y_bins = np.histogram_bin_edges(np.concatenate(
        (data[phi.name1], data[phi.name2])), bins[0], (-np.pi, np.pi))
    for i, _ in enumerate(y_bins):
        if (i + 1) == len(y_bins):
            continue
        tracks1 = np.logical_and(
            y_bins[i] <= data[phi.name1], data[phi.name1] <= y_bins[i + 1])
        tracks2 = np.logical_and(
            y_bins[i] <= data[phi.name2], data[phi.name2] <= y_bins[i + 1])
        if map_type == "median":
            values = np.concatenate(
                (data[variable.name1][tracks1], data[variable.name2][tracks2]))
            _, map[i], _, _, _, _ = to_bins(
                np.concatenate((data[tanLambda.name1][tracks1], data[tanLambda.name2][tracks2])),
                values,
                bins[1],
                (-2, 3)
            )
        elif map_type == "resolution":
            if variable == d:
                values = np.concatenate((
                    data[variable.name1][tracks1] + data[variable.name2][tracks1],
                    data[variable.name1][tracks2] + data[variable.name2][tracks2],
                ))
            else:
                values = np.concatenate((
                    data[variable.name1][tracks1] - data[variable.name2][tracks1],
                    data[variable.name1][tracks2] - data[variable.name2][tracks2],
                ))
            _, _, _, _, map[i], _ = to_bins(
                np.concatenate((data[tanLambda.name1][tracks1], data[tanLambda.name2][tracks2])),
                values,
                bins[1],
                (-2, 3)
            )
    if vmin is None and vmax is None:
        (rawmin, rawmax) = auto_range(
            [np.ndarray.flatten(map[~np.isnan(map)])], 98)
        vmin = rawmin / 2 ** 0.5 * variable.unit.convert
        vmax = rawmax / 2 ** 0.5 * variable.unit.convert
    elif vmin is None:
        (rawmin, _) = auto_range([np.ndarray.flatten(map[~np.isnan(map)])], 98)
        vmin = rawmin / 2 ** 0.5 * variable.unit.convert
    elif vmax is None:
        (_, rawmax) = auto_range([np.ndarray.flatten(map[~np.isnan(map)])], 98)
        vmax = rawmax / 2 ** 0.5 * variable.unit.convert
    plt.pcolormesh(
        x_bins,
        y_bins,
        map /
        2 ** 0.5 *
        variable.unit.convert,
        vmax=vmax,
        vmin=vmin,
        rasterized=True)
    if map_type == "median":
        plt.colorbar(
            label=fr"$\tilde{{{variable.latex.replace('$','')}}}/\sqrt{{2}}$" +
            variable.unit.dname)
    elif map_type == "resolution":
        if variable == d:
            plt.colorbar(
                label=f"$\\sigma_{{68}}$($\\Sigma${d.latex})/$\\sqrt{{2}}$" +
                variable.unit.dname)
        else:
            plt.colorbar(
                label=f"$\\sigma_{{68}}$($\\Delta${variable.latex})/$\\sqrt{{2}}$" +
                variable.unit.dname)
    plt.ylabel(phi.latex + phi.unit.name)
    plt.xlabel(tanLambda.latex + tanLambda.unit.name)
    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/{map_type}_map_{variable.plaintext}_{label}.{file_format}")
    plt.close()


def plot_resolutions_hist(
        suptitle: str,
        data: dict,  # dictionary of data
        labels: dict,  # dictionary of labels for each variable
        nbins: float,  # number of bins in histogram
        ranges: dict = 90,  # a percentage or a dictionary of ranges
        vars_to_fit: list = [],  # list of variables to fit
        shape: tuple = (2, 3),  # shape of the plot (rows, columns)
        figsize: tuple = (9.0, 6.0)
):
    """Plot a grid of residual histograms, optionally with Gaussian fits, for each variable."""
    fig, axs = plt.subplots(shape[0], shape[1])
    fig.suptitle(suptitle, y=0.98)
    fig.set_size_inches(*figsize)
    fig.subplots_adjust(wspace=0.4, hspace=0.6, top=0.85)
    for i, var in enumerate(data.keys()):
        # get the correct axis for all possible shapes
        if shape[0] >= 2 and shape[1] >= 2:
            ax = axs[i // shape[1], i % shape[1]]
        elif shape[0] < 2 and shape[1] >= 2:
            ax = axs[i % shape[1]]
        elif shape[0] >= 2 and shape[1] < 2:
            ax = axs[i // shape[1]]
        else:
            ax = axs

        ax.set_xlabel(labels[var])

        # ranges
        if isinstance(ranges, (int, float)):
            bounds = auto_range([data[var]], ranges,
                                modify=0.1, symmetric=True)
        else:
            bounds = ranges[var]

        nphist = np.histogram(data[var], nbins, range=bounds)
        x = np.linspace(bounds[0], bounds[1], nbins)
        ax.hist(data[var], nbins, range=bounds, rasterized=True)
        ax.set_ylabel("Entries")

        # fit data
        if var in vars_to_fit:
            try:
                fit, cov = curve_fit(
                    normal_distribution, x, nphist[0], (5000, 0, 1))
                err = np.sqrt(np.diag(cov))
                ax.plot(x, normal_distribution(x, fit[0], fit[1], fit[2]), "k")
                # write fit parameters
                brackets = re.findall(r'\[(.*?)\]', labels[var])
                used_units = brackets[-1] if brackets else None
                a_description = f"a = {fit[0]:.3}" + \
                    r" $\pm$ " + f"{err[0]:.1} " + "\n"
                mu_description = fr"$\mu$ = {fit[1]:.3}" + \
                    r" $\pm$ " + f"{err[1]:.1} " + used_units + "\n"
                sigma_description = fr"$\sigma$ = {fit[2]:.3}" + \
                    r" $\pm$ " + f"{err[2]:.1} " + used_units
                fit_parameters = a_description + mu_description + sigma_description
                ax.text(
                    ax.get_xlim()[0],
                    ax.get_ylim()[1],
                    fit_parameters,
                    size=9,
                    va='bottom')
                if log_level > 0:  # write fit parameters into console
                    print(f"Fit parameters for {suptitle}, {var.plaintext}:")
                    print(a_description + mu_description + sigma_description)
                    mu_sig = abs(fit[1]) / fit[2] * 100   # mu/sig [%]
                    # error of mu/sig [%]
                    err_mu_sig = 100 * \
                        ((err[1] / fit[2])**2 + (fit[1] * err[2] / fit[2]**2)**2)**0.5
                    print(
                        r"$\frac{\abs{\mu}}{\sigma}$ = " +
                        f"{mu_sig:.3}" +
                        fr" $\pm$ {err_mu_sig:.3}")
            except BaseException:
                print(f"[Warning] Failed to fit {var.plaintext}")
    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/{suptitle.replace(' ', '_')}.{file_format}")
    plt.close()


def plot_resolution_comparison(
        suptitle: str,
        data_list: list,  # list of dictionaries
        data_labels: list,  # labels of data in case of multiple files
        labels: dict,  # dictionary of labels for each variable
        nbins: float,  # number of bins in histogram
        ranges: dict = 90,  # a percentage or a dictionary of ranges
        shape: tuple = (2, 3),  # shape of the plot (rows, columns)
        figsize: tuple = (9.0, 6.0)
):
    """Overlay residual distributions from multiple files and annotate with median and sigma68."""
    fig, axs = plt.subplots(shape[0], shape[1])
    fig.suptitle(suptitle, y=0.98)
    fig.subplots_adjust(wspace=0.4, hspace=0.6, top=0.85)
    fig.set_size_inches(figsize[0], figsize[1])
    fits = [{} for _ in data_list]
    for i, data in enumerate(data_list):
        for j, var in enumerate(data.keys()):
            # get the correct axis for all possible shapes
            if shape[0] >= 2 and shape[1] >= 2:
                ax = axs[j // shape[1], j % shape[1]]
            elif shape[0] < 2 and shape[1] >= 2:
                ax = axs[j % shape[1]]
            elif shape[0] >= 2 and shape[1] < 2:
                ax = axs[j // shape[1]]
            else:
                ax = axs
            ax.set_xlabel(labels[var])

            # ranges
            if isinstance(ranges, (int, float)):
                bounds = auto_range([data[var]], ranges,
                                    modify=0.1, symmetric=True)
            else:
                bounds = ranges[var]

            ax.hist(
                data[var],
                nbins,
                range=bounds,
                alpha=0.6,
                label=data_labels[i],
                rasterized=True)
            ax.set_ylabel("Entries")

            # write medians and sigma68 on the plots
            sig68 = (np.percentile(data[var], 84) -
                     np.percentile(data[var], 16)) / 2
            median = np.median(data[var])
            mutxt = fr"Median = {median:.3}" + var.unit.dname + "\n"
            sigtxt = fr"$\sigma_{{68}}$ = {sig68:.3}" + var.unit.dname
            fits[i][var] = mutxt + sigtxt
            handles, _ = ax.get_legend_handles_labels()
            if i == len(data_list) - \
                    1:  # only write the fit text on the last pass
                ax.legend(
                    handles=handles,
                    labels=[
                        dict[var] for dict in fits],
                    framealpha=0,
                    loc='upper left')
    fig.legend(
        data_labels,
        loc="upper center",
        ncol=len(data_labels),
        bbox_to_anchor=(
            0.5,
            0.95))

    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/{suptitle.replace(' ', '_')}.{file_format}")
    plt.close()


def plot_resolution(
    suptitle: str,
    datadict: dict,  # dictionary[variable] = [[x,y] for file in filenames]
    data_labels: list,  # labels of data in case of multiple files
    axlabels: dict,  # dict of labels for each variable
    xlimit: list,  # x-axis range [min, max]
    ylimits: dict,  # dictionary of lists
    bins: int,  # number of bins - int or list of edges
    fitfunction: callable = None,  # function to fit the data
    fitlabel: callable = None,  # function returning a sttring
    fitrange: list = None,  # range for the fit function
    figsize: tuple = (10.0, 6),  # figure size
    err_override: dict = None,  # dictionary of errors to override the calculated errors
):
    """Plot sigma68 vs pseudomomentum for each variable with an optional parametric fit."""
    fig, axs = plt.subplots(1, len(datadict))
    fig.set_size_inches(figsize[0], figsize[1])
    fig.subplots_adjust(wspace=0.3)
    fig.suptitle(suptitle)
    for i, var in enumerate(datadict.keys()):
        # get the correct axis
        if len(datadict) > 1:
            ax = axs[i]
        else:
            ax = axs

        # prepare nicer colors than the defaults
        if len(data_labels) <= 3:
            color = iter(["b", "r", "g"])
        else:
            color = iter(plt.cm.rainbow(np.linspace(0, 1, len(data_labels))))

        for j, file in enumerate(datadict[var]):
            c = next(color)
            xdata = datadict[var][j][0]
            ydata = datadict[var][j][1]
            x_vals, _, x_width, _, sig68, sig68_uncert = to_bins(
                xdata, ydata, bins, xlimit)
            if err_override:
                # use the provided errors instead of calculated ones
                sig68_uncert = err_override[var][j]
            ax.errorbar(
                x_vals,
                sig68,
                sig68_uncert,
                x_width,
                fmt="o",
                label=data_labels[j],
                c=c,
                rasterized=True
            )

            # fit the data
            if fitfunction:
                if not fitrange:
                    fitrange = xlimit
                fitmask = [fitrange[0] < val < fitrange[1] for val in x_vals]
                Fit, Cov = curve_fit(
                    fitfunction,
                    np.array(x_vals)[fitmask],
                    np.array(sig68)[fitmask],
                    sigma=np.array(sig68_uncert)[fitmask]
                )
                Err = np.sqrt(np.diag(Cov))
                fittextlines = fitlabel(Fit, Err).splitlines()
                if j == 0:  # only write the fit text on the first plot
                    fittext = "\n".join(fittextlines)
                else:
                    # skip the first line
                    fittext = "\n".join(fittextlines[1:])

                ax.plot(
                    np.linspace(fitrange[0], fitrange[1], 100),
                    fitfunction(
                        np.linspace(fitrange[0], fitrange[1], 100),
                        Fit[0],
                        Fit[1]
                    ),
                    label=fittext,
                    c=c
                )

        ax.set_xlim(xlimit[0], xlimit[1])
        ax.set_ylim(ylimits[var][0], ylimits[var][1])
        ax.set_xlabel(axlabels[var][0], fontsize=16)
        ax.set_ylabel(axlabels[var][1], fontsize=16)
        ax.legend()

    varsplaintext = f"{[var.plaintext for var in datadict.keys()]}".replace(
        "'",
        "").replace(
        " ",
        "")
    plt.savefig(
        format=f"{file_format}",
        fname=f"{output_dirname}/{suptitle.replace(' ', '_')}{varsplaintext}.{file_format}")
    plt.close()


def randomize_data(original_data: dict, variables: list):
    """Create a Poisson-resampled copy of the data for bootstrap uncertainty estimation."""
    randomized_data = {}
    poiss = np.random.poisson(1, len(original_data[variables[0].getName()[0]]))
    randomized_data_length = sum(poiss)
    for var in variables:
        for varname in var.getName():
            randomized_data[varname] = np.empty(randomized_data_length)
            k = 0  # position in randomized_data
            for i, event in enumerate(original_data[varname]):
                # add each event poiss[i] times
                randomized_data[varname][k:k + poiss[i]] = event
                k += poiss[i]  # move to new position
    return randomized_data


def to_bins_lite(x, y, bins, xlimits):
    """Return per-bin sigma68 half-widths from (x, y) data; optimised for repeated bootstrap calls."""
    x_edges = np.histogram_bin_edges(x, bins, xlimits)
    digitized = np.digitize(x, x_edges[:-1])  # bin index for each element of x
    halfwidth_68 = []
    for i in range(len(x_edges) - 1):
        if y[digitized == i].size != 0:
            halfwidth_68.append((np.percentile(
                y[digitized == i], 84) - np.percentile(y[digitized == i], 16)) / 2)
        else:
            print(f"Warning: bin {x_edges[i]}-{x_edges[i+1]} is empty!")
            pass
    return halfwidth_68


def bootstrap(
    n_samples: int,
    data_list: list,  # list of dictionaries
    vars: list,  # list of variables to bootstrap
    input_type: str,  # 'cosmics' or 'dimuon'
    bins: list,  # list of bin edges or number of bins
    xlimits: tuple,  # (x min, x max)
):
    """Estimate per-bin sigma68 uncertainties via Poisson bootstrapping over n_samples iterations."""
    if n_samples == 0:
        return None  # no bootstrap
    print("Preparing bootstrap.")
    sig68 = {}
    for var in vars:
        sig68[var] = [[] for _ in data_list]
    for i, data in enumerate(data_list):
        iteration = 0
        while iteration < n_samples:
            print(
                f"Bootstrapping: {iteration}/{n_samples}            ",
                end="\r",
                flush=True)

            # calculate sig68 for tanLambda, phi, d, z
            if set(vars) & set([tanLambda, phi, d, z]):
                bootstrap_data = randomize_data(
                    data, list(set(vars).union([pt, tanLambda])))
                pseudomom3 = np.array([
                    *pseudomomentum(bootstrap_data[pt.name1], bootstrap_data[tanLambda.name1], 3),
                    *pseudomomentum(bootstrap_data[pt.name2], bootstrap_data[tanLambda.name2], 3)
                ])
                pseudomom5 = np.array([
                    *pseudomomentum(bootstrap_data[pt.name1], bootstrap_data[tanLambda.name1], 5),
                    *pseudomomentum(bootstrap_data[pt.name2], bootstrap_data[tanLambda.name2], 5)
                ])
                for var in [d, phi]:
                    if var in vars:
                        if input_type == "dimuon":
                            y = (
                                bootstrap_data[var.name1] + bootstrap_data[var.name2]) / 2**0.5 * var.unit.convert,
                        if input_type == "cosmics":
                            y = (
                                bootstrap_data[var.name1] - bootstrap_data[var.name2]) / 2**0.5 * var.unit.convert,
                        ydata = np.ndarray.flatten(np.array([*y, *y]))
                        sig68[var][i].append(to_bins_lite(
                            pseudomom5, ydata, bins, xlimits))

                for var in [z, tanLambda]:
                    if var in vars:
                        y = (
                            bootstrap_data[var.name1] - bootstrap_data[var.name2]) / 2**0.5 * var.unit.convert,
                        ydata = np.ndarray.flatten(np.array([*y, *y]))
                        sig68[var][i].append(to_bins_lite(
                            pseudomom3, ydata, bins, xlimits))

            # calculate sig68 for pt
            if pt in vars:
                bootstrap_data = randomize_data(data, [pt])
                xdata = np.array([*bootstrap_data[pt.name1],
                                 *bootstrap_data[pt.name2]])
                y1 = ((bootstrap_data[pt.name1] -
                       bootstrap_data[pt.name2]) /
                      bootstrap_data[pt.name1] *
                      100)  # ( pt1 - pt2 ) / pt1 in %
                y2 = ((bootstrap_data[pt.name1] -
                       bootstrap_data[pt.name2]) /
                      bootstrap_data[pt.name2] *
                      100)  # ( pt1 - pt2 ) / pt2 in %
                ydata = np.ndarray.flatten(np.array([*y1, *y2]))
                sig68[pt][i].append(to_bins_lite(xdata, ydata, bins, xlimits))

            iteration += 1
        if iteration == n_samples:
            print(
                f"Bootstrapping: {iteration}/{n_samples}            ",
                end="\n",
                flush=True)
    err = {}
    for var in vars:
        err[var] = [[] for _ in data_list]
        for i, _ in enumerate(data_list):
            err[var][i] = np.array(sig68[var][i]).std(axis=0)
    return err


def main(input_type: str, filenames_pattern: str, output_dir=None):
    """Load data, create the output directory, and run all validation plots for the given input type."""
    global output_dirname

    if output_dir is None:
        output_dir = ""
    else:
        output_dir = str(output_dir).rstrip("/") + "/"

    root_files = glob.glob(filenames_pattern)
    # merge root files with hadd
    os.system(f"hadd -f {input_type}.root {' '.join(root_files)}")

    filenames = [f"{input_type}.root"]

    # Prepare a string for output directory name
    output_dirname = [string.replace(".root", "") for string in filenames]
    output_dirname = ",".join(output_dir + str(x) for x in output_dirname)

    # Create output directory
    try:
        os.mkdir(f"{output_dirname}")
    except FileExistsError:
        print(f"Overwriting existing output directory. ({output_dirname})")

    # ====================================================================================================================#
    # Data handling

    # selection for cosmics:
    # no cut
    cosmics_selection = "run>=0"
    # only tracks through beampipe cut
    cosmics_selection += " && abs(D01)<1 && abs(D02)<1&& Z01>-2 && Z02>-2 && Z01<4 && Z02<4"
    # remove outliers cut
    cosmics_selection += " && abs(D01-D02)<0.2 && abs(Z01-Z02)<0.2"

    # selection for dimuons:
    # no cut
    dimuon_selection = "__run__>=0"
    # only muons cut
    dimuon_selection += " && daughter__bo0__cm__spmuonID__bc>=0.8 && daughter__bo1__cm__spmuonID__bc>=0.8"

    # you may need to edit the variables in these functions to match your
    # input file
    if input_type == 'cosmics':
        data = load_cosmic_data(filenames, cosmics_selection)
    elif input_type == 'dimuon':
        data = load_dimuon_data(filenames, dimuon_selection, "cms")

    # ================================================================================================#
    # Histograms
    print("Making histograms")

    # base histograms for TrackVariables
    for var in [d, z, omega, pt]:
        plot_histogram(
            [[*data[file][var.name1], *data[file][var.name2]] for file in filenames],
            [file[:-5] for file in filenames],
            var.plaintext,
            f"{var.latex} {var.unit.name}"
        )

    # histograms with maximum range for angles
    for var in [phi, tanLambda]:
        plot_histogram(
            [[*data[file][var.name1], *data[file][var.name2]] for file in filenames],
            [file[:-5] for file in filenames],
            var.plaintext,
            f"{var.latex} {var.unit.name}",
            range=100
        )

    # base histograms for GlobalVariables
    for var in [run, time]:
        plot_histogram(
            [data[file][var.name] for file in filenames],
            [file[:-5] for file in filenames],
            var.plaintext,
            f"{var.latex} {var.unit.name}"
        )

    if input_type == 'dimuon':
        # InvM fit
        for var in [InvM]:
            plot_histogram(
                [data[file][var.name] for file in filenames],
                [file[:-5] for file in filenames],
                var.plaintext,
                f"{var.latex} {var.unit.name}"
            )

    # 'delta' histograms
    print("Making track1 - track2 histograms")
    for var in [d, z, phi, tanLambda, omega, pt]:
        plot_histogram(
            [(data[file][var.name1] - data[file][var.name2]) / 2**0.5 for file in filenames],
            [file[:-5] for file in filenames],
            "delta-" + var.plaintext,
            f"$\\Delta${var.latex} {var.unit.name}"
        )

    # 'sigma' histograms
    print("Making track1 + track2 histograms")
    for var in [d, z, phi, tanLambda, omega, pt]:
        plot_histogram(
            [(data[file][var.name1] + data[file][var.name2]) / 2**0.5 for file in filenames],
            [file[:-5] for file in filenames],
            "sigma-" + var.plaintext,
            f"$\\Sigma${var.latex} {var.unit.name}"
        )

    # ==============
    # Correlations
    print("Making Correlations")

    # # example ranges for correlations
    # correlations_xlimits = {
    #     d: [-0.00001, 0.00001],
    #     z: [-0.00001, 0.00001],
    #     phi: [-2, 2],
    #     tanLambda: [0.280, 0.290],
    #     omega: [-5, 5],
    #     pt: [-10, 10]
    #     }
    # correlations_ylimits = {
    #     d: [-20, 20],
    #     z: [-2, 2],
    #     phi: [2000, 2250],
    #     tanLambda: [-1500, 1500],
    #     omega: [-2, 2],
    #     pt: [-5, 5]
    #     }

    correlations_xdata, correlations_ydata = {}, {}
    correlations_xlabels, correlations_ylabels = {}, {}

    if input_type == 'cosmics':
        for file in filenames:
            correlations_xdata[file] = {}
            correlations_ydata[file] = {}
            for var in [d, z, phi, tanLambda, omega]:
                correlations_xdata[file][var] = np.array(
                    [*data[file][var.name1], *data[file][var.name2]])
                correlations_xlabels[var] = var.latex + var.unit.name
                y = (data[file][var.name1] - data[file]
                     [var.name2]) / 2**0.5 * var.unit.convert
                correlations_ydata[file][var] = np.array(
                    [*y, *y])  # for each muon once
                correlations_ylabels[var] = r"$\Delta$" + \
                    var.latex + var.unit.dname
                figsize = (10.0, 7.5)

    elif input_type == 'dimuon':
        for file in filenames:
            correlations_xdata[file] = {}
            correlations_ydata[file] = {}
            correlations_xdata[file][phi] = np.array(
                [*data[file][phi.name1], *data[file][phi.name2]])
            correlations_xlabels[phi] = phi.latex + phi.unit.name
            correlations_xdata[file][tanLambda] = np.array(
                [*data[file][tanLambda.name1], *data[file][tanLambda.name2]])
            correlations_xlabels[tanLambda] = tanLambda.latex + \
                tanLambda.unit.name
            y = (data[file][d.name1] + data[file]
                 [d.name2]) / 2**0.5 * d.unit.convert
            correlations_ydata[file][d] = np.array(
                [*y, *y])  # for each muon once
            correlations_ylabels[d] = r"$\Sigma$" + d.latex + d.unit.dname
            y = (data[file][z.name1] - data[file]
                 [z.name2]) / 2**0.5 * z.unit.convert
            correlations_ydata[file][z] = np.array(
                [*y, *y])  # for each muon once
            correlations_ylabels[z] = r"$\Delta$" + z.latex + z.unit.dname
            figsize = (6, 5)

    plot_correlations(
        'median',
        [correlations_xdata[file] for file in filenames],
        [correlations_ydata[file] for file in filenames],
        [correlations_xlabels[var] for var in correlations_xlabels.keys()],
        [correlations_ylabels[var] for var in correlations_ylabels.keys()],
        [file[:-5] for file in filenames],
        15,
        figsize=figsize,
    )

    plot_correlations('resolution',
                      [correlations_xdata[file] for file in filenames],
                      [correlations_ydata[file] for file in filenames],
                      [correlations_xlabels[var] for var in correlations_xlabels.keys()],
                      [r"$\sigma_{{68}} ({})$".format(
                          correlations_ylabels[var].replace(
                              var.unit.dname, "").replace("$", " "))
                       + "\n" + var.unit.dname
                       for var in correlations_ylabels.keys()],
                      [file[:-5] for file in filenames],
                      15,
                      figsize=figsize,
                      make_2D_hist=False)

    # ==============
    # Detector 'maps'
    print("Making maps")

    map_bins = (80, 80)

    # not very useful for cosmics
    # also for cosmics, one track is in the opposite direction! (which one?)
    # you might want to do something about that if you plan to use this for
    # cosmics
    for file in filenames:
        plot_2D_histogram(data[file], file[:-5], map_bins)

        draw_map('median', data[file], file[:-5], d, map_bins)
        draw_map('median', data[file], file[:-5], z, map_bins)

        draw_map('resolution', data[file], file[:-5], d, map_bins)
        draw_map('resolution', data[file], file[:-5], z, map_bins)

    # ==============
    # Resolutions
    print("Making resolutions")

    # example how to add a cut, can be just an empty dict
    resolutions_cut = {
        # pt: (3,5)
    }

    # these plots make most sense for cosmics, but can be done for dimuons too as shown below
    # I recommend setting the ranges manually for each variable, as the
    # automatic ranges can get cut off in case of multiple files
    if input_type == "cosmics":
        resolutions_data = {}
        for file in filenames:
            resolutions_labels = {}
            resolutions_data[file] = {}
            mask = get_filter(data[file], resolutions_cut)
            for var in [d, z, phi, tanLambda, omega]:
                resolutions_data[file][var] = (data[file][var.name1][mask] - data[file]
                                               [var.name2][mask]) / 2 ** 0.5 * var.unit.convert
                resolutions_labels[var] = r"$\Delta$" + \
                    var.latex + var.unit.dname
            for var in [pt]:
                resolutions_data[file][var] = ((data[file][var.name1][mask] - data[file][var.name2][mask]) / (
                    data[file][var.name1][mask] + data[file][var.name2][mask])) / 2 ** 0.5
                resolutions_labels[var] = r"$\frac{\Delta P_{t}}{\overline{P_{t}}}$ [1]"

            plot_resolutions_hist(
                f"Resolutions {file[:-5]}",
                resolutions_data[file],
                resolutions_labels,
                40,
                vars_to_fit=resolutions_data[file].keys()
            )
        plot_resolution_comparison(
            "Resolutions",
            [resolutions_data[file] for file in filenames],
            [file[:-5] for file in filenames],
            resolutions_labels,
            40,
            figsize=(11.0, 6.0)
        )

    if input_type == "dimuon":
        resolutions_data = {}
        for file in filenames:
            resolutions_labels = {}
            resolutions_data[file] = {}
            mask = get_filter(data[file], resolutions_cut)
            for var in [d]:
                resolutions_data[file][var] = (data[file][var.name1][mask] + data[file]
                                               [var.name2][mask]) / 2 ** 0.5 * var.unit.convert
                resolutions_labels[var] = r"$\Sigma$" + \
                    var.latex + var.unit.dname
            for var in [z]:
                resolutions_data[file][var] = (data[file][var.name1][mask] - data[file]
                                               [var.name2][mask]) / 2 ** 0.5 * var.unit.convert
                resolutions_labels[var] = r"$\Delta$" + \
                    var.latex + var.unit.dname

            plot_resolutions_hist(
                f"Resolutions {file[:-5]}",
                resolutions_data[file],
                resolutions_labels,
                40,
                vars_to_fit=resolutions_data[file].keys(),
                shape=(1, 2),
                figsize=(9.0, 4.0)
            )
        plot_resolution_comparison(
            "Resolutions",
            [resolutions_data[file] for file in filenames],
            [file[:-5] for file in filenames],
            resolutions_labels,
            40,
            shape=(1, 2),
            figsize=(9.0, 4.0)
        )

    # ==============
    # Resolution vs Pseudomomentum
    print("Making resolution vs pseudomomentum")

    pseudomom3 = [np.array([
        *pseudomomentum(data[file][pt.name1], data[file][tanLambda.name1], 3),
        *pseudomomentum(data[file][pt.name2], data[file][tanLambda.name2], 3)
    ]) for file in filenames]
    pseudomom5 = [np.array([
        *pseudomomentum(data[file][pt.name1], data[file][tanLambda.name1], 5),
        *pseudomomentum(data[file][pt.name2], data[file][tanLambda.name2], 5)
    ]) for file in filenames]

    pm_res_data = {}
    pm_res_labels = {}

    for var in [d, phi]:
        ydata = []
        for i, file in enumerate(filenames):
            if input_type == "dimuon":
                y = (data[file][var.name1] + data[file]
                     [var.name2]) / 2**0.5 * var.unit.convert,
            if input_type == "cosmics":
                y = (data[file][var.name1] - data[file]
                     [var.name2]) / 2**0.5 * var.unit.convert,
            ydata.append(np.ndarray.flatten(np.array([*y, *y])))
        pm_res_data[var] = [[pseudomom5[i], ydata[i]]
                            for i, _ in enumerate(filenames)]

        # labels
        if input_type == "dimuon":
            ylabel = r"$\sigma_{{68}}(\frac{{\Sigma {}}}{{\sqrt{{2}}}})$".format(
                var.latex.replace("$", "")) + f"{var.unit.dname}"
        if input_type == "cosmics":
            ylabel = r"$\sigma_{{68}}(\frac{{\Delta {}}}{{\sqrt{{2}}}})$".format(
                var.latex.replace("$", "")) + f"{var.unit.dname}"
        xlabel = r"p$\beta$" + r"(sin$\theta$)$^{{5/2}}$ [GeV/c]"
        pm_res_labels[var] = [xlabel, ylabel]

    for var in [z, tanLambda]:
        ydata = []
        for i, file in enumerate(filenames):
            y = (data[file][var.name1] - data[file][var.name2]) / \
                2**0.5 * var.unit.convert,
            ydata.append(np.ndarray.flatten(np.array([*y, *y])))
        pm_res_data[var] = [[pseudomom3[i], ydata[i]]
                            for i, _ in enumerate(filenames)]

        # labels
        ylabel = r"$\sigma_{{68}}(\frac{{\Delta {}}}{{\sqrt{{2}}}})$".format(
            var.latex.replace("$", "")) + f"{var.unit.dname}"
        xlabel = r"p$\beta$" + r"(sin$\theta$)$^{{3/2}}$ [GeV/c]"
        pm_res_labels[var] = [xlabel, ylabel]

    if input_type == "dimuon":
        pm_res_xlimit = [0, 5]
        pm_res_ylimits = {
            d: [0, 50],
            z: [0, 100],
            phi: [1000, 2000],
            tanLambda: [0, 3000]
        }
        pm_res_bins = [2, 2.5, 3, 3.5, 4, 4.2, 4.4, 4.6, 4.8, 5, 6]
    if input_type == "cosmics":
        pm_res_xlimit = [0, 10]
        pm_res_ylimits = {
            d: [0, 50],
            z: [0, 100],
            phi: [0, 3],
            tanLambda: [0, 4]
        }
        pm_res_bins = [0.5, 1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10]

    def resolutionfit(x, a, b): return (a**2 + (b / x)**2)**0.5

    def ptfitlabel(fit, err): return r"y=$\sqrt{a^{2} +b^{2}/x^{2}} $" + "\n" \
        + f"a = ({fit[0]:.3f}" + r" $\pm$ " + f"{err[0]:.3f})" + "[$\\mu $m]" + "\n" \
        + f"b = ({fit[1]:.3f}" + r" $\pm$ " + f"{err[1]:.3f})" + "[$\\mu $m GeV/c]"

    # resolutionerr = bootstrap(
    #   n_samples=100,
    #   data_list=[data[file] for file in filenames],
    #   vars=pm_res_vars,
    #   input_type=input_type,
    #   bins=pm_res_bins,
    #   xlimits=pm_res_xlimit
    #   )

    plot_resolution(
        "Resolution vs Pseudomomentum",
        {var: pm_res_data[var] for var in [d, z]},
        [file[:-5] for file in filenames],
        pm_res_labels,
        pm_res_xlimit,
        pm_res_ylimits,
        pm_res_bins,
        resolutionfit,
        ptfitlabel,
        # avoid division by zero in fit function
        fitrange=[0.01, pm_res_xlimit[1]],
        # err_override=resolutionerr,
    )
    if input_type == "cosmics":
        plot_resolution(
            "Resolution vs Pseudomomentum",
            {var: pm_res_data[var] for var in [phi, tanLambda]},
            [file[:-5] for file in filenames],
            pm_res_labels,
            pm_res_xlimit,
            pm_res_ylimits,
            pm_res_bins,
            resolutionfit,
            ptfitlabel,
            # avoid division by zero in fit function
            fitrange=[0.01, pm_res_xlimit[1]],
            # err_override=resolutionerr,
        )

    # ==============
    # Pt resolution vs pt
    print("Making pt resolution vs pt")

    pt_res_data = {}
    pt_res_labels = {}

    xdata = [np.array([*data[file][pt.name1], *data[file][pt.name2]])
             for file in filenames]

    for var in [pt]:
        ydata = []
        for i, file in enumerate(filenames):
            y1 = ((data[file][var.name1] - data[file][var.name2]) /
                  data[file][var.name1] * 100)  # ( pt1 - pt2 ) / pt1 in %
            y2 = ((data[file][var.name1] - data[file][var.name2]) /
                  data[file][var.name2] * 100)  # ( pt1 - pt2 ) / pt2 in %
            ydata.append(np.ndarray.flatten(np.array([*y1, *y2])))
        pt_res_data[var] = [[xdata[i], ydata[i]]
                            for i, _ in enumerate(filenames)]

        ylabel = r"$\sigma_{{68}}(\frac{{\Delta {}}}{{{}}})$".format(
            var.latex.replace("$", ""), var.latex.replace("$", "")) + "[%]"
        xlabel = f"{var.latex}" + f"{var.unit.name}"
        pt_res_labels[var] = [xlabel, ylabel]

        def ptfit(x, a, b): return ((x * a)**2 + b**2)**0.5

        def ptfitlabel(fit, err): return r"y=$\sqrt{A^{2} x^{2}+B^{2}} $" + "\n" \
            + f"A = ({fit[0]:.3f}" + r" $\pm$ " + f"{err[0]:.3f})" + "[% c/GeV]" + "\n" \
            + f"B = ({fit[1]:.3f}" + r" $\pm$ " + f"{err[1]:.3f})" + "[%]"

    if input_type == "cosmics":
        pt_res_xrange = [0, 10]
        pt_res_yrange = {pt: [0, 2]}
        pt_res_bins = [1, 1.5, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10]

    if input_type == "dimuon":
        pt_res_xrange = [2, 5.5]
        pt_res_yrange = {pt: [0, 9]}
        pt_res_bins = [2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5]

    # pt_resolutionerr = bootstrap(
    #   n_samples=100,
    #   data_list=[data[file] for file in filenames],
    #   vars=[pt],
    #   input_type=input_type,
    #   bins=pt_res_bins,
    #   xlimits=pt_res_xrange
    #   )

    plot_resolution(
        "Pt resolution vs Pt",
        pt_res_data,
        [file[:-5] for file in filenames],
        pt_res_labels,
        pt_res_xrange,
        pt_res_yrange,
        pt_res_bins,
        fitfunction=ptfit,
        fitlabel=ptfitlabel
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Validation script for track reconstruction.")
    parser.add_argument(
        "input_type",
        choices=[
            "cosmics",
            "dimuon"],
        help="Type of input data (cosmics or dimuon).")
    parser.add_argument(
        "filenames_pattern",
        help="Pattern for input ROOT files (e.g. 'data/*.root').")
    args = parser.parse_args()
    main(args.input_type, args.filenames_pattern)
