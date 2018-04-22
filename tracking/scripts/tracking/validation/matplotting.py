"""Tools to plot ROOT object into matplotlib"""

import ROOT

import re
import functools
import numpy as np
np.seterr(invalid='ignore')

import sys
flt_max = sys.float_info.max
flt_min = sys.float_info.min

import collections
from .plot import ValidationPlot

import logging


def get_logger():
    """Getter for the logger instance of this file."""
    return logging.getLogger(__name__)


try:
    import matplotlib
    # Switch to noninteractive backend
    matplotlib.use('Agg')

    import matplotlib.pyplot as plt
    import matplotlib.transforms as transforms
except ImportError:
    raise ImportError("matplotlib is not installed in your basf2 environment. "
                      "You may install it with 'pip install matplotlib'")


class defaults:

    """Default values of the plotting options"""
    style = "bmh"
    legend = True
    label = True

#: A list of classes that are implemented as plotable
plotable_classes = (
    ROOT.TH1,
    ROOT.TH2,
    ROOT.THStack,
    ROOT.TGraph,
    ROOT.TGraphErrors,
    ROOT.TMultiGraph
)


def is_plotable(tobject):
    """Indicates if a module can be plotted with matplotlib using this module."""
    return isinstance(tobject, plotable_classes)


def plot(tobject, **kwd):
    """Plot the given plotable TObject.

    Parameters
    ----------
    tobject : ROOT.TObject
        Plotable TObject.
    legend : bool, optional
        Create a by-side legend containing statistical information.
    style : list(str), optional
        List of matplotlib styles to be used for the plotting.

    Returns
    -------
    matplotlib.figure.Figure
        The figure containing the plot
    """
    if isinstance(tobject, ROOT.TH2):
        return plot_th2(tobject, **kwd)

    elif isinstance(tobject, ROOT.THStack):
        return plot_thstack(tobject, **kwd)

    elif isinstance(tobject, ROOT.TMultiGraph):
        return plot_tmultigraph(tobject, **kwd)

    elif isinstance(tobject, ROOT.TGraph):
        return plot_tgraph(tobject, **kwd)

    elif isinstance(tobject, (ROOT.TProfile, ROOT.TH1)):
        return plot_th1(tobject, **kwd)

    else:
        raise ValueError("Plotting to matplot lib only supported for TH1, TProfile, and THStack.")

    return fig


def use_style(plot_function):
    """Decorator to adjust the matplotlib style before plotting"""
    @functools.wraps(plot_function)
    def plot_function_with_style(*args, **kwds):
        style = kwds.get("style", None)
        if style is None:
            style = defaults.style
        matplotlib.style.use(style)

        matplotlib.rcParams["patch.linewidth"] = 2
        matplotlib.rcParams['mathtext.fontset'] = 'custom'
        matplotlib.rcParams['mathtext.rm'] = 'DejaVu Sans'
        matplotlib.rcParams['mathtext.it'] = 'DejaVu Sans:italic'
        matplotlib.rcParams['mathtext.cal'] = 'DejaVu Sans:italic'
        matplotlib.rcParams['mathtext.bf'] = 'DejaVu Sans:bold'

        # matplotlib.rcParams['font.family'] = ['sans-serif']
        matplotlib.rcParams['font.sans-serif'] = ['DejaVu Sans']
        matplotlib.rcParams['font.monospace'] = ['cmtt10']

        return plot_function(*args, **kwds)

    return plot_function_with_style


@use_style
def plot_th2(th2,
             label=None,
             legend=None,
             style=None,
             **kwd):
    """Plots a two dimensional histogram"""

    title = reformat_root_latex_to_matplotlib_latex(th2.GetTitle())
    fig, ax = create_figure(title=title)

    th2_label = create_label(th2, label)
    plot_th2_data_into(ax, th2, label=th2_label)

    put_legend(fig, ax, legend=legend)
    return fig


