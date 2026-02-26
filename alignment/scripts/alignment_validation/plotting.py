"""
Plotting functions for alignment validation.

Module-level config (set by caller before running):
    output_dir   — directory where plots are saved (default: "validation_plots")
    file_format  — image format (default: "pdf")
"""

import re

import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit

from alignment_validation.utils import auto_range, normal_distribution, to_bins

#: Output directory for saved plots
output_dir = "validation_plots"
#: Image file format
file_format = "pdf"


def plot_histogram(
        data_list: list,
        data_labels: list,
        plot_filename: str,
        xlabel: str,
        range: int = 96,
        opacity: float = 0.5,
        ):
    """Plot overlaid histograms for each dataset and save to the output directory.

    The histogram range is determined automatically from the data unless an
    explicit range is provided.

    Parameters
    ----------
    data_list : list of array-like
        One array per dataset to histogram.
    data_labels : list of str
        Legend labels, one per dataset.
    plot_filename : str
        Output file base name (without extension). The file is saved as
        ``{output_dir}/{plot_filename}.{file_format}``.
    xlabel : str
        x-axis label (supports LaTeX).
    range : int or float or tuple, optional
        If a number, the central ``range``% of the union of all datasets is
        used as the histogram range (via :func:`auto_range`). If a ``(min,
        max)`` tuple, it is used directly. Default is 96.
    opacity : float, optional
        Histogram bar opacity (alpha). Default is 0.5.
    """
    if isinstance(range, (int, float)):
        hist_ranges = auto_range(data_list, range, 0.1)
    else:
        hist_ranges = range

    plt.figure(figsize=(5, 4))
    plt.ticklabel_format(scilimits=(-2, 3), useMathText=True)
    plt.ylabel("Entries")
    plt.xlabel(xlabel)
    for i, data in enumerate(data_list):
        plt.hist(data, bins=30, range=hist_ranges, alpha=opacity, density=False, label=data_labels[i], rasterized=True)

    plt.legend()
    plt.savefig(f"{output_dir}/{plot_filename}.{file_format}", bbox_inches="tight")
    plt.close()


