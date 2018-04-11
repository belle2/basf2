#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2015

import copy
import math

import numpy
import numpy as np
import matplotlib

# Do not use standard backend TkAgg, because it is NOT thread-safe
# You will get an RuntimeError: main thread is not in main loop otherwise!
matplotlib.use("svg")
matplotlib.rcParams.update({'font.size': 36})

import matplotlib.pyplot as plt
import matplotlib.artist
import matplotlib.figure
import matplotlib.gridspec
import matplotlib.colors
import matplotlib.patches
import matplotlib.ticker

from . import histogram

from basf2 import *

import basf2_mva_util


class Plotter(object):
    """
    Base class for all Plotters.
    """

    # stupid workaround for doxygen refusing to document things

    #: @fn set_errorbar_options(errorbar_kwargs)
    #: Overrides default errorbar options for datapoint errorbars

    #: @var xscale
    #: limit scale
    #: @var yscale
    #: limit scale

    #: Plots added to the axis so far
    plots = None
    #: Labels of the plots added so far
    labels = None
    #: Minimum x value
    xmin = None
    #: Maximum x value
    xmax = None
    #: Minimum y value
    ymin = None
    #: Maximum y value
    ymax = None
    yscale = 0.0
    xscale = 0.0
    #: figure which is used to draw
    figure = None
    #: Main axis which is used to draw
    axis = None

    def __init__(self, figure=None, axis=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        """
        B2INFO("Create new figure for class " + str(type(self)))
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(32, 18))
            self.figure.set_tight_layout(False)
        else:
            self.figure = figure

        if axis is None:
            self.axis = self.figure.add_subplot(1, 1, 1)
        else:
            self.axis = axis

        self.plots = []
        self.labels = []
        self.xmin, self.xmax = float(0), float(1)
        self.ymin, self.ymax = float(0), float(1)
        #: y limit scale
        self.yscale = 0.1
        #: x limit scale
        self.xscale = 0.0

        #: Default keyword arguments for plot function
        self.plot_kwargs = None
        #: Default keyword arguments for errorbar function
        self.errorbar_kwargs = None
        #: Default keyword arguments for errorband function
        self.errorband_kwargs = None
        #: Default keyword arguments for fill_between function
        self.fill_kwargs = None

        self.set_plot_options()
        self.set_errorbar_options()
        self.set_errorband_options()
        self.set_fill_options()

    def add_subplot(self, gridspecs):
        """
        Adds a new subplot to the figure, updates all other axes
        according to the given gridspec
        @param gridspecs gridspecs for all axes including the new one
        """
        for gs, ax in zip(gridspecs[:-1], self.figure.axes):
            ax.set_position(gs.get_position(self.figure))
            ax.set_subplotspec(gs)
        axis = self.figure.add_subplot(gridspecs[-1], sharex=self.axis)
        return axis

    def save(self, filename):
        """
        Save the figure into a file
        @param filename of the file
        """
        B2INFO("Save figure for class " + str(type(self)))
        from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
        canvas = FigureCanvas(self.figure)
        canvas.print_figure(filename, dpi=50)
        return self

    def set_plot_options(self, plot_kwargs={'linestyle': ''}):
        """
        Overrides default plot options for datapoint plot
        @param plot_kwargs keyword arguments for the plot function
        """
        self.plot_kwargs = copy.copy(plot_kwargs)
        return self

    def set_errorbar_options(self, errorbar_kwargs={'fmt': '.', 'elinewidth': 3, 'alpha': 1}):
        """
        Overrides default errorbar options for datapoint errorbars
        @param errorbar_kwargs keyword arguments for the errorbar function
        """
        self.errorbar_kwargs = copy.copy(errorbar_kwargs)
        return self

    def set_errorband_options(self, errorband_kwargs={'alpha': 0.5}):
        """
        Overrides default errorband options for datapoint errorband
        @param errorbar_kwargs keyword arguments for the fill_between function
        """
        self.errorband_kwargs = copy.copy(errorband_kwargs)
        return self

    def set_fill_options(self, fill_kwargs=None):
        """
        Overrides default fill_between options for datapoint errorband
        @param fill_kwargs keyword arguments for the fill_between function
        """
        self.fill_kwargs = copy.copy(fill_kwargs)
        return self

    def _plot_datapoints(self, axis, x, y, xerr=None, yerr=None):
        """
        Plot the given datapoints, with plot, errorbar and make a errorband with fill_between
        @param x coordinates of the data points
        @param y coordinates of the data points
        @param xerr symmetric error on x data points
        @param yerr symmetric error on y data points
        """
        p = e = f = None
        plot_kwargs = copy.copy(self.plot_kwargs)
        errorbar_kwargs = copy.copy(self.errorbar_kwargs)
        errorband_kwargs = copy.copy(self.errorband_kwargs)
        fill_kwargs = copy.copy(self.fill_kwargs)

        if plot_kwargs is None or 'color' not in plot_kwargs:
            color = next(axis._get_lines.prop_cycler)
            color = color['color']
            plot_kwargs['color'] = color
        else:
            color = plot_kwargs['color']
        color = matplotlib.colors.ColorConverter().to_rgb(color)
        patch = matplotlib.patches.Patch(color=color, alpha=0.5)
        patch.get_color = patch.get_facecolor
        patches = [patch]

        if plot_kwargs is not None:
            p, = axis.plot(x, y, rasterized=True, **plot_kwargs)
            patches.append(p)

        if errorbar_kwargs is not None and (xerr is not None or yerr is not None):
            if 'color' not in errorbar_kwargs:
                errorbar_kwargs['color'] = color
            if 'ecolor' not in errorbar_kwargs:
                errorbar_kwargs['ecolor'] = [0.5 * x for x in color]
            e = axis.errorbar(x, y, xerr=xerr, yerr=yerr, rasterized=True, **errorbar_kwargs)
            patches.append(e)

        if errorband_kwargs is not None and yerr is not None:
            if 'color' not in errorband_kwargs:
                errorband_kwargs['color'] = color
            if xerr is not None:
                # Ensure that xerr and yerr are iterable numpy arrays
                xerr = x + xerr - x
                yerr = y + yerr - y
                for _x, _y, _xe, _ye in zip(x, y, xerr, yerr):
                    axis.add_patch(matplotlib.patches.Rectangle((_x - _xe, _y - _ye), 2 * _xe, 2 * _ye, rasterized=True,
                                                                **errorband_kwargs))
            else:
                f = axis.fill_between(x, y - yerr, y + yerr, interpolate=True, rasterized=True, **errorband_kwargs)

        if fill_kwargs is not None:
            axis.fill_between(x, y, 0, rasterized=True, **fill_kwargs)

        return (tuple(patches), p, e, f)

    def add(self, *args, **kwargs):
        """
        Add a new plot to this plotter
        """
        return NotImplemented

    def finish(self, *args, **kwargs):
        """
        Finish plotting and set labels, legends and stuff
        """
        return NotImplemented

    def scale_limits(self):
        """
        Scale limits to increase distance to boundaries
        """
        self.ymin *= 1.0 - math.copysign(self.yscale, self.ymin)
        self.ymax *= 1.0 + math.copysign(self.yscale, self.ymax)
        self.xmin *= 1.0 - math.copysign(self.xscale, self.xmin)
        self.xmax *= 1.0 + math.copysign(self.xscale, self.xmax)
        return self


class PurityAndEfficiencyOverCut(Plotter):
    """
    Plots the purity and the efficiency over the cut value (for cut choosing)
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, normed=True):
        """
        Add a new curve to the plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate efficiency and purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """

        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)

        if normed:
            efficiency, efficiency_error = hists.get_efficiency(['Signal'])
            purity, purity_error = hists.get_purity(['Signal'], ['Background'])
        else:
            efficiency, efficiency_error = hists.get_true_positives(['Signal'])
            purity, purity_error = hists.get_false_positives(['Background'])

        cuts = hists.bin_centers

        self.xmin, self.xmax = numpy.nanmin([numpy.nanmin(cuts), self.xmin]), numpy.nanmax([numpy.nanmax(cuts), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(efficiency), numpy.nanmin(purity), self.ymin]), \
            numpy.nanmax([numpy.nanmax(efficiency), numpy.nanmax(purity), self.ymax])

        self.plots.append(self._plot_datapoints(self.axis, cuts, efficiency, xerr=0, yerr=efficiency_error))

        if normed:
            self.labels.append("Efficiency")
        else:
            self.labels.append("True positive")

        self.plots.append(self._plot_datapoints(self.axis, cuts, purity, xerr=0, yerr=purity_error))

        if normed:
            self.labels.append("Purity")
        else:
            self.labels.append("False positive")

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Classification Plot")
        self.axis.get_xaxis().set_label_text('Cut Value')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class SignalToNoiseOverCut(Plotter):
    """
    Plots the signal to noise ratio over the cut value (for cut choosing)
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, normed=True):
        """
        Add a new curve to the plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate signal to noise ratio for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """

        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)

        signal2noise, signal2noise_error = hists.get_signal_to_noise(['Signal'], ['Background'])

        cuts = hists.bin_centers

        self.xmin, self.xmax = numpy.nanmin([numpy.nanmin(cuts), self.xmin]), numpy.nanmax([numpy.nanmax(cuts), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(signal2noise), self.ymin]), \
            numpy.nanmax([numpy.nanmax(signal2noise), self.ymax])

        self.plots.append(self._plot_datapoints(self.axis, cuts, signal2noise, xerr=0, yerr=signal2noise_error))

        self.labels.append(column)

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Signal to Noise Plot")
        self.axis.get_xaxis().set_label_text('Cut Value')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class PurityOverEfficiency(Plotter):
    """
    Plots the purity over the efficiency also known as ROC curve
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, label=None):
        """
        Add a new curve to the ROC plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate efficiency and purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        purity, purity_error = hists.get_purity(['Signal'], ['Background'])

        self.xmin, self.xmax = numpy.nanmin([efficiency.min(), self.xmin]), numpy.nanmax([efficiency.max(), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(purity), self.ymin]), numpy.nanmax([numpy.nanmax(purity), self.ymax])

        p = self._plot_datapoints(self.axis, efficiency, purity, xerr=efficiency_error, yerr=purity_error)
        self.plots.append(p)
        if label is not None:
            self.labels.append(label)
        else:
            self.labels.append(column)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("ROC Purity Plot")
        self.axis.get_xaxis().set_label_text('Efficiency')
        self.axis.get_yaxis().set_label_text('Purity')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class RejectionOverEfficiency(Plotter):
    """
    Plots the rejection over the efficiency also known as ROC curve
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, label=None):
        """
        Add a new curve to the ROC plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate efficiency and purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        rejection, rejection_error = hists.get_efficiency(['Background'])
        rejection = 1 - rejection

        self.xmin, self.xmax = numpy.nanmin([efficiency.min(), self.xmin]), numpy.nanmax([efficiency.max(), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([rejection.min(), self.ymin]), numpy.nanmax([rejection.max(), self.ymax])

        auc = numpy.abs(numpy.trapz(rejection, efficiency))

        p = self._plot_datapoints(self.axis, efficiency, rejection, xerr=efficiency_error, yerr=rejection_error)
        self.plots.append(p)
        if label is not None:
            self.labels.append(label[:10] + " ({:.2f})".format(auc))
        else:
            self.labels.append(column[:10] + " ({:.2f})".format(auc))
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("ROC Rejection Plot")
        self.axis.get_xaxis().set_label_text('Signal Efficiency')
        self.axis.get_yaxis().set_label_text('Background Rejection')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Multiplot(Plotter):
    """
    Plots multiple other plots into a grid 3x?
    """
    #: figure which is used to draw
    figure = None
    #: Main axis
    axis = None

    def __init__(self, cls, number_of_plots, figure=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        """
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(32, 18))
            self.figure.set_tight_layout(True)
        else:
            self.figure = figure

        if number_of_plots == 1:
            gs = matplotlib.gridspec.GridSpec(1, 1)
        elif number_of_plots == 2:
            gs = matplotlib.gridspec.GridSpec(1, 2)
        elif number_of_plots == 3:
            gs = matplotlib.gridspec.GridSpec(1, 3)
        else:
            gs = matplotlib.gridspec.GridSpec(int(numpy.ceil(number_of_plots / 3)), 3)

        #: the subplots which are displayed in the grid
        self.sub_plots = [cls(self.figure, self.figure.add_subplot(gs[i // 3, i % 3])) for i in range(number_of_plots)]
        self.axis = self.sub_plots[0].axis
        super(Multiplot, self).__init__(self.figure, self.axis)

    def add(self, i, *args, **kwargs):
        """
        Call add function of ith subplot
        @param i position of the subplot
        """
        self.sub_plots[i].add(*args, **kwargs)

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        for plot in self.sub_plots:
            plot.finish()
        return self


class Diagonal(Plotter):
    """
    Plots the purity in each bin over the classifier output.
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        """
        Add a new curve to the Diagonal plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        purity, purity_error = hists.get_purity_per_bin(['Signal'], ['Background'])

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        # self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(purity), self.ymin]), numpy.nanmax([numpy.nanmax(purity), self.ymax])
        self.ymin, self.ymax = 0, 1

        p = self._plot_datapoints(self.axis, hists.bin_centers, purity, xerr=hists.bin_widths / 2.0, yerr=purity_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.scale_limits()
        self.axis.plot((0.0, 1.0), (0.0, 1.0), color='black')
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Diagonal Plot")
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.get_yaxis().set_label_text('Purity Per Bin')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Distribution(Plotter):
    """
    Plots distribution of a quantity
    """

    def __init__(self, figure=None, axis=None, normed_to_all_entries=False, normed_to_bin_width=False,
                 keep_first_binning=False, range_in_std=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param normed true if histograms should be normed before drawing
        @param keep_first_binning use the binning of the first distribution for further plots
        @param range_in_std show only the data in a windows around +- range_in_std * standard_deviation around the mean
        """
        super(Distribution, self).__init__(figure, axis)
        #: Normalize histograms before drawing them
        self.normed_to_all_entries = normed_to_all_entries
        #: Normalize histograms before drawing them
        self.normed_to_bin_width = normed_to_bin_width
        #: Show only a certain range in terms of standard deviations of the data
        self.range_in_std = range_in_std
        # if self.normed_to_all_entries or self.normed_to_bin_width:
        #: size in x/y
        self.ymin = float(0)
        #: size in x/y
        self.ymax = float('-inf')
        #: size in x/y
        self.xmin = float('inf')
        #: size in x/y
        self.xmax = float('-inf')
        #: Keep first binning if user wants so
        self.keep_first_binning = keep_first_binning
        #: first binning
        self.first_binning = None
        #: x axis label
        self.x_axis_label = ''

    def add(self, data, column, mask=None, weight_column=None, label=None):
        """
        Add a new distribution to the plots
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param mask boolean numpy.array defining which events are used for the histogram
        @param weight_column column in data containing the weights for each event
        """
        if mask is None:
            mask = numpy.ones(len(data)).astype('bool')

        bins = 100
        if self.keep_first_binning and self.first_binning is not None:
            bins = self.first_binning
        hists = histogram.Histograms(data, column, {'Total': mask}, weight_column=weight_column,
                                     bins=bins, equal_frequency=False, range_in_std=self.range_in_std)
        if self.keep_first_binning and self.first_binning is None:
            self.first_binning = hists.bins
        hist, hist_error = hists.get_hist('Total')

        if self.normed_to_all_entries:
            normalization = float(numpy.sum(hist))
            hist = hist / normalization
            hist_error = hist_error / normalization

        if self.normed_to_bin_width:
            hist = hist / hists.bin_widths
            hist_error = hist_error / hists.bin_widths

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin = numpy.nanmin([hist.min(), self.ymin])
        self.ymax = numpy.nanmax([(hist + hist_error).max(), self.ymax, self.ymin])

        p = self._plot_datapoints(self.axis, hists.bin_centers, hist, xerr=hists.bin_widths / 2, yerr=hist_error)
        self.plots.append(p)
        self.x_axis_label = column
        if label is None:
            self.labels.append(column)
        else:
            self.labels.append(label)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.scale_limits()
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Distribution Plot")
        self.axis.get_xaxis().set_label_text(self.x_axis_label)
        if self.normed_to_all_entries and self.normed_to_bin_width:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / (# Entries * Bin Width)')
        elif self.normed_to_all_entries:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / # Entries')
        elif self.normed_to_bin_width:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / Bin Width')
        else:
            self.axis.get_yaxis().set_label_text('# Entries per Bin')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Box(Plotter):
    """
    Create a boxplot
    """
    #: @var x_axis_label
    #: Label on x axis

    def __init__(self, figure=None, axis=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        """
        super().__init__(figure=figure, axis=axis)

        #: Label on x axis
        self.x_axis_label = ""

    def add(self, data, column, mask=None, weight_column=None):
        """
        Add a new boxplot to the plots
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate boxplot quantities
        @param mask boolean numpy.array defining which events are used for the histogram
        @param weight_column column in data containing the weights for each event
        """
        if mask is None:
            mask = numpy.ones(len(data)).astype('bool')
        x = data[column][mask]
        if weight_column is not None:
            weight = data[weight_column][mask]
            B2WARNING("Weights are currently not used in boxplot, due to limitations in matplotlib")

        if len(x) == 0:
            B2WARNING("Ignore empty boxplot.")
            return self

        p = self.axis.boxplot(x, sym='k.', whis=1.5, vert=False, patch_artist=True, showmeans=True, widths=1,
                              boxprops=dict(facecolor='blue', alpha=0.5),
                              # medianprobs=dict(color='blue'),
                              # meanprobs=dict(color='red'),
                              )
        self.plots.append(p)
        self.labels.append(column)
        self.x_axis_label = column
        """
        self.axis.text(0.1, 0.9, (r'$     \mu = {:.2f}$' + '\n' + r'$median = {:.2f}$').format(x.mean(), x.median()),
                       fontsize=28, verticalalignment='top', horizontalalignment='left', transform=self.axis.transAxes)
        self.axis.text(0.4, 0.9, (r'$  \sigma = {:.2f}$' + '\n' + r'$IQD = {:.2f}$').format(x.std(),
                                                                                            x.quantile(0.75) - x.quantile(0.25)),
                       fontsize=28, verticalalignment='top', horizontalalignment='left', transform=self.axis.transAxes)
        self.axis.text(0.7, 0.9, (r'$min = {:.2f}$' + '\n' + r'$max = {:.2f}$').format(x.min(), x.max()),
                       fontsize=28, verticalalignment='top', horizontalalignment='left', transform=self.axis.transAxes)
        """

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        matplotlib.artist.setp(self.axis.get_yaxis(), visible=False)
        self.axis.get_xaxis().set_label_text(self.x_axis_label)
        self.axis.set_title("Box Plot")
        return self


class Difference(Plotter):
    """
    Plots the difference between two histograms
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value
    #: @var ymin
    #: min y value
    #: @var x_axis_label
    #: Label on x axis
    #: @var normed
    #: Minuend and subtrahend are normed before comparing them if this is true
    #: @var shift_to_zero
    #: Mean difference is shifted to zero (removes constant offset) if this is true

    def __init__(self, figure=None, axis=None, normed=False, shift_to_zero=False):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param normed normalize minuend and subtrahend before comparing them
        @param shift_to_zero mean difference is shifted to zero, to remove constant offset due to e.g. different sample sizes
        """
        super(Difference, self).__init__(figure, axis)
        self.normed = normed
        self.shift_to_zero = shift_to_zero
        if self.normed:
            self.ymin = -0.01
            self.ymax = 0.01
        else:
            self.ymin = -1
            self.ymax = 1

    def add(self, data, column, minuend_mask, subtrahend_mask, weight_column=None, label=None):
        """
        Add a new difference plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param minuend_mask boolean numpy.array defining which events are for the minuend histogram
        @param subtrahend_mask boolean numpy.array defining which events are for the subtrahend histogram
        @param weight_column column in data containing the weights for each event
        @param label label for the legend if None, the column name is used
        """
        hists = histogram.Histograms(data, column, {'Minuend': minuend_mask, 'Subtrahend': subtrahend_mask},
                                     weight_column=weight_column, equal_frequency=False)
        minuend, minuend_error = hists.get_hist('Minuend')
        subtrahend, subtrahend_error = hists.get_hist('Subtrahend')

        difference_error = histogram.poisson_error(minuend + subtrahend)
        if self.normed:
            difference_error = difference_error / (numpy.sum(minuend) + numpy.sum(subtrahend))
            minuend = minuend / numpy.sum(minuend)
            subtrahend = subtrahend / numpy.sum(subtrahend)
        difference = minuend - subtrahend

        if self.shift_to_zero:
            difference = difference - numpy.mean(difference)

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin = min((difference - difference_error).min(), self.ymin)
        self.ymax = max((difference + difference_error).max(), self.ymax)

        p = self._plot_datapoints(self.axis, hists.bin_centers, difference, xerr=hists.bin_widths / 2, yerr=difference_error)
        self.plots.append(p)
        if label is None:
            self.labels.append(label)
        else:
            self.labels.append(column)
        self.x_axis_label = column
        return self

    def finish(self, line_color='black'):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.plot((self.xmin, self.xmax), (0, 0), color=line_color, linewidth=4, rasterized=True)
        self.scale_limits()
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Difference Plot")
        self.axis.get_yaxis().set_major_locator(matplotlib.ticker.MaxNLocator(5))
        self.axis.get_xaxis().set_label_text(self.x_axis_label)
        self.axis.get_yaxis().set_label_text('Difference')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Overtraining(Plotter):
    """
    Create TMVA-like overtraining control plot for a classification training
    """

    #: figure which is used to draw
    figure = None
    #: Main axis which is used to draw
    axis = None
    #: Axis which shows the difference between training and test signal
    axis_d1 = None
    #: Axis which shows the difference between training and test background
    axis_d2 = None

    def __init__(self, figure=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        """
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(32, 18))
            self.figure.set_tight_layout(True)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(5, 1)
        self.axis = self.figure.add_subplot(gs[:3, :])
        self.axis_d1 = self.figure.add_subplot(gs[3, :], sharex=self.axis)
        self.axis_d2 = self.figure.add_subplot(gs[4, :], sharex=self.axis)

        super(Overtraining, self).__init__(self.figure, self.axis)

    def add(self, data, column, train_mask, test_mask, signal_mask, bckgrd_mask, weight_column=None):
        """
        Add a new overtraining plot, I recommend to raw only one overtraining plot at the time,
        otherwise there are too many curves in the plot to reconize anything in the plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param train_mask boolean numpy.array defining which events are training events
        @param test_mask boolean numpy.array defining which events are test events
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        distribution = Distribution(self.figure, self.axis, normed_to_all_entries=True)

        distribution.set_plot_options(self.plot_kwargs)
        distribution.set_errorbar_options(self.errorbar_kwargs)
        distribution.set_errorband_options(self.errorband_kwargs)
        distribution.add(data, column, test_mask & signal_mask, weight_column)
        distribution.add(data, column, test_mask & bckgrd_mask, weight_column)

        distribution.set_plot_options({'color': distribution.plots[0][0][0].get_color(), 'linestyle': 'steps-mid-', 'lw': 4})
        distribution.set_fill_options({'color': distribution.plots[0][0][0].get_color(), 'alpha': 0.5, 'step': 'mid'})
        distribution.set_errorbar_options(None)
        distribution.set_errorband_options(None)
        distribution.add(data, column, train_mask & signal_mask, weight_column)
        distribution.set_plot_options({'color': distribution.plots[1][0][0].get_color(), 'linestyle': 'steps-mid-', 'lw': 4})
        distribution.set_fill_options({'color': distribution.plots[1][0][0].get_color(), 'alpha': 0.5, 'step': 'mid'})
        distribution.add(data, column, train_mask & bckgrd_mask, weight_column)

        distribution.labels = ['Test-Signal', 'Test-Background', 'Train-Signal', 'Train-Background']
        distribution.finish()

        self.plot_kwargs['color'] = distribution.plots[0][0][0].get_color()
        difference_signal = Difference(self.figure, self.axis_d1, shift_to_zero=True, normed=True)
        difference_signal.set_plot_options(self.plot_kwargs)
        difference_signal.set_errorbar_options(self.errorbar_kwargs)
        difference_signal.set_errorband_options(self.errorband_kwargs)
        difference_signal.add(data, column, train_mask & signal_mask, test_mask & signal_mask, weight_column)
        self.axis_d1.set_xlim((difference_signal.xmin, difference_signal.xmax))
        self.axis_d1.set_ylim((difference_signal.ymin, difference_signal.ymax))
        difference_signal.plots = difference_signal.labels = []
        difference_signal.finish(line_color=distribution.plots[0][0][0].get_color())

        self.plot_kwargs['color'] = distribution.plots[1][0][0].get_color()
        difference_bckgrd = Difference(self.figure, self.axis_d2, shift_to_zero=True, normed=True)
        difference_bckgrd.set_plot_options(self.plot_kwargs)
        difference_bckgrd.set_errorbar_options(self.errorbar_kwargs)
        difference_bckgrd.set_errorband_options(self.errorband_kwargs)
        difference_bckgrd.add(data, column, train_mask & bckgrd_mask, test_mask & bckgrd_mask, weight_column)
        self.axis_d2.set_xlim((difference_bckgrd.xmin, difference_bckgrd.xmax))
        self.axis_d2.set_ylim((difference_bckgrd.ymin, difference_bckgrd.ymax))
        difference_bckgrd.plots = difference_bckgrd.labels = []
        difference_bckgrd.finish(line_color=distribution.plots[1][0][0].get_color())

        try:
            import scipy.stats
            # Kolmogorov smirnov test
            if len(data[column][train_mask & signal_mask]) == 0 or len(data[column][test_mask & signal_mask]) == 0:
                B2WARNING("Cannot calculate kolmogorov smirnov test for signal due to missing data")
            else:
                ks = scipy.stats.ks_2samp(data[column][train_mask & signal_mask], data[column][test_mask & signal_mask])
                props = dict(boxstyle='round', edgecolor='gray', facecolor='white', linewidth=0.1, alpha=0.5)
                self.axis_d1.text(0.1, 0.9, r'signal (train - test) difference $p={:.2f}$'.format(ks[1]), fontsize=36, bbox=props,
                                  verticalalignment='top', horizontalalignment='left', transform=self.axis_d1.transAxes)
            if len(data[column][train_mask & bckgrd_mask]) == 0 or len(data[column][test_mask & bckgrd_mask]) == 0:
                B2WARNING("Cannot calculate kolmogorov smirnov test for background due to missing data")
            else:
                ks = scipy.stats.ks_2samp(data[column][train_mask & bckgrd_mask], data[column][test_mask & bckgrd_mask])
                props = dict(boxstyle='round', edgecolor='gray', facecolor='white', linewidth=0.1, alpha=0.5)
                self.axis_d2.text(0.1, 0.9, r'background (train - test) difference $p={:.2f}$'.format(ks[1]), fontsize=36,
                                  bbox=props,
                                  verticalalignment='top', horizontalalignment='left', transform=self.axis_d2.transAxes)
        except ImportError:
            B2WARNING("Cannot calculate kolmogorov smirnov test please install scipy!")

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_title("Overtraining Plot")
        self.axis_d1.set_title("")
        self.axis_d2.set_title("")
        matplotlib.artist.setp(self.axis.get_xticklabels(), visible=False)
        matplotlib.artist.setp(self.axis_d1.get_xticklabels(), visible=False)
        self.axis.get_xaxis().set_label_text('')
        self.axis_d1.get_xaxis().set_label_text('')
        self.axis_d2.get_xaxis().set_label_text('Classifier Output')
        return self


class VerboseDistribution(Plotter):
    """
    Plots distribution of a quantity including boxplots
    """

    #: Axes for the boxplots
    box_axes = None

    def __init__(self, figure=None, axis=None, normed=False, range_in_std=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param normed true if the histograms should be normed before drawing
        @param range_in_std show only the data in a windows around +- range_in_std * standard_deviation around the mean
        """
        super(VerboseDistribution, self).__init__(figure, axis)
        #: Normalize histograms before drawing them
        self.normed = normed
        #: Show only a certain range in terms of standard deviations of the data
        self.range_in_std = range_in_std
        self.box_axes = []
        #: The distribution plot
        self.distribution = Distribution(self.figure, self.axis, normed_to_all_entries=self.normed, range_in_std=self.range_in_std)

    def add(self, data, column, mask=None, weight_column=None, label=None):
        """
        Add a new distribution plot, with additional information like a boxplot compared to
        the ordinary Distribution plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param mask boolean numpy.array defining which events are used for the distribution histogram
        @param weight_column column in data containing the weights for each event
        """
        self.distribution.set_plot_options(self.plot_kwargs)
        self.distribution.set_errorbar_options(self.errorbar_kwargs)
        self.distribution.set_errorband_options(self.errorband_kwargs)
        self.distribution.add(data, column, mask, weight_column, label=label)

        n = len(self.box_axes) + 1
        gs = matplotlib.gridspec.GridSpec(4 * n, 1)
        gridspecs = [gs[:3 * n, :]] + [gs[3 * n + i, :] for i in range(n)]
        box_axis = self.add_subplot(gridspecs)

        if self.range_in_std is not None:
            mean, std = histogram.weighted_mean_and_std(data[column], None if weight_column is None else data[weight_column])
            # Everything outside mean +- range_in_std * std is considered not inside the mask
            mask = mask & (data[column] > (mean - self.range_in_std * std)) & (data[column] < (mean + self.range_in_std * std))
        box = Box(self.figure, box_axis)
        box.add(data, column, mask, weight_column)
        if len(box.plots) > 0:
            box.plots[0]['boxes'][0].set_facecolor(self.distribution.plots[-1][0][0].get_color())
        box.finish()

        self.box_axes.append(box_axis)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.distribution.finish()
        matplotlib.artist.setp(self.axis.get_xticklabels(), visible=False)
        self.axis.get_xaxis().set_label_text('')
        for box_axis in self.box_axes[:-1]:
            matplotlib.artist.setp(box_axis.get_xticklabels(), visible=False)
            box_axis.set_title("")
            box_axis.get_xaxis().set_label_text('')
        self.box_axes[-1].set_title("")
        self.axis.set_title("Distribution Plot")
        self.axis.legend([x[0] for x in self.distribution.plots], self.distribution.labels,
                         loc='best', fancybox=True, framealpha=0.5)
        return self


class Correlation(Plotter):
    """
    Plots change of a distribution of a quantity depending on the cut on a classifier
    """
    #: figure which is used to draw
    figure = None
    #: Main axis which is used to draw
    axis = None
    #: Axis which shows shape of signal
    axis_d1 = None
    #: Axis which shows shape of background
    axis_d2 = None

    def __init__(self, figure=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        """
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(32, 18))
            self.figure.set_tight_layout(True)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(3, 2)
        self.axis = self.figure.add_subplot(gs[0, :])
        self.axis_d1 = self.figure.add_subplot(gs[1, :], sharex=self.axis)
        self.axis_d2 = self.figure.add_subplot(gs[2, :], sharex=self.axis)

        super(Correlation, self).__init__(self.figure, self.axis)

    def add(self, data, column, cut_column, quantiles, signal_mask=None, bckgrd_mask=None, weight_column=None):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param cut_column which is used to calculate cut on the other quantity defined by column
        @param quantiles list of quantiles between 0 and 100, defining the different cuts
        @param weight_column column in data containing the weights for each event
        """
        if len(data[cut_column]) == 0:
            B2WARNING("Ignore empty Correlation.")
            return self

        axes = [self.axis, self.axis_d1, self.axis_d2]

        for i, (l, m) in enumerate([('.', signal_mask | bckgrd_mask), ('S', signal_mask), ('B', bckgrd_mask)]):

            if weight_column is not None:
                weights = numpy.array(data[weight_column][m])
            else:
                weights = numpy.ones(len(data[column][m]))

            # The cast to float32 is a workaround for the following numpy issue:
            # https://github.com/numpy/numpy/issues/8123
            xrange = np.percentile(data[column][m], [5, 95]).astype(np.float32)

            colormap = plt.get_cmap('coolwarm')
            tmp, x = np.histogram(data[column][m], bins=100,
                                  range=xrange, normed=True, weights=weights)
            bin_center = ((x + np.roll(x, 1)) / 2)[1:]
            axes[i].plot(bin_center, tmp, color='black', lw=1)

            for quantil in np.arange(5, 100, 5):
                cut = np.percentile(data[cut_column][m], quantil)
                sel = data[cut_column][m] >= cut
                y, x = np.histogram(data[column][m][sel], bins=100,
                                    range=xrange, normed=True, weights=weights[sel])
                bin_center = ((x + np.roll(x, 1)) / 2)[1:]
                axes[i].fill_between(bin_center, tmp, y, color=colormap(quantil / 100.0))
                tmp = y

            axes[i].set_ylim(bottom=0)

            flatness_score = basf2_mva_util.calculate_flatness(data[column][m], data[cut_column][m], weights)
            axes[i].set_title(r'Distribution for different quantiles: $\mathrm{{Flatness}}_{} = {:.3f}$'.format(l, flatness_score))
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        return self


class TSNE(Plotter):
    """
    Plots multivariate distribution using TSNE algorithm
    """

    def add(self, data, columns, *masks):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param columns which are used to calculate the correlations
        @param masks different classes to show in TSNE
        """
        try:
            import sklearn
            import sklearn.manifold
            model = sklearn.manifold.TSNE(n_components=2, random_state=0)
            data = numpy.array([data[column] for column in columns]).T
            model.fit(data)
            for mask in masks:
                data = numpy.array([data[column][mask] for column in columns]).T
                data = model.transform(data)
                self.axis.scatter(data[:, 0], data[:, 1], rasterized=True)
        except ImportError:
            print("Cannot create TSNE plot. Install sklearn if you want it")
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        return self


class Importance(Plotter):
    """
    Plots importance matrix
    """

    def add(self, data, columns, variables):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param columns which are used to calculate the correlations
        """
        self.figure.set_tight_layout(True)

        def norm(x):
            width = (numpy.max(x) - numpy.min(x))
            if width <= 0:
                return numpy.zeros(x.shape)
            return (x - numpy.min(x)) / width * 100

        importance_matrix = numpy.vstack([norm(data[column]) for column in columns]).T
        importance_heatmap = self.axis.pcolor(importance_matrix, cmap=plt.cm.RdBu, vmin=0.0, vmax=100,
                                              rasterized=True)

        # put the major ticks at the middle of each cell
        self.axis.set_yticks(numpy.arange(importance_matrix.shape[0]) + 0.5, minor=False)
        self.axis.set_xticks(numpy.arange(importance_matrix.shape[1]) + 0.5, minor=False)

        self.axis.set_xticklabels(columns, minor=False, rotation=90)
        self.axis.set_yticklabels(variables, minor=False)

        self.axis.xaxis.tick_top()

        for y in range(importance_matrix.shape[0]):
            for x in range(importance_matrix.shape[1]):
                self.axis.text(x + 0.5, y + 0.5, '%.0f' % importance_matrix[y, x],
                               size=14,
                               horizontalalignment='center',
                               verticalalignment='center')

        cb = self.figure.colorbar(importance_heatmap, ticks=[0.0, 100], orientation='vertical')
        cb.ax.set_yticklabels(['low', 'high'])

        self.axis.set_aspect('equal')

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        return self


class CorrelationMatrix(Plotter):
    """
    Plots correlation matrix
    """
    #: figure which is used to draw
    figure = None
    #: Main axis which shows the correlation of the signal samples
    signal_axis = None
    #: Axis which shows the correlation of the background samples
    bckgrd_axis = None

    def __init__(self, figure=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        """
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(32, 18))
            self.figure.set_tight_layout(True)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(8, 2)
        self.signal_axis = self.figure.add_subplot(gs[:6, 0])
        self.bckgrd_axis = self.figure.add_subplot(gs[:6, 1], sharey=self.signal_axis)
        #: Colorbar axis contains the colorbar
        self.colorbar_axis = self.figure.add_subplot(gs[7, :])
        #: Usual axis object which every Plotter object needs, here it is just a dummy
        self.axis = self.signal_axis

        super(CorrelationMatrix, self).__init__(self.figure, self.axis)

    def add(self, data, columns, signal_mask, bckgrd_mask):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param columns which are used to calculate the correlations
        """
        signal_corr = numpy.corrcoef(numpy.vstack([data[column][signal_mask] for column in columns])) * 100
        bckgrd_corr = numpy.corrcoef(numpy.vstack([data[column][bckgrd_mask] for column in columns])) * 100

        signal_heatmap = self.signal_axis.pcolor(signal_corr, cmap=plt.cm.RdBu, vmin=-100.0, vmax=100.0)

        bckgrd_heatmap = self.bckgrd_axis.pcolor(bckgrd_corr, cmap=plt.cm.RdBu, vmin=-100.0, vmax=100.0)

        self.signal_axis.invert_yaxis()
        self.signal_axis.xaxis.tick_top()
        self.bckgrd_axis.invert_yaxis()
        self.bckgrd_axis.xaxis.tick_top()

        # put the major ticks at the middle of each cell
        self.signal_axis.set_xticks(numpy.arange(signal_corr.shape[0]) + 0.5, minor=False)
        self.signal_axis.set_yticks(numpy.arange(signal_corr.shape[1]) + 0.5, minor=False)

        self.signal_axis.set_xticklabels(columns, minor=False, rotation=90)
        self.signal_axis.set_yticklabels(columns, minor=False)

        # put the major ticks at the middle of each cell
        self.bckgrd_axis.set_xticks(numpy.arange(bckgrd_corr.shape[0]) + 0.5, minor=False)
        self.bckgrd_axis.set_yticks(numpy.arange(bckgrd_corr.shape[1]) + 0.5, minor=False)

        self.bckgrd_axis.set_xticklabels(columns, minor=False, rotation=90)
        self.bckgrd_axis.set_yticklabels(columns, minor=False)

        for y in range(signal_corr.shape[0]):
            for x in range(signal_corr.shape[1]):
                self.signal_axis.text(x + 0.5, y + 0.5, '%.0f' % signal_corr[y, x],
                                      size=14,
                                      horizontalalignment='center',
                                      verticalalignment='center')

        for y in range(bckgrd_corr.shape[0]):
            for x in range(bckgrd_corr.shape[1]):
                self.bckgrd_axis.text(x + 0.5, y + 0.5, '%.0f' % bckgrd_corr[y, x],
                                      size=14,
                                      horizontalalignment='center',
                                      verticalalignment='center')

        cb = self.figure.colorbar(signal_heatmap, cax=self.colorbar_axis, ticks=[-100, 0, 100], orientation='horizontal')
        cb.solids.set_rasterized(True)
        cb.ax.set_xticklabels(['negative', 'uncorrelated', 'positive'])

        self.signal_axis.text(0.5, -1.0, "Signal", horizontalalignment='center')
        self.bckgrd_axis.text(0.5, -1.0, "Background", horizontalalignment='center')

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        matplotlib.artist.setp(self.bckgrd_axis.get_yticklabels(), visible=False)
        return self


if __name__ == '__main__':

    def get_data(N, columns):
        """
        Creates fake data for example plots
        """
        N /= 2
        n = len(columns) - 1
        xs = numpy.random.normal(0, size=(N, n))
        xb = numpy.random.normal(1, size=(N, n))
        ys = numpy.zeros(N)
        yb = numpy.ones(N)
        data = pandas.DataFrame(numpy.c_[numpy.r_[xs, xb], numpy.r_[ys, yb]], columns=columns)
        return data.reindex(numpy.random.permutation(data.index))

    import seaborn
    # Set nice searborn settings
    seaborn.set(font_scale=3)
    seaborn.set_style('whitegrid')

    # Standard plots
    N = 100000
    data = get_data(N, columns=['FastBDT', 'NeuroBayes', 'isSignal'])
    data['type'] = ''
    data.type.iloc[:N / 2] = 'Train'
    data.type.iloc[N / 2:] = 'Test'

    p = Box()
    p.add(data, 'FastBDT')
    p.finish()
    p.save('box_plot.png')

    p = VerboseDistribution()
    p.add(data, 'FastBDT')
    p.add(data, 'NeuroBayes')
    p.finish()
    p.save('verbose_distribution_plot.png')

    p = PurityOverEfficiency()
    p.add(data, 'FastBDT', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'NeuroBayes', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('roc_purity_plot.png')

    p = RejectionOverEfficiency()
    p.add(data, 'FastBDT', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'NeuroBayes', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('roc_rejection_plot.png')

    p = Diagonal()
    p.add(data, 'FastBDT', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'NeuroBayes', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('diagonal_plot.png')

    p = Distribution()
    p.add(data, 'FastBDT')
    p.add(data, 'NeuroBayes')
    p.finish()
    p.save('distribution_plot.png')

    p = Difference()
    p.add(data, 'FastBDT', data['type'] == 'Train', data['type'] == 'Test')
    p.add(data, 'NeuroBayes', data['type'] == 'Train', data['type'] == 'Test')
    p.finish()
    p.save('difference_plot.png')

    p = Overtraining()
    p.add(data, 'FastBDT', data['type'] == 'Train', data['type'] == 'Test', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('overtraining_plot.png')

    p = Correlation()
    p.add(data, 'FastBDT', 'NeuroBayes', [0, 20, 40, 60, 80, 100], data['isSignal'] == 0)
    p.finish()
    p.save('correlation_plot.png')

    p = CorrelationMatrix()
    data['FastBDT2'] = data['FastBDT']**2
    data['NeuroBayes2'] = data['NeuroBayes']**2
    data['FastBDT3'] = data['FastBDT']**3
    data['NeuroBayes3'] = data['NeuroBayes']**3
    p.add(data, ['FastBDT', 'NeuroBayes', 'FastBDT2', 'NeuroBayes2', 'FastBDT3', 'NeuroBayes3'])
    p.finish()
    p.save('correlation_matrix.png')