@use_style
def plot_thstack(thstack,
                 label=None,
                 legend=None,
                 style=None,
                 **kwd):
    """Plots a stack of histograms"""

    title = create_title(thstack)
    fig, ax = create_figure(title=title)

    # plot reversed such that signal appears on top
    ths = list(thstack.GetHists())

    if all(isinstance(th, ROOT.TH3) for th in ths):
        raise ValueError("Cannot plot a stack of three dimensional histograms")

    elif all(isinstance(th, ROOT.TH2) and not isinstance(th, ROOT.TH3) for th in ths):
        raise NotImplementedError("Cannot plot a stack of two dimensional histograms")

    elif all(isinstance(th, ROOT.TH1) and not isinstance(th, (ROOT.TH3, ROOT.TH2)) for th in ths):
        # currently plot only non stacked
        th1s = ths
        for th1 in th1s:
            th1_label = create_label(th1, label=label)
            plot_th1_data_into(ax, th1, label=th1_label)

        # Fixing that the limits sometimes clip in the y direction
        max_bin_content = thstack.GetMaximum("nostack")
        ax.set_ylim(0, 1.02 * max_bin_content)

    else:
        ValueError("Stack of histograms with mismatching dimensions")

    put_legend(fig, ax, legend=legend)
    return fig


@use_style
def plot_tmultigraph(tmultigraph,
                     label=None,
                     legend=None,
                     style=None,
                     **kwd):
    """Plots multiple overlayed graphs"""

    title = create_title(tmultigraph)
    fig, ax = create_figure(title=title)

    for tgraph in tmultigraph.GetListOfGraphs():
        tgraph_label = create_label(tgraph, label=label)
        plot_tgraph_data_into(ax, tgraph, label=tgraph_label)

    y_lower_bound, y_upper_bound = common_bounds(
        ax.get_ylim(),
        (tmultigraph.GetMinimum(), tmultigraph.GetMaximum())
    )

    ax.set_ylim(y_lower_bound, y_upper_bound)

    put_legend(fig, ax, legend=legend)
    return fig


@use_style
def plot_th1(th1,
             label=None,
             legend=None,
             style=None,
             **kwd):
    """Plots a one dimensional histogram including the fit function if present"""

    title = create_title(th1)
    fig, ax = create_figure(title=title)

    th1_label = create_label(th1, label)

    if th1.GetSumOfWeights() == 0:
        get_logger().info("Skipping empty histogram %s", th1.GetName())
        return fig

    plot_th1_data_into(ax, th1, label=th1_label)

    tf1 = get_fit(th1)
    if tf1:
        tf1_label = create_label(tf1, label=label)
        fit_handles = plot_tf1_data_into(ax, tf1, label=tf1_label)
    else:
        fit_handles = None

    put_legend(fig, ax, legend=legend, top_handles=fit_handles, top_title="Fit")
    return fig


@use_style
def plot_tgraph(tgraph,
                label=None,
                legend=None,
                style=None):
    """Plots graph including the fit function if present"""
    title = create_title(tgraph)
    fig, ax = create_figure(title=title)

    tgraph_label = create_label(tgraph, label=label)
    plot_tgraph_data_into(ax, tgraph, label=tgraph_label)

    tf1 = get_fit(tgraph)
    if tf1:
        tf1_label = create_label(tf1, label=label)
        fit_handles = plot_tf1_data_into(ax, tf1, label=tf1_label)
    else:
        fit_handles = None

    put_legend(fig, ax, legend=legend, top_handles=fit_handles, top_title="Fit")
    return fig


def create_figure(title=None):
    """Create a new figure already making space for a by side legend if requested

    Returns
    -------
    (matplotlib.figure.Figure, matplotlib.axes.Axes)
        A prepared figure and axes into which can be plotted.
    """
    fig = plt.figure()
    ax = fig.add_subplot(111)

    if title:
        ax.set_title(title, y=1.04)

    return fig, ax


def create_title(tplotable):
    """Extract the title from the plotable ROOT object and translate to ROOT latex"""
    return reformat_root_latex_to_matplotlib_latex(tplotable.GetTitle())


def create_label(th_or_tgraph, label=None):
    """Create a label from the plotable object incorporating available summary statistics."""
    if label is None:
        label = defaults.label

    if label:
        if isinstance(th_or_tgraph, ROOT.TH1):
            th = th_or_tgraph
            stats = get_stats_from_th(th)
            label = compose_stats_label("", stats)

        elif isinstance(th_or_tgraph, ROOT.TGraph):
            tgraph = th_or_tgraph
            stats = get_stats_from_tgraph(tgraph)
            label = compose_stats_label("", stats)

        elif isinstance(th_or_tgraph, ROOT.TF1):
            tf1 = th_or_tgraph
            parameters = get_fit_parameters(tf1)
            label = compose_stats_label("", parameters)

        else:
            raise ValueError("Can only create a label from a ROOT.TH or ROOT.TGraph")

        return label
    return None