def plot_correlations(
        plot_type: str,
        x_data_list: list,
        y_data_list: list,
        x_data_labels: list,
        y_data_labels: list,
        data_labels: list,
        nbins: int,
        ranges: tuple = (96, 90),
        make_2D_hist: bool = True,
        figsize: tuple = (10.0, 7.5),
        ):
    """Plot a grid of per-bin median or resolution profiles for all x/y variable pairs.

    Produces one subplot per (x variable, y variable) combination. Optionally
    also produces matching 2D histogram plots via
    :func:`plot_correlations_2D_histograms`.

    Saves to ``{output_dir}/{plot_type}_correlations.{file_format}``.

    Parameters
    ----------
    plot_type : str
        ``'median'`` to plot per-bin medians, or ``'resolution'`` to plot
        per-bin sigma68 values.
    x_data_list : list of dict
        One dict per dataset, mapping variable objects to x arrays.
    y_data_list : list of dict
        One dict per dataset, mapping variable objects to y arrays. Must have
        the same keys across all datasets.
    x_data_labels : list of str
        x-axis labels for each x variable, in the same order as the dict keys.
    y_data_labels : list of str
        y-axis labels for each y variable, in the same order as the dict keys.
    data_labels : list of str
        Legend labels, one per dataset.
    nbins : int
        Number of bins for the profile.
    ranges : tuple, optional
        ``(x_range, y_range)`` where each element is either a percentile
        integer/float (auto-computed via :func:`auto_range`) or a pre-computed
        dict mapping variable objects to ``[min, max]`` limits.
        Default is ``(96, 90)``.
    make_2D_hist : bool, optional
        If True, also call :func:`plot_correlations_2D_histograms` for each
        dataset. Default is True.
    figsize : tuple of float, optional
        Figure size in inches ``(width, height)``. Default is ``(10.0, 7.5)``.
    """
    fig, axs = plt.subplots(len(y_data_list[0]), len(x_data_list[0]), sharex="col", sharey="row")
    fig.set_size_inches(*figsize)
    if plot_type == 'median':
        fig.suptitle("Correlations")
    elif plot_type == 'resolution':
        fig.suptitle("Resolution relations")
    else:
        raise ValueError("plot_type must be 'median' or 'resolution'")
    fig.subplots_adjust(wspace=0.2, hspace=0.2)

    if isinstance(ranges[0], (int, float)):
        percentage = ranges[0]
        ranges = ({}, ranges[1])
        for var in x_data_list[0].keys():
            temp = []
            for i, _ in enumerate(x_data_list):
                temp.append(auto_range([x_data_list[i][var]], percentage, modify=0.1))
            ranges[0][var] = [min(temp, key=lambda x: x[0])[0], max(temp, key=lambda x: x[1])[1]]
    if isinstance(ranges[1], (int, float)):
        percentage = ranges[1]
        ranges = (ranges[0], {})
        for var in y_data_list[0].keys():
            temp = []
            for i, _ in enumerate(y_data_list):
                if plot_type == 'median':
                    temp.append(auto_range([y_data_list[i][var]], percentage, modify=0.1, symmetric=True))
                elif plot_type == 'resolution':
                    temp.append(
                        (0,
                         np.percentile(y_data_list[i][var], 50 + percentage / 2) -
                         np.percentile(y_data_list[i][var], 50 - percentage / 2)))
            ranges[1][var] = [min(temp, key=lambda x: x[0])[0], max(temp, key=lambda x: x[1])[1]]

    for y, vary in enumerate(y_data_list[0].keys()):
        for x, varx in enumerate(x_data_list[0].keys()):
            for i, _ in enumerate(x_data_list):
                if y == (len(y_data_list[0].keys()) - 1):
                    axs[y, x].set_xlabel(x_data_labels[x])
                if x == 0:
                    axs[y, x].set_ylabel(y_data_labels[y], fontsize=8)

                if plot_type == 'median':
                    x_vals, y_vals, x_width, y_err, _, _ = to_bins(
                        x_data_list[i][varx], y_data_list[i][vary], nbins, ranges[0][varx])
                elif plot_type == 'resolution':
                    x_vals, _, x_width, _, y_vals, y_err = to_bins(
                        x_data_list[i][varx], y_data_list[i][vary], nbins, ranges[0][varx])

                axs[y, x].tick_params(labelsize=7)
                with np.errstate(invalid='ignore'):
                    axs[y, x].errorbar(x_vals, y_vals, y_err, x_width, fmt=".", label=data_labels[i])

            axs[y, x].set_xlim(ranges[0][varx][0], ranges[0][varx][1])
        axs[y, x].set_ylim(ranges[1][vary][0], ranges[1][vary][1])

    fig.legend(data_labels, loc="upper center", ncol=len(data_labels), bbox_to_anchor=(0.5, 0.95))
    plt.savefig(format=f"{file_format}", fname=f"{output_dir}/{plot_type}_correlations.{file_format}")
    plt.close()

    if make_2D_hist:
        for i, _ in enumerate(x_data_list):
            plot_correlations_2D_histograms(
                x_data_list[i], y_data_list[i],
                x_data_labels, y_data_labels,
                data_labels[i], nbins, ranges, figsize=figsize,
            )


