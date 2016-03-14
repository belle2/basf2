#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2015

import copy

import numpy
import pandas
import scipy
import scipy.stats
import matplotlib
import matplotlib.artist
import matplotlib.figure
import matplotlib.gridspec
import matplotlib.colors
import matplotlib.patches
import matplotlib.ticker
import seaborn

import b2stat


class Plotter(object):
    """
    Base class for all Plotters.
    """

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

        #: Default keyword arguments for plot function
        self.plot_kwargs = None
        #: Default keyword arguments for errorbar function
        self.errorbar_kwargs = None
        #: Default keyword arguments for fill_between function
        self.errorband_kwargs = None

        self.set_plot_options()
        self.set_errorbar_options()
        self.set_errorband_options()

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

        if plot_kwargs is None or 'color' not in plot_kwargs:
            color = next(axis._get_lines.color_cycle)
            plot_kwargs['color'] = color
        else:
            color = plot_kwargs['color']
        color = matplotlib.colors.ColorConverter().to_rgb(color)
        patch = matplotlib.patches.Patch(color=color, alpha=0.5)
        patch.get_color = patch.get_facecolor

        if plot_kwargs is not None:
            p, = axis.plot(x, y, **plot_kwargs)

        if errorbar_kwargs is not None and (xerr is not None or yerr is not None):
            if 'color' not in errorbar_kwargs:
                errorbar_kwargs['color'] = color
            if 'ecolor' not in errorbar_kwargs:
                errorbar_kwargs['ecolor'] = map(lambda x: 0.5*x, color)
            e = axis.errorbar(x, y, xerr=xerr, yerr=yerr, **errorbar_kwargs)

        if errorband_kwargs is not None and yerr is not None:
            if 'color' not in errorband_kwargs:
                errorband_kwargs['color'] = color
            x1 = x
            y1 = y-yerr
            y2 = y+yerr
            if xerr is not None:
                boundaries = numpy.r_[numpy.c_[x-xerr, y1, y2], numpy.c_[x+xerr, y1, y2]]
                boundaries = boundaries[boundaries[:, 0].argsort()]
                x1 = boundaries[:, 0]
                y1 = boundaries[:, 1]
                y2 = boundaries[:, 2]
            f = axis.fill_between(x1, y1, y2, interpolate=True, **errorband_kwargs)

        return (patch, p, e, f)

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