def put_legend(fig,
               ax,
               legend=None,
               top_handles=None,
               top_title=None,
               bottom_title=None):
    """Put the legend of the plot

    Put one legend to right beside the axes space for some plot handles if desired.
    Put one legend at the bottom for the remaining plot handles.
    """

    if legend is None:
        legend = defaults.legend

    if legend:
        fig_width = fig.get_figwidth()
        # Expanding figure by 33 %
        fig.set_figwidth(1.0 * 4.0 / 3.0 * fig_width)

        # Shink current axis by 25%
        box = ax.get_position()
        ax.set_position([box.x0, box.y0, box.width * 0.75, box.height])

        if top_handles:
            put_legend_outside(ax,
                               exclude_handles=top_handles,
                               force_add_legend=True,
                               title=bottom_title)
            put_legend_outside(ax,
                               select_handles=top_handles,
                               bottom=False,
                               title=top_title)
        else:
            put_legend_outside(ax,
                               exclude_handles=top_handles,
                               title=bottom_title)


def put_legend_outside(ax,
                       title=None,
                       bottom=True,
                       select_handles=[],
                       exclude_handles=[],
                       force_add_legend=False,
                       ):
    """Put a legned right beside the axes space"""

    if not select_handles:
        select_handles, _ = ax.get_legend_handles_labels()

    if exclude_handles:
        select_handles = [handle for handle in select_handles if handle not in exclude_handles]

    fig = ax.get_figure()
    # trans = transforms.blended_transform_factory(fig.transFigure, ax.transAxes)

    if bottom:
        # legend_artist = ax.legend(handles=select_handles,
        #                           bbox_to_anchor=(1., 0.),
        #                           bbox_transform=trans,
        #                           borderaxespad=0.,
        #                           loc=4,
        #                           prop={"family": "monospace"},
        #                           title=title)

        legend_artist = ax.legend(handles=select_handles,
                                  bbox_to_anchor=(1.02, 0),
                                  borderaxespad=0.,
                                  loc=3,
                                  prop={"family": "monospace"},
                                  title=title)
    else:
        # legend_artist = ax.legend(handles=select_handles,
        #                           bbox_to_anchor=(1., 1.),
        #                           bbox_transform=trans,
        #                           borderaxespad=0.,
        #                           loc=1,
        #                           prop={"family": "monospace"},
        #                           title=title)

        legend_artist = ax.legend(handles=select_handles,
                                  bbox_to_anchor=(1.02, 1),
                                  borderaxespad=0.,
                                  loc=2,
                                  prop={"family": "monospace"},
                                  title=title)

    if force_add_legend:
        ax.add_artist(legend_artist)


def get_fit(th1_or_tgraph):
    """Retrieve a potential fit function form the plotable object"""
    for tobject in th1_or_tgraph.GetListOfFunctions():
        if isinstance(tobject, ROOT.TF1):
            return tobject


def get_fit_parameters(tf1):
    """Retrieve the fitted parameters explicitly excluding fixed parameters.

    Fixed parameters are usually additional stats entries that are already shown in the main
    legend for the plot.
    """
    parameters = collections.OrderedDict()

    formula = tf1.GetTitle()
    parameters["formula"] = formula

    n_parameters = tf1.GetNpar()
    for i_parameter in range(n_parameters):

        lower_bound = ROOT.Double()
        upper_bound = ROOT.Double()
        tf1.GetParLimits(i_parameter, lower_bound, upper_bound)

        name = tf1.GetParName(i_parameter)
        value = tf1.GetParameter(i_parameter)

        if lower_bound == upper_bound and lower_bound != 0:
            # fixed parameter, is an additional stats entry
            continue

        parameters[name] = value

    return parameters


def get_stats_from_tgraph(tgraph):
    """Get the summary statistics from the graph"""
    stats = collections.OrderedDict()

    additional_stats = ValidationPlot.get_additional_stats(tgraph)
    if additional_stats:
        for key, value in list(additional_stats.items()):
            stats[key] = value

    return stats