def plot_correlations_2D_histograms(
        x_data: dict,
        y_data: dict,
        x_data_labels: list,
        y_data_labels: list,
        data_label: str,
        nbins: int,
        ranges: tuple,
        figsize: tuple = (10.0, 7.5),
        ):
    """Plot a grid of 2D histograms showing correlations between all x/y variable pairs.

    Saves to ``{output_dir}/Correlations_2dhist_{data_label}.{file_format}``.

    Parameters
    ----------
    x_data : dict
        Maps variable objects to x arrays for a single dataset.
    y_data : dict
        Maps variable objects to y arrays for a single dataset.
    x_data_labels : list of str
        x-axis labels for each x variable.
    y_data_labels : list of str
        y-axis labels for each y variable.
    data_label : str
        Dataset label used in the figure title and file name.
    nbins : int
        Number of bins along the x axis of each 2D histogram. The y axis
        uses a fixed 10 bins.
    ranges : tuple of (dict, dict)
        Pre-computed ``(x_ranges, y_ranges)`` dicts mapping variable objects
        to ``[min, max]`` limits, as produced by :func:`plot_correlations`.
    figsize : tuple of float, optional
        Figure size in inches ``(width, height)``. Default is ``(10.0, 7.5)``.
    """
    fig, axs = plt.subplots(len(y_data), len(x_data), sharex="col", sharey="row")
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

            with np.errstate(invalid='ignore'):
                axs[y, x].hist2d(x_data[varx], y_data[vary], [nbins, 10],
                                 [axs[y, x].get_xlim(), axs[y, x].get_ylim()], rasterized=True)

            axs[y, x].set_xlim(ranges[0][varx][0], ranges[0][varx][1])
        axs[y, x].set_ylim(ranges[1][vary][0], ranges[1][vary][1])

    plt.savefig(format=f"{file_format}", fname=f"{output_dir}/Correlations_2dhist_{data_label}.{file_format}")
    plt.close()


def plot_2D_histogram(data: dict, label: str, bins: tuple, phi_var, tan_lambda_var):
    """Plot a 2D histogram of track occupancy in the phi vs tan(lambda) plane.

    Both tracks are combined into a single histogram. Saves to
    ``{output_dir}/map_2dhist_{label}.{file_format}``.

    Parameters
    ----------
    data : dict
        Data dictionary mapping branch names to arrays.
    label : str
        Dataset label used in the file name.
    bins : tuple of int
        ``(n_phi_bins, n_tan_lambda_bins)`` for the 2D histogram.
    phi_var : TrackVariable
        Variable used for the y axis (phi).
    tan_lambda_var : TrackVariable
        Variable used for the x axis (tan lambda).
    """
    plt.figure()
    plt.title("Detector map histogram")
    plt.hist2d(
        np.concatenate((data[tan_lambda_var.name1], data[tan_lambda_var.name2])),
        np.concatenate((data[phi_var.name1], data[phi_var.name2])),
        (bins[1], bins[0]),
        range=((-2, 3), (-np.pi, np.pi)),
        rasterized=True,
    )
    plt.colorbar(label="Events")
    plt.ylabel(phi_var.latex + phi_var.unit.name)
    plt.xlabel(tan_lambda_var.latex + tan_lambda_var.unit.name)
    plt.savefig(format=f"{file_format}", fname=f"{output_dir}/map_2dhist_{label}.{file_format}")
    plt.close()