class PurityOverEfficiency(Plotter):
    """
    Plots the purity over the efficiency also known as ROC curve
    """
    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        """
        Add a new curve to the ROC plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate efficiency and purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        hists = b2stat.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        purity, purity_error = hists.get_purity(['Signal'], ['Background'])

        self.xmin, self.xmax = numpy.nanmin([efficiency.min(), self.xmin]), numpy.nanmax([efficiency.max(), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(purity), self.ymin]), numpy.nanmax([numpy.nanmax(purity), self.ymax])

        p = self._plot_datapoints(self.axis, efficiency, purity, xerr=efficiency_error, yerr=purity_error)
        self.plots.append(p)
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
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class RejectionOverEfficiency(Plotter):
    """
    Plots the rejection over the efficiency also known as ROC curve
    """
    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        """
        Add a new curve to the ROC plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate efficiency and purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        hists = b2stat.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        rejection, rejection_error = hists.get_efficiency(['Background'])
        rejection = 1 - rejection

        self.xmin, self.xmax = numpy.nanmin([efficiency.min(), self.xmin]), numpy.nanmax([efficiency.max(), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([rejection.min(), self.ymin]), numpy.nanmax([rejection.max(), self.ymax])

        p = self._plot_datapoints(self.axis, efficiency, rejection, xerr=efficiency_error, yerr=rejection_error)
        self.plots.append(p)
        self.labels.append(column)
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
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Diagonal(Plotter):
    """
    Plots the purity in each bin over the classifier output.
    """
    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        """
        Add a new curve to the Diagonal plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        hists = b2stat.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        purity, purity_error = hists.get_purity_per_bin(['Signal'], ['Background'])

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(purity), self.ymin]), numpy.nanmax([numpy.nanmax(purity), self.ymax])

        p = self._plot_datapoints(self.axis, hists.bin_centers, purity, xerr=hists.bin_widths/2.0, yerr=purity_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.plot((self.xmin, self.xmax), (0, 1), color='black')
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Diagonal Plot")
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.get_yaxis().set_label_text('Purity Per Bin')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Distribution(Plotter):
    """
    Plots distribution of a quantity
    """

    def __init__(self, figure=None, axis=None, normed=False):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param normed true if histograms should be normed before drawing
        """
        super(Distribution, self).__init__(figure, axis)
        #: Normalize histograms before drawing them
        self.normed = normed
        if self.normed:
            self.ymin = float(0)
            self.ymax = float('-inf')
        self.xmin = float('inf')
        self.xmax = float('-inf')

    def add(self, data, column, mask=None, weight_column=None):
        """
        Add a new distribution to the plots
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param mask boolean numpy.array defining which events are used for the histogram
        @param weight_column column in data containing the weights for each event
        """
        if mask is None:
            mask = numpy.ones(len(data)).astype('bool')
        hists = b2stat.Histograms(data, column, {'Total': mask}, weight_column=weight_column)
        hist, hist_error = hists.get_hist('Total')

        if self.normed:
            normalization = float(numpy.sum(hist))
            hist = hist / normalization
            hist_error = hist_error / normalization

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin, self.ymax = numpy.nanmin([hist.min(), self.ymin]), numpy.nanmax([(hist + hist_error).max(), self.ymax])

        p = self._plot_datapoints(self.axis, hists.bin_centers, hist, xerr=hists.bin_widths/2, yerr=hist_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Distribution Plot")
        self.axis.get_xaxis().set_label_text('Classifier Output')
        if self.normed:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / # Entries')
        else:
            self.axis.get_yaxis().set_label_text('# Entries per Bin')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Box(Plotter):
    """
    Create a boxplot
    """
    def add(self, data, column, mask=None):
        """
        Add a new boxplot to the plots
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate boxplot quantities
        @param mask boolean numpy.array defining which events are used for the histogram
        @param weight_column column in data containing the weights for each event
        """
        if mask is None:
            mask = numpy.ones(len(data)).astype('bool')
        x = data.loc[mask, column]
        p = self.axis.boxplot(x, sym='k.', whis=1.5, vert=False, patch_artist=True, showmeans=True, widths=1,
                              boxprops=dict(facecolor='blue', alpha=0.5),
                              # medianprobs=dict(color='blue'),
                              # meanprobs=dict(color='red'),
                              )
        """
        self.axis.text(0.1, 0.9, (r'$     \mu = {:.2f}$' + '\n' + r'$median = {:.2f}$').format(x.mean(), x.median()),
                       fontsize=28, verticalalignment='top', horizontalalignment='left', transform=self.axis.transAxes)
        self.axis.text(0.4, 0.9, (r'$  \sigma = {:.2f}$' + '\n' + r'$IQD = {:.2f}$').format(x.std(),
                                                                                            x.quantile(0.75) - x.quantile(0.25)),
                       fontsize=28, verticalalignment='top', horizontalalignment='left', transform=self.axis.transAxes)
        self.axis.text(0.7, 0.9, (r'$min = {:.2f}$' + '\n' + r'$max = {:.2f}$').format(x.min(), x.max()),
                       fontsize=28, verticalalignment='top', horizontalalignment='left', transform=self.axis.transAxes)
        """
        self.plots.append(p)
        self.labels.append(column)

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        matplotlib.artist.setp(self.axis.get_yaxis(), visible=False)
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.set_title("Box Plot")
        return self


class Difference(Plotter):
    """
    Plots the difference between two histograms
    """
    def add(self, data, column, minuend_mask, subtrahend_mask, weight_column=None):
        """
        Add a new difference plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param minuend_mask boolean numpy.array defining which events are for the minuend histogram
        @param subtrahend_mask boolean numpy.array defining which events are for the subtrahend histogram
        @param weight_column column in data containing the weights for each event
        """
        hists = b2stat.Histograms(data, column,
                                  {'Minuend': minuend_mask, 'Subtrahend': subtrahend_mask}, weight_column=weight_column)
        minuend, minuend_error = hists.get_hist('Minuend')
        subtrahend, subtrahend_error = hists.get_hist('Subtrahend')
        difference, difference_error = minuend - subtrahend, b2stat.poisson_error(minuend + subtrahend)

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin = min((difference - difference_error).min(), self.ymin)
        self.ymax = max((difference + difference_error).max(), self.ymax)

        p = self._plot_datapoints(self.axis, hists.bin_centers, difference, xerr=hists.bin_widths/2, yerr=difference_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self, line_color='black'):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.plot((self.xmin, self.xmax), (0, 0), color=line_color, linewidth=4)
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Difference Plot")
        self.axis.get_yaxis().set_major_locator(matplotlib.ticker.MaxNLocator(5))
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.get_yaxis().set_label_text('Difference')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc='best', fancybox=True, framealpha=0.5)
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
        distribution = Distribution(self.figure, self.axis, normed=True)
        distribution.set_plot_options(self.plot_kwargs)
        distribution.set_errorbar_options(self.errorbar_kwargs)
        distribution.set_errorband_options(self.errorband_kwargs)
        distribution.add(data, column, train_mask & signal_mask, weight_column)
        distribution.add(data, column, train_mask & bckgrd_mask, weight_column)
        distribution.add(data, column, test_mask & signal_mask, weight_column)
        distribution.add(data, column, test_mask & bckgrd_mask, weight_column)
        distribution.labels = ['Train-Signal', 'Train-Background', 'Test-Signal', 'Test-Background']
        distribution.finish()

        self.plot_kwargs['color'] = distribution.plots[2][0].get_color()
        difference_signal = Difference(self.figure, self.axis_d1)
        difference_signal.set_plot_options(self.plot_kwargs)
        difference_signal.set_errorbar_options(self.errorbar_kwargs)
        difference_signal.set_errorband_options(self.errorband_kwargs)
        difference_signal.add(data, column, train_mask & signal_mask, test_mask & signal_mask, weight_column)
        self.axis_d1.set_xlim((difference_signal.xmin, difference_signal.xmax))
        self.axis_d1.set_ylim((difference_signal.ymin, difference_signal.ymax))
        difference_signal.plots = difference_signal.labels = []
        difference_signal.finish(line_color=distribution.plots[0][0].get_color())

        self.plot_kwargs['color'] = distribution.plots[3][0].get_color()
        difference_bckgrd = Difference(self.figure, self.axis_d2)
        difference_bckgrd.set_plot_options(self.plot_kwargs)
        difference_bckgrd.set_errorbar_options(self.errorbar_kwargs)
        difference_bckgrd.set_errorband_options(self.errorband_kwargs)
        difference_bckgrd.add(data, column, train_mask & bckgrd_mask, test_mask & bckgrd_mask, weight_column)
        self.axis_d2.set_xlim((difference_bckgrd.xmin, difference_bckgrd.xmax))
        self.axis_d2.set_ylim((difference_bckgrd.ymin, difference_bckgrd.ymax))
        difference_bckgrd.plots = difference_bckgrd.labels = []
        difference_bckgrd.finish(line_color=distribution.plots[1][0].get_color())

        # Kolmogorov smirnov test
        ks = scipy.stats.ks_2samp(data.loc[train_mask & signal_mask, column], data.loc[test_mask & signal_mask, column])
        props = dict(boxstyle='round', edgecolor='gray', facecolor='white', linewidth=0.1, alpha=0.5)
        self.axis_d1.text(0.1, 0.9, r'signal (train - test) difference $p={:.2f}$'.format(ks[1]), fontsize=28, bbox=props,
                          verticalalignment='top', horizontalalignment='left', transform=self.axis_d1.transAxes)
        ks = scipy.stats.ks_2samp(data.loc[train_mask & bckgrd_mask, column], data.loc[test_mask & bckgrd_mask, column])
        self.axis_d2.text(0.1, 0.9, r'background (train - test) difference $p={:.2f}$'.format(ks[1]), fontsize=28, bbox=props,
                          verticalalignment='top', horizontalalignment='left', transform=self.axis_d2.transAxes)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        matplotlib.artist.setp(self.axis.get_xaxis(), visible=False)
        matplotlib.artist.setp(self.axis_d1.get_xaxis(), visible=False)
        self.axis.set_title("Overtraining Plot")
        self.axis_d1.set_title("")
        self.axis_d2.set_title("")
        self.axis_d2.get_xaxis().set_label_text('Classifier Output')
        return self


class VerboseDistribution(Plotter):
    """
    Plots distribution of a quantity including boxplots
    """

    #: Axes for the boxplots
    box_axes = None

    def __init__(self, figure=None, axis=None, normed=False):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param normed true if the histograms should be normed before drawing
        """
        super(VerboseDistribution, self).__init__(figure, axis)
        #: Normalize histograms before drawing them
        self.normed = normed
        self.box_axes = []

    def add(self, data, column, mask=None, weight_column=None):
        """
        Add a new distribution plot, with additional information like a boxplot compared to
        the ordinary Distribution plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param mask boolean numpy.array defining which events are used for the distribution histogram
        @param weight_column column in data containing the weights for each event
        """
        distribution = Distribution(self.figure, self.axis, self.normed)
        distribution.set_plot_options(self.plot_kwargs)
        distribution.set_errorbar_options(self.errorbar_kwargs)
        distribution.set_errorband_options(self.errorband_kwargs)
        distribution.add(data, column, mask, weight_column)
        distribution.finish()
        self.plots += distribution.plots
        self.labels += distribution.labels

        n = len(self.box_axes)+1
        gs = matplotlib.gridspec.GridSpec(4*n, 1)
        gridspecs = [gs[:3*n, :]] + [gs[3*n+i, :] for i in range(n)]
        box_axis = self.add_subplot(gridspecs)

        box = Box(self.figure, box_axis)
        box.add(data, column, mask)
        box.plots[0]['boxes'][0].set_facecolor(distribution.plots[0][0].get_color())
        box.finish()

        self.box_axes.append(box_axis)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        for box_axis in self.box_axes:
            matplotlib.artist.setp(box_axis.get_xaxis(), visible=False)
            box_axis.set_title('')
        self.axis.set_title("Distribution Plot")
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class Correlation(Plotter):
    """
    Plots distribution of a quantity multiple times with different cuts on the quantiles of another quantity
    """
    def add(self, data, column, cut_column, quantiles, mask=None, weight_column=None):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param cut_column which is used to calculate cut on the other quantity defined by column
        @param quantiles list of quantiles between 0 and 100, defining the different cuts
        @param weight_column column in data containing the weights for each event
        """
        percentiles = numpy.percentile(data[cut_column], q=quantiles)
        distribution = Distribution(self.figure, self.axis, normed=False)
        distribution.set_plot_options(self.plot_kwargs)
        distribution.set_errorbar_options(self.errorbar_kwargs)
        distribution.set_errorband_options(self.errorband_kwargs)
        for p in percentiles:
            distribution.add(data, column, data[cut_column] > p, weight_column)
        if mask is not None:
            self.axis.set_color_cycle(None)
            distribution.set_plot_options({'linestyle': '--'})
            distribution.set_errorbar_options(None)
            distribution.set_errorband_options(None)
            for p in percentiles:
                distribution.add(data, column, (data[cut_column] > p) & mask, weight_column)
        distribution.plots = distribution.plots[:len(quantiles)]
        distribution.labels = [str(q) + '% Quantiles' for q in quantiles]
        distribution.finish()
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
    def add(self, data, columns):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param columns which are used to calculate the correlations
        """
        import seaborn
        corr = data[columns].corr()
        # Generate a custom diverging colormap
        cmap = seaborn.diverging_palette(220, 10, as_cmap=True)
        # Draw the heatmap with the mask and correct aspect ratio
        seaborn.heatmap(corr, cmap=cmap,
                        square=True, xticklabels=range(1, len(columns)+1), yticklabels=range(1, len(columns)+1), annot=True,
                        linewidths=.5, cbar_kws={"shrink": .5}, ax=self.axis)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
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
    data = get_data(100000, columns=['FastBDT', 'NeuroBayes', 'isSignal'])
    data['type'] = ''
    data.type.iloc[:50000] = 'Train'
    data.type.iloc[50000:] = 'Test'

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

    """
    p = CorrelationMatrix()
    data['FastBDT2'] = data['FastBDT']**2
    data['NeuroBayes2'] = data['NeuroBayes']**2
    data['FastBDT3'] = data['FastBDT']**3
    data['NeuroBayes3'] = data['NeuroBayes']**3
    p.add(data, ['FastBDT', 'NeuroBayes', 'FastBDT2', 'NeuroBayes2', 'FastBDT3', 'NeuroBayes3'])
    p.finish()
    p.save('correlation_matrix.png')
    """