def get_stats_from_th(th):
    """Get the summary statistics from the histogram"""
    stats = collections.OrderedDict()

    stats["count"] = th.GetEntries()

    additional_stats = ValidationPlot.get_additional_stats(th)
    if additional_stats:
        for key, value in list(additional_stats.items()):
            stats[key] = value

    if not isinstance(th, (ROOT.TH2, ROOT.TH3)):
        x_taxis = th.GetXaxis()
        n_bins = x_taxis.GetNbins()

        if isinstance(th, ROOT.TProfile):
            underflow_content = th.GetBinEntries(0)
            overflow_content = th.GetBinEntries(n_bins + 1)
        else:
            underflow_content = th.GetBinContent(0)
            overflow_content = th.GetBinContent(n_bins + 1)

        if underflow_content:
            stats["x underf."] = underflow_content

        if overflow_content:
            stats["x overf."] = overflow_content

    stats_values = np.array([np.nan] * 7)
    th.GetStats(stats_values)

    sum_w = stats_values[0]
    sum_w2 = stats_values[1]
    sum_wx = stats_values[2]
    sum_wx2 = stats_values[3]
    sum_wy = stats_values[4]  # Only for TH2 and TProfile
    sum_wy2 = stats_values[5]  # Only for TH2 and TProfile
    sum_wxy = stats_values[6]  # Only for TH2

    if np.isnan(sum_wy):
        # Only one dimensional
        stats["x avg"] = np.divide(sum_wx, sum_w)
        stats["x std"] = np.divide(np.sqrt(sum_wx2 * sum_w - sum_wx * sum_wx), sum_w)

    else:
        # Only two dimensional
        stats["x avg"] = np.divide(sum_wx, sum_w)
        stats["x std"] = np.divide(np.sqrt(sum_wx2 * sum_w - sum_wx * sum_wx), sum_w)
        stats["y avg"] = np.divide(sum_wy, sum_w)
        stats["y std"] = np.divide(np.sqrt(sum_wy2 * sum_w - sum_wy * sum_wy), sum_w)

        if not np.isnan(sum_wxy):
            stats["cov"] = np.divide((sum_wxy * sum_w - sum_wx * sum_wy), (sum_w * sum_w))
            stats["corr"] = np.divide(stats["cov"], (stats["x std"] * stats["y std"]))

    return stats


def compose_stats_label(title, additional_stats={}):
    """Render the summary statistics to a label string."""
    keys = list(additional_stats.keys())
    labeled_value_template = "{0:<9}: {1:.3g}"
    labeled_string_template = "{0:<9}: {1:>9s}"
    label_elements = []
    if title:
        label_elements.append(str(title))

    for key, value in list(additional_stats.items()):
        if isinstance(value, str):
            label_element = labeled_string_template.format(key, value)
        else:
            label_element = labeled_value_template.format(key, value)
        label_elements.append(label_element)

    return "\n".join(label_elements)


def plot_tgraph_data_into(ax,
                          tgraph,
                          plot_errors=None,
                          label=None,
                          clip_to_data=True):
    """Plot a ROOT TGraph into a matplotlib axes

    Parameters
    ----------
    ax : matplotlib.axes.Axes
        An axes space in which to plot
    tgraph : ROOT.TGraph
       A plotable one dimensional ROOT histogram
    plot_errors : bool, optional
      Plot graph as errorbar plot. Default None means True for TGraphErrors and False else.
    label : str
      label to be given to the plot
    """

    plt.autoscale(tight=clip_to_data)

    if plot_errors is None:
        if isinstance(tgraph, ROOT.TGraphErrors):
            plot_errors = True
        else:
            plot_errors = False

    x_taxis = tgraph.GetXaxis()
    y_taxis = tgraph.GetYaxis()

    xlabel = reformat_root_latex_to_matplotlib_latex(x_taxis.GetTitle())
    ylabel = reformat_root_latex_to_matplotlib_latex(y_taxis.GetTitle())

    n_points = tgraph.GetN()

    xs = np.ndarray((n_points,), dtype=float)
    ys = np.ndarray((n_points,), dtype=float)

    x_lower_errors = np.ndarray((n_points,), dtype=float)
    x_upper_errors = np.ndarray((n_points,), dtype=float)

    y_lower_errors = np.ndarray((n_points,), dtype=float)
    y_upper_errors = np.ndarray((n_points,), dtype=float)

    x = ROOT.Double()
    y = ROOT.Double()

    for i_point in range(n_points):
        tgraph.GetPoint(i_point, x, y)
        xs[i_point] = float(x)
        ys[i_point] = float(y)

        x_lower_errors[i_point] = tgraph.GetErrorXlow(i_point)
        x_upper_errors[i_point] = tgraph.GetErrorXhigh(i_point)

        y_lower_errors[i_point] = tgraph.GetErrorYlow(i_point)
        y_upper_errors[i_point] = tgraph.GetErrorYhigh(i_point)

    if plot_errors:
        root_color_index = tgraph.GetLineColor()
        linecolor = root_color_to_matplot_color(root_color_index)

        ax.errorbar(xs,
                    ys,
                    xerr=[x_lower_errors, x_upper_errors],
                    yerr=[y_lower_errors, y_upper_errors],
                    fmt="none",
                    ecolor=linecolor,
                    label=label)

    else:
        root_color_index = tgraph.GetMarkerColor()
        markercolor = root_color_to_matplot_color(root_color_index)

        ax.scatter(xs,
                   ys,
                   c=markercolor,
                   s=2,
                   marker="+",
                   label=label)
        x_lower_bound, x_upper_bound = ax.get_xlim()
        x_lower_bound = min(x_lower_bound, np.nanmin(xs))

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    plt.autoscale(tight=None)