def draw_map(
        map_type: str,
        data: dict,
        label: str,
        variable,
        bins: tuple,
        phi_var,
        tan_lambda_var,
        vmin: float = None,
        vmax: float = None,
        ):
    """Draw a median or resolution detector map for ``variable`` binned in phi vs tan(lambda).

    The map is built by iterating over phi strips and computing per-bin
    statistics along tan(lambda) within each strip using :func:`to_bins`.
    For the ``d`` variable the sum (track1 + track2) is used; for all other
    variables the difference (track1 - track2) is used.

    Saves to ``{output_dir}/{map_type}_map_{variable.plaintext}_{label}.{file_format}``.

    Parameters
    ----------
    map_type : str
        ``'median'`` to map the per-bin median, or ``'resolution'`` to map
        the per-bin sigma68.
    data : dict
        Data dictionary mapping branch names to arrays.
    label : str
        Dataset label used in the figure title and file name.
    variable : TrackVariable
        Observable to map (e.g. ``d`` or ``z``).
    bins : tuple of int
        ``(n_phi_bins, n_tan_lambda_bins)`` for the map grid.
    phi_var : TrackVariable
        Variable providing the phi coordinate (y axis of the map).
    tan_lambda_var : TrackVariable
        Variable providing the tan(lambda) coordinate (x axis of the map).
    vmin : float, optional
        Minimum of the colour scale. Auto-computed from data if not given.
    vmax : float, optional
        Maximum of the colour scale. Auto-computed from data if not given.
    """
    if isinstance(bins[0], int):
        xdim = bins[0]
    else:
        xdim = len(bins[0]) - 1
    if isinstance(bins[1], int):
        ydim = bins[1]
    else:
        ydim = len(bins[1]) - 1
    map_data = np.zeros((xdim, ydim))

    plt.figure()
    if map_type == "median":
        plt.title(f"Median map ({label})")
    elif map_type == "resolution":
        plt.title(f"Resolution map ({label})")
    else:
        raise ValueError("map_type must be 'median' or 'resolution'")

    x_bins = np.histogram_bin_edges(
        np.concatenate((data[tan_lambda_var.name1], data[tan_lambda_var.name2])), bins[1], (-2, 3))
    y_bins = np.histogram_bin_edges(
        np.concatenate((data[phi_var.name1], data[phi_var.name2])), bins[0], (-np.pi, np.pi))

    for i, _ in enumerate(y_bins):
        if (i + 1) == len(y_bins):
            continue
        tracks1 = np.logical_and(y_bins[i] <= data[phi_var.name1], data[phi_var.name1] <= y_bins[i + 1])
        tracks2 = np.logical_and(y_bins[i] <= data[phi_var.name2], data[phi_var.name2] <= y_bins[i + 1])

        if map_type == "median":
            if variable.plaintext == "d":
                values = np.concatenate((
                    data[variable.name1][tracks1] + data[variable.name2][tracks1],
                    data[variable.name1][tracks2] + data[variable.name2][tracks2],
                ))
            else:
                values = np.concatenate((
                    data[variable.name1][tracks1] - data[variable.name2][tracks1],
                    data[variable.name1][tracks2] - data[variable.name2][tracks2],
                ))
            _, map_data[i], _, _, _, _ = to_bins(
                np.concatenate((data[tan_lambda_var.name1][tracks1], data[tan_lambda_var.name2][tracks2])),
                values, bins[1], (-2, 3),
            )
        elif map_type == "resolution":
            if variable.plaintext == "d":
                values = np.concatenate((
                    data[variable.name1][tracks1] + data[variable.name2][tracks1],
                    data[variable.name1][tracks2] + data[variable.name2][tracks2],
                ))
            else:
                values = np.concatenate((
                    data[variable.name1][tracks1] - data[variable.name2][tracks1],
                    data[variable.name1][tracks2] - data[variable.name2][tracks2],
                ))
            _, _, _, _, map_data[i], _ = to_bins(
                np.concatenate((data[tan_lambda_var.name1][tracks1], data[tan_lambda_var.name2][tracks2])),
                values, bins[1], (-2, 3),
            )

    flat = map_data[~np.isnan(map_data)].flatten()
    if vmin is None and vmax is None:
        rawmin, rawmax = auto_range([flat], 98)
        vmin = rawmin / 2 ** 0.5 * variable.unit.convert
        vmax = rawmax / 2 ** 0.5 * variable.unit.convert
    elif vmin is None:
        rawmin, _ = auto_range([flat], 98)
        vmin = rawmin / 2 ** 0.5 * variable.unit.convert
    elif vmax is None:
        _, rawmax = auto_range([flat], 98)
        vmax = rawmax / 2 ** 0.5 * variable.unit.convert

    plt.pcolormesh(x_bins, y_bins, map_data / 2 ** 0.5 * variable.unit.convert, vmax=vmax, vmin=vmin, rasterized=True)
    if map_type == "median":
        if variable.plaintext == "d":
            plt.colorbar(label=f"$\\tilde{{}}$($\\Sigma${variable.latex})/$\\sqrt{{2}}$" + variable.unit.dname)
        else:
            plt.colorbar(label=f"$\\tilde{{}}$($\\Delta${variable.latex})/$\\sqrt{{2}}$" + variable.unit.dname)
    elif map_type == "resolution":
        if variable.plaintext == "d":
            plt.colorbar(label=f"$\\sigma_{{68}}$($\\Sigma${variable.latex})/$\\sqrt{{2}}$" + variable.unit.dname)
        else:
            plt.colorbar(label=f"$\\sigma_{{68}}$($\\Delta${variable.latex})/$\\sqrt{{2}}$" + variable.unit.dname)
    plt.ylabel(phi_var.latex + phi_var.unit.name)
    plt.xlabel(tan_lambda_var.latex + tan_lambda_var.unit.name)
    plt.savefig(format=f"{file_format}",
                fname=f"{output_dir}/{map_type}_map_{variable.plaintext}_{label}.{file_format}")
    plt.close()