def plot_th1_data_into(ax,
                       th1,
                       plot_errors=None,
                       label=None):
    """Plot a ROOT histogram into a matplotlib axes

    Parameters
    ----------
    ax : matplotlib.axes.Axes
        An axes space in which to plot
    th1 : ROOT.TH1
       A plotable one dimensional ROOT histogram
    plot_errors : bool, optional
      Plot histogram as errorbar plot. Default None means True for TProfile and False else.
    label : str, optional
      label to be given to the histogram
    """

    if plot_errors is None:
        if isinstance(th1, ROOT.TProfile):
            plot_errors = True
        else:
            plot_errors = th1.GetSumw2N() != 0

    # Bin content
    x_taxis = th1.GetXaxis()
    n_bins = x_taxis.GetNbins()

    bin_ids_with_underflow = list(range(n_bins + 1))
    bin_ids_without_underflow = list(range(1, n_bins + 1))

    # Get the n_bins + 1 bin edges starting from the underflow bin 0
    bin_edges = np.array([x_taxis.GetBinUpEdge(i_bin) for i_bin in bin_ids_with_underflow])

    # Bin center and width not including the underflow
    bin_centers = np.array([x_taxis.GetBinCenter(i_bin) for i_bin in bin_ids_without_underflow])
    bin_widths = np.array([x_taxis.GetBinWidth(i_bin) for i_bin in bin_ids_without_underflow])
    bin_x_errors = bin_widths / 2.0

    # Now for the histogram content not including the underflow
    bin_contents = np.array([th1.GetBinContent(i_bin) for i_bin in bin_ids_without_underflow])
    bin_y_errors = np.array([th1.GetBinError(i_bin) for i_bin in bin_ids_without_underflow])
    bin_y_upper_errors = np.array([th1.GetBinErrorUp(i_bin) for i_bin in bin_ids_without_underflow])
    bin_y_lower_errors = np.array([th1.GetBinErrorLow(i_bin) for i_bin in bin_ids_without_underflow])

    empty_bins = (bin_contents == 0) & (bin_y_errors == 0)

    is_discrete_binning = bool(x_taxis.GetLabels())
    bin_labels = [x_taxis.GetBinLabel(i_bin) for i_bin in bin_ids_without_underflow]

    xlabel = reformat_root_latex_to_matplotlib_latex(x_taxis.GetTitle())

    y_taxis = th1.GetYaxis()
    ylabel = reformat_root_latex_to_matplotlib_latex(y_taxis.GetTitle())

    # May set these from th1 properties
    y_log_scale = False
    histtype = "step"

    root_color_index = th1.GetLineColor()
    linecolor = root_color_to_matplot_color(root_color_index)

    if plot_errors:
        ax.errorbar(bin_centers[~empty_bins],
                    bin_contents[~empty_bins],
                    yerr=[bin_y_lower_errors[~empty_bins],
                          bin_y_upper_errors[~empty_bins]],
                    xerr=bin_x_errors[~empty_bins],
                    fmt="none",
                    ecolor=linecolor,
                    label=label)

        y_lower_bound, y_upper_bound = common_bounds(
            ax.get_ylim(),
            (th1.GetMinimum(flt_min), th1.GetMaximum(flt_max))
        )

        y_total_width = y_upper_bound - y_lower_bound
        ax.set_ylim(y_lower_bound - 0.02 * y_total_width, y_upper_bound + 0.02 * y_total_width)

    else:
        if is_discrete_binning:
            ax.bar(bin_centers - 0.4,
                   bin_contents,
                   width=0.8,
                   label=label,
                   edgecolor=linecolor,
                   color=(1, 1, 1, 0),  # fill transparent white
                   log=y_log_scale)
        else:
            ax.hist(bin_centers,
                    bins=bin_edges,
                    weights=bin_contents,
                    edgecolor=linecolor,
                    histtype=histtype,
                    label=label,
                    log=y_log_scale)

        # Fixing that the limits sometimes clip in the y direction
        ax.set_ylim(0, 1.02 * max(bin_contents))

    if is_discrete_binning:
        ax.set_xticks(bin_centers)
        ax.set_xticklabels(bin_labels, rotation=0)

    total_width = bin_edges[-1] - bin_edges[0]
    if total_width != 0:
        ax.set_xlim(bin_edges[0] - 0.01 * total_width, bin_edges[-1] + 0.01 * total_width)

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)