def plot_resolutions_hist(
        suptitle: str,
        data: dict,
        labels: dict,
        nbins: float,
        ranges=90,
        vars_to_fit: list = [],
        shape: tuple = (2, 3),
        figsize: tuple = (9.0, 6.0),
        ):
    """Plot a grid of residual histograms, optionally with Gaussian fits, for each variable.

    Saves to ``{output_dir}/{suptitle}.{file_format}`` (spaces replaced by underscores).

    Parameters
    ----------
    suptitle : str
        Figure title, also used to derive the output file name.
    data : dict
        Maps variable objects to 1-D arrays of residual values.
    labels : dict
        Maps variable objects to x-axis label strings.
    nbins : int
        Number of histogram bins.
    ranges : int or float or dict, optional
        If a number, the central ``ranges``% of each variable's data is used
        as the histogram range (symmetric, via :func:`auto_range`). If a dict,
        maps variable objects to explicit ``(min, max)`` tuples. Default is 90.
    vars_to_fit : list, optional
        Subset of variables for which a Gaussian fit is overlaid and
        annotated. Default is ``[]`` (no fits).
    shape : tuple of int, optional
        ``(nrows, ncols)`` layout of the subplot grid. Default is ``(2, 3)``.
    figsize : tuple of float, optional
        Figure size in inches ``(width, height)``. Default is ``(9.0, 6.0)``.
    """
    fig, axs = plt.subplots(shape[0], shape[1])
    fig.suptitle(suptitle, y=0.98)
    fig.set_size_inches(*figsize)
    fig.subplots_adjust(wspace=0.4, hspace=0.6, top=0.85)

    for i, var in enumerate(data.keys()):
        if shape[0] >= 2 and shape[1] >= 2:
            ax = axs[i // shape[1], i % shape[1]]
        elif shape[0] < 2 and shape[1] >= 2:
            ax = axs[i % shape[1]]
        elif shape[0] >= 2 and shape[1] < 2:
            ax = axs[i // shape[1]]
        else:
            ax = axs

        ax.set_xlabel(labels[var])

        if isinstance(ranges, (int, float)):
            bounds = auto_range([data[var]], ranges, modify=0.1, symmetric=True)
        else:
            bounds = ranges[var]

        nphist = np.histogram(data[var], nbins, range=bounds)
        x = np.linspace(bounds[0], bounds[1], nbins)
        ax.hist(data[var], nbins, range=bounds, rasterized=True)
        ax.set_ylabel("Entries")

        if var in vars_to_fit:
            try:
                fit, cov = curve_fit(normal_distribution, x, nphist[0], (5000, 0, 1))
                err = np.sqrt(np.diag(cov))
                ax.plot(x, normal_distribution(x, fit[0], fit[1], fit[2]), "k")
                brackets = re.findall(r'\[(.*?)\]', labels[var])
                used_units = brackets[-1] if brackets else None
                fit_parameters = (
                    f"a = {fit[0]:.3}" + r" $\pm$ " + f"{err[0]:.1} " + "\n" +
                    fr"$\mu$ = {fit[1]:.3}" + r" $\pm$ " + f"{err[1]:.1} " + used_units + "\n" +
                    fr"$\sigma$ = {fit[2]:.3}" + r" $\pm$ " + f"{err[2]:.1} " + used_units
                )
                ax.text(ax.get_xlim()[0], ax.get_ylim()[1], fit_parameters, size=9, va='bottom')
            except Exception:
                print(f"[Warning] Failed to fit {var.plaintext}")

    plt.savefig(format=f"{file_format}", fname=f"{output_dir}/{suptitle.replace(' ', '_')}.{file_format}")
    plt.close()


def plot_resolution_comparison(
        suptitle: str,
        data_list: list,
        data_labels: list,
        labels: dict,
        nbins: float,
        ranges=90,
        shape: tuple = (2, 3),
        figsize: tuple = (9.0, 6.0),
        ):
    """Overlay residual distributions from multiple datasets and annotate with median and sigma68.

    Each subplot shows histograms from all datasets overlaid, with per-dataset
    median and sigma68 annotated in the legend. Saves to
    ``{output_dir}/{suptitle}.{file_format}`` (spaces replaced by underscores).

    Parameters
    ----------
    suptitle : str
        Figure title, also used to derive the output file name.
    data_list : list of dict
        One dict per dataset, mapping variable objects to residual arrays.
    data_labels : list of str
        Legend labels, one per dataset.
    labels : dict
        Maps variable objects to x-axis label strings.
    nbins : int
        Number of histogram bins.
    ranges : int or float or dict, optional
        If a number, the central ``ranges``% of each variable's data is used
        as the histogram range (symmetric, via :func:`auto_range`). If a dict,
        maps variable objects to explicit ``(min, max)`` tuples. Default is 90.
    shape : tuple of int, optional
        ``(nrows, ncols)`` layout of the subplot grid. Default is ``(2, 3)``.
    figsize : tuple of float, optional
        Figure size in inches ``(width, height)``. Default is ``(9.0, 6.0)``.
    """
    fig, axs = plt.subplots(shape[0], shape[1])
    fig.suptitle(suptitle, y=0.98)
    fig.subplots_adjust(wspace=0.4, hspace=0.6, top=0.85)
    fig.set_size_inches(*figsize)
    fits = [{} for _ in data_list]

    for i, data in enumerate(data_list):
        for j, var in enumerate(data.keys()):
            if shape[0] >= 2 and shape[1] >= 2:
                ax = axs[j // shape[1], j % shape[1]]
            elif shape[0] < 2 and shape[1] >= 2:
                ax = axs[j % shape[1]]
            elif shape[0] >= 2 and shape[1] < 2:
                ax = axs[j // shape[1]]
            else:
                ax = axs

            ax.set_xlabel(labels[var])

            if isinstance(ranges, (int, float)):
                bounds = auto_range([data[var]], ranges, modify=0.1, symmetric=True)
            else:
                bounds = ranges[var]

            ax.hist(data[var], nbins, range=bounds, alpha=0.6, label=data_labels[i], rasterized=True)
            ax.set_ylabel("Entries")

            sig68 = (np.percentile(data[var], 84) - np.percentile(data[var], 16)) / 2
            median = np.median(data[var])
            fits[i][var] = fr"Median = {median:.3}" + var.unit.dname + "\n" + fr"$\sigma_{{68}}$ = {sig68:.3}" + var.unit.dname

            handles, _ = ax.get_legend_handles_labels()
            if i == len(data_list) - 1:
                ax.legend(handles=handles, labels=[d[var] for d in fits], framealpha=0, loc='upper left')

    fig.legend(data_labels, loc="upper center", ncol=len(data_labels), bbox_to_anchor=(0.5, 0.95))
    plt.savefig(format=f"{file_format}", fname=f"{output_dir}/{suptitle.replace(' ', '_')}.{file_format}")
    plt.close()


def plot_resolution(
        suptitle: str,
        datadict: dict,
        data_labels: list,
        axlabels: dict,
        xlimit: list,
        ylimits: dict,
        bins,
        fitfunction: callable = None,
        fitlabel: callable = None,
        fitrange: list = None,
        figsize: tuple = (10.0, 6),
        err_override: dict = None,
        ):
    """Plot sigma68 vs an x variable (e.g. pseudomomentum) for each observable.

    One subplot is produced per variable in ``datadict``. An optional
    parametric fit is overlaid on each dataset's sigma68 profile. Saves to
    ``{output_dir}/{suptitle}{var_names}.{file_format}``.

    Parameters
    ----------
    suptitle : str
        Figure title, also used (with variable names appended) to derive the
        output file name.
    datadict : dict
        Maps variable objects to a list of ``[xdata, ydata]`` pairs, one per
        dataset. ``xdata`` and ``ydata`` are 1-D arrays of equal length.
    data_labels : list of str
        Legend labels, one per dataset.
    axlabels : dict
        Maps variable objects to ``[xlabel, ylabel]`` string pairs.
    xlimit : list of float
        ``[xmin, xmax]`` range for the x axis.
    ylimits : dict
        Maps variable objects to ``[ymin, ymax]`` range for the y axis.
    bins : int or sequence of float
        Bin edges or number of bins passed to :func:`to_bins`.
    fitfunction : callable, optional
        Function ``f(x, *params)`` to fit to the sigma68 profile. If None,
        no fit is drawn. Default is None.
    fitlabel : callable, optional
        Function ``f(params, errors) -> str`` that produces the fit annotation
        string. Required when ``fitfunction`` is provided. Default is None.
    fitrange : list of float, optional
        ``[xmin, xmax]`` sub-range used for the fit. Defaults to ``xlimit``.
    figsize : tuple of float, optional
        Figure size in inches ``(width, height)``. Default is ``(10.0, 6)``.
    err_override : dict, optional
        Maps variable objects to a list of per-dataset sigma68 uncertainty
        arrays that replace the values computed by :func:`to_bins`. Useful for
        providing external uncertainty estimates. Default is None.
    """
    fig, axs = plt.subplots(1, len(datadict))
    fig.set_size_inches(*figsize)
    fig.subplots_adjust(wspace=0.3)
    fig.suptitle(suptitle)

    for i, var in enumerate(datadict.keys()):
        ax = axs[i] if len(datadict) > 1 else axs

        if len(data_labels) <= 3:
            color = iter(["b", "r", "g"])
        else:
            color = iter(plt.cm.rainbow(np.linspace(0, 1, len(data_labels))))

        for j, _ in enumerate(datadict[var]):
            c = next(color)
            xdata = datadict[var][j][0]
            ydata = datadict[var][j][1]
            x_vals, _, x_width, _, sig68, sig68_uncert = to_bins(xdata, ydata, bins, xlimit)
            if err_override:
                sig68_uncert = err_override[var][j]
            ax.errorbar(x_vals, sig68, sig68_uncert, x_width, fmt="o", label=data_labels[j], c=c, rasterized=True)

            if fitfunction:
                if not fitrange:
                    fitrange = xlimit
                fitmask = [fitrange[0] < val < fitrange[1] for val in x_vals]
                Fit, Cov = curve_fit(
                    fitfunction,
                    np.array(x_vals)[fitmask],
                    np.array(sig68)[fitmask],
                    sigma=np.array(sig68_uncert)[fitmask],
                )
                Err = np.sqrt(np.diag(Cov))
                fittextlines = fitlabel(Fit, Err).splitlines()
                fittext = "\n".join(fittextlines) if j == 0 else "\n".join(fittextlines[1:])
                ax.plot(
                    np.linspace(fitrange[0], fitrange[1], 100),
                    fitfunction(np.linspace(fitrange[0], fitrange[1], 100), Fit[0], Fit[1]),
                    label=fittext, c=c,
                )

        ax.set_xlim(xlimit[0], xlimit[1])
        ax.set_ylim(ylimits[var][0], ylimits[var][1])
        ax.set_xlabel(axlabels[var][0], fontsize=16)
        ax.set_ylabel(axlabels[var][1], fontsize=16)
        ax.legend()

    varsplaintext = f"{[var.plaintext for var in datadict.keys()]}".replace("'", "").replace(" ", "")
    plt.savefig(format=f"{file_format}",
                fname=f"{output_dir}/{suptitle.replace(' ', '_')}{varsplaintext}.{file_format}")
    plt.close()