def plot_th2_data_into(ax,
                       th2,
                       plot_3d=False,
                       label=None):
    """Plot a ROOT histogram into a matplotlib axes

    Parameters
    ----------
    ax : matplotlib.axes.Axes
        An axes space in which to plot
    th2 : ROOT.TH2
       A plotable two dimensional ROOT histogram
    plot_3d : bool, optional
      Plot as a three dimensional plot
    label : str, optional
      label to be given to the histogram
    """

    # Bin content
    x_taxis = th2.GetXaxis()
    y_taxis = th2.GetYaxis()

    x_n_bins = x_taxis.GetNbins()
    y_n_bins = y_taxis.GetNbins()

    x_bin_ids_with_underflow = list(range(x_n_bins + 1))
    y_bin_ids_with_underflow = list(range(y_n_bins + 1))

    x_bin_ids_without_underflow = list(range(1, x_n_bins + 1))
    y_bin_ids_without_underflow = list(range(1, y_n_bins + 1))

    # Get the n_bins + 1 bin edges starting from the underflow bin 0
    x_bin_edges = np.array([x_taxis.GetBinUpEdge(i_bin) for i_bin in x_bin_ids_with_underflow])
    y_bin_edges = np.array([y_taxis.GetBinUpEdge(i_bin) for i_bin in y_bin_ids_with_underflow])

    # Bin center and width not including the underflow
    x_bin_centers = np.array([x_taxis.GetBinCenter(i_bin) for i_bin in x_bin_ids_without_underflow])
    y_bin_centers = np.array([y_taxis.GetBinCenter(i_bin) for i_bin in y_bin_ids_without_underflow])

    x_centers, y_centers = np.meshgrid(x_bin_centers, y_bin_centers)

    x_ids, y_ids = np.meshgrid(x_bin_ids_without_underflow, y_bin_ids_without_underflow)

    bin_contents = np.array([th2.GetBinContent(int(x_i_bin), int(y_i_bin))
                             for x_i_bin, y_i_bin in zip(x_ids.flat, y_ids.flat)])

    x_is_discrete_binning = bool(x_taxis.GetLabels())
    x_bin_labels = [x_taxis.GetBinLabel(i_bin) for i_bin in x_bin_ids_without_underflow]

    y_is_discrete_binning = bool(y_taxis.GetLabels())
    y_bin_labels = [y_taxis.GetBinLabel(i_bin) for i_bin in y_bin_ids_without_underflow]

    xlabel = reformat_root_latex_to_matplotlib_latex(x_taxis.GetTitle())
    ylabel = reformat_root_latex_to_matplotlib_latex(y_taxis.GetTitle())

    # May set these from th2 properties
    log_scale = False

    root_color_index = th2.GetLineColor()
    linecolor = root_color_to_matplot_color(root_color_index)

    if plot_3d:
        raise NotImplementedError("3D plotting of two dimensional histograms not implemented yet")

    else:
        if log_scale:
            _, _, _, colorbar_mappable = ax.hist2d(x_centers.flatten(),
                                                   y_centers.flatten(),
                                                   weights=bin_contents,
                                                   bins=[x_bin_edges, y_bin_edges],
                                                   label=label,
                                                   norm=matplotlib.colors.LogNorm())
        else:
            _, _, _, colorbar_mappable = ax.hist2d(x_centers.flatten(),
                                                   y_centers.flatten(),
                                                   weights=bin_contents,
                                                   bins=[x_bin_edges, y_bin_edges],
                                                   label=label)

        lowest_color = colorbar_mappable.get_cmap()(0)

        # Dummy artist to show in legend
        ax.fill(0, 0, "-", color=lowest_color, label=label)

        # For colorbar on the left
        # cbar_ax, _ = matplotlib.colorbar.make_axes(ax,
        #                                            location="left",
        #                                            )

        colorbar_ax, _ = matplotlib.colorbar.make_axes(ax,
                                                       pad=0.02,
                                                       shrink=0.5,
                                                       anchor=(0.0, 1.0),
                                                       panchor=(1.0, 1.0),
                                                       aspect=10,
                                                       )

        matplotlib.colorbar.Colorbar(colorbar_ax, colorbar_mappable)

    if x_is_discrete_binning:
        ax.set_xticks(x_bin_centers)
        ax.set_xticklabels(x_bin_labels, rotation=0)

    if y_is_discrete_binning:
        ax.set_yticks(y_bin_centers)
        ax.set_yticklabels(y_bin_labels, rotation=0)

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)


def plot_tf1_data_into(ax,
                       tf1,
                       label=None):
    """Plots the data of the tf1 into a matplotlib axes

    Parameters
    ----------
    ax : matplotlib.axes.Axes
        An axes space in which to plot
    tf1 : ROOT.TF1
        Function to be ploted.
    label : str, optional
        Label for the legend entry.
    """

    lower_bound = tf1.GetXmin()
    upper_bound = tf1.GetXmin()
    n_plot_points = max(tf1.GetNpx(), 100)

    if lower_bound == upper_bound:
        lower_bound, upper_bound = ax.get_xlim()

    xs = np.linspace(lower_bound, upper_bound, n_plot_points)
    ys = [tf1.Eval(x) for x in xs]

    root_color_index = tf1.GetLineColor()
    linecolor = root_color_to_matplot_color(root_color_index)

    if any(y != 0 for y in ys):
        line_handles = ax.plot(xs, ys, color=linecolor, label=label)
        return line_handles


def root_color_to_matplot_color(root_color_index):
    """Translates ROOT color into an RGB tuple.

    Parameters
    ----------
    root_color_index : int
        Index  of a color as defined in ROOT

    Returns
    -------
    (float, float, float)
        tuple of floats that represent to RGB color fractions.
    """
    tcolor = ROOT.gROOT.GetColor(root_color_index)
    return (tcolor.GetRed(), tcolor.GetGreen(), tcolor.GetBlue())


def reformat_root_latex_to_matplotlib_latex(text):
    """Takes text that may contain ROOT pseudo latex directives and
    translate it in to proper latex that can be understood by matplotlib"""

    # Dumb implementation, can be improved a lot
    # Splits by white space and try to treat every part separatly.
    # Additionally a dump regular expression replacement for ROOT latex directives
    # is applied.
    # It may loose some context,
    # but I could not bother to implement a parser for ROOTs latex dialect.

    text_parts = text.split(" ")

    reformatted_text_parts = []

    for text_part in text_parts:
        if is_root_latex_directive(text_part):
            # All directive are wrapped into math mode
            reformatted_text_part = r'$' + text_part.replace('#', '\\') + r'$'
            # print 'Format' , text_part ,'to' , reformatted_text_part
        else:
            reformatted_text_part = text_part

        reformatted_text_part = re.sub("#([a-zA-Z_{}]*)", r"$\\\1$", reformatted_text_part)

        reformatted_text_parts.append(reformatted_text_part)

    return " ".join(reformatted_text_parts)


def is_root_latex_directive(text_part):
    """Test if a text part looks like a ROOT latex directive"""
    return text_part.startswith('#') or '_{' in text_part or '{}' in text_part


def common_bounds(matplot_bounds, root_bounds):
    lower_bound, upper_bound = matplot_bounds
    root_lower_bound, root_upper_bound = root_bounds

    if root_lower_bound != 0 or root_upper_bound != 0:
        lower_bound = np.nanmin((lower_bound, root_lower_bound))
        upper_bound = np.nanmax((upper_bound, root_upper_bound))

    return lower_bound, upper_bound
