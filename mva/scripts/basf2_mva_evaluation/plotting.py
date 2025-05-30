#!/usr/bin/env python3


##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import copy
import math

import pandas
import numpy
import itertools
import matplotlib.pyplot as plt
import matplotlib.artist
import matplotlib.figure
import matplotlib.gridspec
import matplotlib.colors
import matplotlib.patches
import matplotlib.ticker
import matplotlib.patheffects as PathEffects


from basf2_mva_evaluation import histogram

import basf2 as b2

import basf2_mva_util
import matplotlib

# Do not use standard backend TkAgg, because it is NOT thread-safe
# You will get an RuntimeError: main thread is not in main loop otherwise!
matplotlib.use("svg")

# Use the Belle II style while producing the plots
plt.style.use("belle2")


class Plotter:
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

    def __init__(self, figure=None, axis=None, dpi=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param dpi dpi for the matplotlib figure, if None default is used
        """
        b2.B2INFO("Create new figure for class " + str(type(self)))
        #: set default dpi
        self.dpi = dpi
        if figure is None:
            #: create figure
            self.figure = matplotlib.figure.Figure(figsize=(12, 8), dpi=dpi)
        else:
            self.figure = figure

        if axis is None:
            #: divide figure into subplots
            self.axis = self.figure.add_subplot(1, 1, 1)
        else:
            self.axis = axis

        #: create empty list for plots
        self.plots = []
        #: create empty list for labels
        self.labels = []
        #: set x limits
        self.xmin, self.xmax = float(0), float(1)
        #: set y limits
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

        #: Property cycler used to give plots unique colors
        self.prop_cycler = itertools.cycle(plt.rcParams["axes.prop_cycle"])

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
        b2.B2INFO("Save figure for class " + str(type(self)))
        from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
        canvas = FigureCanvas(self.figure)
        canvas.print_figure(filename, dpi=self.dpi, bbox_inches='tight')
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
            color = next(self.prop_cycler)
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

            # fully mask nan values.
            # Needed until https://github.com/matplotlib/matplotlib/pull/23333 makes it into the externals.
            # TODO: remove in release 8.
            if not isinstance(xerr, (numpy.ndarray, list)):
                xerr = xerr*numpy.ones(len(x))
            if not isinstance(yerr, (numpy.ndarray, list)):
                yerr = yerr*numpy.ones(len(y))
            mask = numpy.logical_and.reduce([numpy.isfinite(v) for v in [x, y, xerr, yerr]])

            e = axis.errorbar(
                x[mask], y[mask], xerr=numpy.where(
                    xerr[mask] < 0, 0.0, xerr[mask]), yerr=numpy.where(
                    yerr[mask] < 0, 0.0, yerr[mask]), rasterized=True, **errorbar_kwargs)
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
            # to fill the last bin of a histogram
            x = numpy.append(x, x[-1]+2*xerr[-1])
            y = numpy.append(y, y[-1])
            xerr = numpy.append(xerr, xerr[-1])

            axis.fill_between(x-xerr, y, 0, rasterized=True, **fill_kwargs)

        return (tuple(patches), p, e, f)

    def add(self, *args, **kwargs):
        """
        Add a new plot to this plotter
        """
        return NotImplemented

    def setAxisLimits(self, factor=0.0):
        """
        Sets the limits of the axis with an optional expansion factor.

        Parameters:
            factor (float): Fraction by which to expand the axis limits beyond the data range.
        """
        dx = self.xmax - self.xmin
        dy = self.ymax - self.ymin
        self.axis.set_xlim((self.xmin - factor*dx, self.xmax + factor*dx))
        self.axis.set_ylim((self.ymin - factor*dy, self.ymax + factor*dy))

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
        @param normed boolean if True, the efficiency and purity are normalized to 1
        """

        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)

        if normed:
            efficiency, efficiency_error = hists.get_efficiency(['Signal'])
            purity, purity_error = hists.get_purity(['Signal'], ['Background'])
        else:
            efficiency, efficiency_error = hists.get_true_positives(['Signal'])
            purity, purity_error = hists.get_false_positives(['Background'])

        if isinstance(efficiency, int) and not isinstance(purity, int):
            efficiency = numpy.array([efficiency] * len(purity))
        elif isinstance(purity, int) and not isinstance(efficiency, int):
            purity = numpy.array([purity] * len(efficiency))
        elif isinstance(purity, int) and isinstance(efficiency, int):
            efficiency = numpy.array([efficiency])
            purity = numpy.array([purity])
        cuts = hists.bin_centers

        self.xmin, self.xmax = numpy.nanmin(numpy.append(cuts, self.xmin)), numpy.nanmax(numpy.append(cuts, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(
            numpy.concatenate(
                (efficiency, purity, [
                    self.ymin]))), numpy.nanmax(
                numpy.concatenate(
                    (efficiency, purity, [
                        self.ymax])))

        self.set_errorbar_options({'fmt': '-o'})
        self.plots.append(self._plot_datapoints(self.axis, cuts, efficiency, xerr=0, yerr=efficiency_error))

        if normed:
            self.labels.append("Efficiency")
        else:
            self.labels.append("True positive")

        self.set_errorbar_options({'fmt': '-o'})
        self.plots.append(self._plot_datapoints(self.axis, cuts, purity, xerr=0, yerr=purity_error))

        if normed:
            self.labels.append("Purity")
        else:
            self.labels.append("False positive")

        self.axis.set_title("Classification Plot")

        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.setAxisLimits(factor=0.01)
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

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, label=None):
        """
        Add a new curve to the plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate signal to noise ratio for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        @param label label for the plot legend
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        signal2noise, signal2noise_error = hists.get_signal_to_noise(['Signal'], ['Background'])
        cuts = hists.bin_centers

        valid = numpy.isfinite(signal2noise)
        signal2noise = signal2noise[valid]
        signal2noise_error = signal2noise_error[valid]
        cuts = cuts[valid]

        # Determine "best" cut by maximizing Signal to Noise
        if len(signal2noise) == 0 or numpy.all(numpy.isnan(signal2noise)):
            best_idx = None
        else:
            best_idx = numpy.nanargmax(signal2noise)
            best_cut = cuts[best_idx]
            best_signal2noise = signal2noise[best_idx]

        self.xmin, self.xmax = numpy.nanmin(numpy.append(cuts, self.xmin)), numpy.nanmax(numpy.append(cuts, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(
            numpy.append(
                signal2noise, self.ymin)), numpy.nanmax(
            numpy.append(
                signal2noise, self.ymax))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, cuts, signal2noise, xerr=0, yerr=signal2noise_error)
        self.plots.append(p)

        # Plot best cut point
        if best_idx is not None:
            self.axis.plot(best_cut, best_signal2noise, 'x', color=p[1].get_color(), markersize=8, label='Best cut')
            self.axis.axvline(best_cut, color=p[1].get_color(), linestyle='dashed', linewidth=1)
            self.axis.axhline(best_signal2noise, color=p[1].get_color(), linestyle='dashed', linewidth=1)

            # Add label with best cut info
            cut_label = f"{label[:10] if label else column[:10]} (Best cut: {best_cut:.3f}, S/N: {best_signal2noise:.2f})"
            self.labels.append(cut_label)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.setAxisLimits(factor=0.05)
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
        @param label label for the plot legend
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        purity, purity_error = hists.get_purity(['Signal'], ['Background'])
        if isinstance(efficiency, int) and not isinstance(purity, int):
            efficiency = numpy.array([efficiency] * len(purity))
        elif isinstance(purity, int) and not isinstance(efficiency, int):
            purity = numpy.array([purity] * len(efficiency))
        elif isinstance(purity, int) and isinstance(efficiency, int):
            efficiency = numpy.array([efficiency])
            purity = numpy.array([purity])
        cuts = hists.bin_centers

        valid = numpy.isfinite(purity) & numpy.isfinite(efficiency)
        efficiency = efficiency[valid]
        purity = purity[valid]
        cuts = cuts[valid]
        if not isinstance(efficiency_error, int):
            efficiency_error = efficiency_error[valid]
        if not isinstance(purity_error, int):
            purity_error = purity_error[valid]

        # Determine "best" cut (closest to point (1,1))
        distance = numpy.sqrt(numpy.square(1 - purity) + numpy.square(1 - efficiency))
        if len(distance) == 0 or numpy.all(numpy.isnan(distance)):
            best_idx = None
        else:
            best_idx = numpy.nanargmin(distance)
            best_cut = cuts[best_idx]
            best_efficiency = efficiency[best_idx]
            best_purity = purity[best_idx]

        self.xmin, self.xmax = numpy.nanmin(numpy.append(efficiency, self.xmin)), numpy.nanmax(numpy.append(efficiency, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(purity, self.ymin)), numpy.nanmax(numpy.append(purity, self.ymax))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, efficiency, purity, xerr=efficiency_error, yerr=purity_error)
        self.plots.append(p)

        if best_idx is not None:
            # Plot best cut point
            self.axis.plot(best_efficiency, best_purity, 'x', color=p[1].get_color(), markersize=8, label='Best cut')
            self.axis.axhline(best_purity, color=p[1].get_color(), linestyle='dashed', linewidth=1)
            self.axis.axvline(best_efficiency, color=p[1].get_color(), linestyle='dashed', linewidth=1)

            # Add label with best cut info
            cut_label = f"{label[:10] if label else column[:10]} (Best cut: {best_cut:.3f})"
            self.labels.append(cut_label)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.setAxisLimits(factor=0.01)
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
        @param label label for the plot legend
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        rejection, rejection_error = hists.get_efficiency(['Background'])
        rejection = 1 - rejection
        if isinstance(efficiency, int) and not isinstance(rejection, int):
            efficiency = numpy.array([efficiency] * len(rejection))
        elif isinstance(rejection, int) and not isinstance(efficiency, int):
            rejection = numpy.array([rejection] * len(efficiency))
        elif isinstance(rejection, int) and isinstance(efficiency, int):
            efficiency = numpy.array([efficiency])
            rejection = numpy.array([rejection])
        cuts = hists.bin_centers

        valid = numpy.isfinite(rejection) & numpy.isfinite(efficiency)
        efficiency = efficiency[valid]
        rejection = rejection[valid]
        cuts = cuts[valid]
        if not isinstance(efficiency_error, int):
            efficiency_error = efficiency_error[valid]
        if not isinstance(rejection_error, int):
            rejection_error = rejection_error[valid]

        # Determine "best" cut by maximizing Rejection / Efficiency
        distance = numpy.sqrt(numpy.square(1 - rejection) + numpy.square(1 - efficiency))
        if len(distance) == 0 or numpy.all(numpy.isnan(distance)):
            best_idx = None
        else:
            best_idx = numpy.nanargmin(distance)
            best_cut = cuts[best_idx]
            best_rejection = rejection[best_idx]
            best_efficiency = efficiency[best_idx]

        self.xmin, self.xmax = numpy.nanmin(numpy.append(efficiency, self.xmin)), numpy.nanmax(numpy.append(efficiency, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(rejection, self.ymin)), numpy.nanmax(numpy.append(rejection, self.ymax))

        auc = numpy.abs(numpy.trapz(rejection, efficiency))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, efficiency, rejection, xerr=efficiency_error, yerr=rejection_error)
        self.plots.append(p)

        if best_idx is not None:
            # Plot best cut point
            self.axis.plot(best_efficiency, best_rejection, 'x', color=p[1].get_color(), markersize=8, label='Best cut')
            self.axis.axhline(best_rejection, color=p[1].get_color(), linestyle='dashed', linewidth=1)
            self.axis.axvline(best_efficiency, color=p[1].get_color(), linestyle='dashed', linewidth=1)

            # Add label with best cut info
            cut_label = f"{label[:10] if label else column[:10]} (AUC: {auc:.2f}, Best cut: {best_cut:.3f})"
            self.labels.append(cut_label)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.setAxisLimits(factor=0.01)
        self.axis.set_title("ROC Rejection Plot")
        self.axis.get_yaxis().set_label_text('Background Rejection')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)

        self.axis.get_xaxis().set_label_text('Signal Efficiency')
        return self


class TrueVsFalsePositiveRate(Plotter):
    """
    Plots the true ROC curve: True Positive Rate (TPR) vs False Positive Rate (FPR),
    and marks the cut that gives the point closest to the ideal (0,1).
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
        @param label label for the plot legend
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask},
                                     weight_column=weight_column)

        tpr, tpr_error = hists.get_efficiency(['Signal'])       # True Positive Rate (TPR)
        fpr, fpr_error = hists.get_efficiency(['Background'])   # False Positive Rate (FPR)
        if isinstance(tpr, int) and not isinstance(fpr, int):
            tpr = numpy.array([tpr] * len(fpr))
        elif isinstance(fpr, int) and not isinstance(tpr, int):
            fpr = numpy.array([fpr] * len(tpr))
        elif isinstance(fpr, int) and isinstance(tpr, int):
            tpr = numpy.array([tpr])
            fpr = numpy.array([fpr])
        cuts = hists.bin_centers                                 # Cut values for each bin

        valid = numpy.isfinite(tpr) & numpy.isfinite(fpr)
        tpr = tpr[valid]
        fpr = fpr[valid]
        cuts = cuts[valid]
        if not isinstance(tpr_error, int):
            tpr_error = tpr_error[valid]
        if not isinstance(fpr_error, int):
            fpr_error = fpr_error[valid]

        # Determine "best" cut (closest to top-left corner (0,1))
        distance = numpy.sqrt(numpy.square(fpr) + numpy.square(1 - tpr))
        if len(distance) == 0 or numpy.all(numpy.isnan(distance)):
            best_idx = None
        else:
            best_idx = numpy.nanargmin(distance)
            best_cut = cuts[best_idx]
            best_tpr = tpr[best_idx]
            best_fpr = fpr[best_idx]

        # Update plot range
        self.xmin, self.xmax = numpy.nanmin(numpy.append(fpr, self.xmin)), numpy.nanmax(numpy.append(fpr, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(tpr, self.ymin)), numpy.nanmax(numpy.append(tpr, self.ymax))

        auc = numpy.abs(numpy.trapz(tpr, fpr))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, fpr, tpr, xerr=fpr_error, yerr=tpr_error)
        self.plots.append(p)

        if best_idx is not None:
            # Plot best cut point
            self.axis.plot(best_fpr, best_tpr, 'x', color=p[1].get_color(), markersize=8)
            self.axis.axhline(best_tpr, color=p[1].get_color(), linestyle='dashed', linewidth=1)
            self.axis.axvline(best_fpr, color=p[1].get_color(), linestyle='dashed', linewidth=1)

            # Add label with best cut info
            cut_label = f"{label[:10] if label else column[:10]} (AUC: {auc:.2f}, Cut: {best_cut:.3f})"
            self.labels.append(cut_label)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.setAxisLimits(factor=0.01)
        self.axis.set_title("True ROC Curve")
        self.axis.get_xaxis().set_label_text('False Positive Rate (Background Efficiency)')
        self.axis.get_yaxis().set_label_text('True Positive Rate (Signal Efficiency)')
        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)
        return self


class PrecisionRecallCurve(Plotter):
    """
    Plots the Precision vs Recall curve and marks the cut that gives the point closest to the ideal (1,1).
    """
    #: @var xmax
    #: Maximum x value
    #: @var ymax
    #: Maximum y value

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, label=None):
        """
        Add a new curve to the Precision-Recall plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate efficiency and purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        @param label label for the plot legend
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask},
                                     weight_column=weight_column)

        recall, recall_error = hists.get_efficiency(['Signal'])  # Recall = TPR
        precision, precision_error = hists.get_purity(['Signal'], ['Background'])
        if isinstance(recall, int) and not isinstance(precision, int):
            recall = numpy.array([recall] * len(precision))
        elif isinstance(precision, int) and not isinstance(recall, int):
            precision = numpy.array([precision] * len(recall))
        elif isinstance(precision, int) and isinstance(recall, int):
            recall = numpy.array([recall])
            precision = numpy.array([precision])
        cuts = hists.bin_centers

        valid = numpy.isfinite(precision) & numpy.isfinite(recall)
        precision = precision[valid]
        recall = recall[valid]
        cuts = cuts[valid]
        if not isinstance(recall_error, int):
            recall_error = recall_error[valid]
        if not isinstance(precision_error, int):
            precision_error = precision_error[valid]

        # Determine "best" cut (closest to point (1,1))
        distance = numpy.sqrt(numpy.square(1 - precision) + numpy.square(1 - recall))
        if len(distance) == 0 or numpy.all(numpy.isnan(distance)):
            best_idx = None
        else:
            best_idx = numpy.nanargmin(distance)
            best_cut = cuts[best_idx]
            best_recall = recall[best_idx]
            best_precision = precision[best_idx]

        # Update plot range
        self.xmin, self.xmax = numpy.nanmin(numpy.append(recall, self.xmin)), numpy.nanmax(numpy.append(recall, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(precision, self.ymin)), numpy.nanmax(numpy.append(precision, self.ymax))

        auc = numpy.abs(numpy.trapz(precision, recall))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, recall, precision, xerr=recall_error, yerr=precision_error)
        self.plots.append(p)

        if best_idx is not None:
            # Plot best cut point
            self.axis.plot(best_recall, best_precision, 'x', color=p[1].get_color(), markersize=8, label='Best cut')
            self.axis.axhline(best_precision, color=p[1].get_color(), linestyle='dashed', linewidth=1)
            self.axis.axvline(best_recall, color=p[1].get_color(), linestyle='dashed', linewidth=1)

            # Add label with best cut info
            cut_label = f"{label[:10] if label else column[:10]} (AUC: {auc:.2f}, Cut: {best_cut:.3f})"
            self.labels.append(cut_label)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.setAxisLimits(factor=0.01)
        self.axis.set_title("Precision-Recall Curve")
        self.axis.get_xaxis().set_label_text('Recall (Signal Efficiency)')
        self.axis.get_yaxis().set_label_text('Precision (Purity)')
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

    def __init__(self, cls, number_of_plots, figure=None, dpi=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param cls class of the plot
        @param number_of_plots number of plots which should be displayed
        @param figure default draw figure which is used
        @param dpi dpi for the matplotlib figure, if None default is used
        """
        if number_of_plots == 1:
            gsTuple = (1, 1)
        elif number_of_plots == 2:
            gsTuple = (1, 2)
        elif number_of_plots == 3:
            gsTuple = (1, 3)
        elif number_of_plots == 4:
            gsTuple = (2, 2)
        elif number_of_plots == 6:
            gsTuple = (2, 3)
        else:
            gsTuple = (int(numpy.ceil(number_of_plots / 3)), 3)

        #: set default dpi
        self.dpi = dpi
        if figure is None:
            #: create figure
            self.figure = matplotlib.figure.Figure(figsize=(12*gsTuple[1], 8*gsTuple[0]), dpi=dpi)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(gsTuple[0], gsTuple[1])
        #: get grid list of tuples of grid positions
        grid_list = list(itertools.product(range(gs.nrows), range(gs.ncols)))
        #: the subplots which are displayed in the grid
        self.sub_plots = [cls(self.figure, self.figure.add_subplot(gs[grid_list[i][0], grid_list[i][1]]))
                          for i in range(number_of_plots)]
        #: the axis of the first subplot
        self.axis = self.sub_plots[0].axis
        super().__init__(self.figure, self.axis)

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

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None, label=None):
        """
        Add a new curve to the Diagonal plot
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate purity for different cuts
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        @param label label for the plot legend
        """
        hists = histogram.Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        purity, purity_error = hists.get_purity_per_bin(['Signal'], ['Background'])

        self.xmin, self.xmax = numpy.nanmin(
            numpy.append(
                hists.bin_centers, self.xmin)), numpy.nanmax(
            numpy.append(
                hists.bin_centers, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(purity, self.ymin)), numpy.nanmax(numpy.append(purity, self.ymax))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, hists.bin_centers, purity, xerr=hists.bin_widths / 2.0, yerr=purity_error)
        self.plots.append(p)
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
        self.axis.plot((0.0, 1.0), (0.0, 1.0), color='black')
        self.setAxisLimits(factor=0.01)
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
        super().__init__(figure, axis)
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
        @param label label for the plot legend
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
            hist = hist / normalization if normalization > 0 else hist
            hist_error = hist_error / normalization if normalization > 0 else hist_error

        if self.normed_to_bin_width:
            hist = hist / hists.bin_widths if normalization > 0 else hist
            hist_error = hist_error / hists.bin_widths if normalization > 0 else hist_error

        self.xmin, self.xmax = numpy.nanmin(
            numpy.append(
                hists.bin_centers, self.xmin)), numpy.nanmax(
            numpy.append(
                hists.bin_centers, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(hist, self.ymin)), numpy.nanmax(numpy.append(hist + hist_error, self.ymax))

        self.set_errorbar_options({'fmt': '-o'})
        p = self._plot_datapoints(self.axis, hists.bin_centers, hist, xerr=hists.bin_widths / 2, yerr=hist_error)
        self.plots.append(p)
        self.x_axis_label = column

        appendix = ''
        if self.ymax <= self.ymin or self.xmax <= self.xmin:
            appendix = ' No data to plot!'

        if label is None:
            self.labels.append(column + appendix)
        else:
            self.labels.append(label + appendix)
        return self

    def finish(self):
        """
        Sets limits, title, axis-labels and legend of the plot
        """
        self.axis.set_title("Distribution Plot")
        self.axis.get_xaxis().set_label_text(self.x_axis_label)

        self.axis.legend([x[0] for x in self.plots], self.labels, loc='best', fancybox=True, framealpha=0.5)

        if self.ymax <= self.ymin or self.xmax <= self.xmin:
            self.axis.set_xlim((0., 1.))
            self.axis.set_ylim((0., 1.))
            self.axis.text(0.36, 0.5, 'No data to plot', fontsize=60, color='black')
            return self

        self.scale_limits()
        self.setAxisLimits(factor=0.01)

        if self.normed_to_all_entries and self.normed_to_bin_width:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / (# Entries * Bin Width)')
        elif self.normed_to_all_entries:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / # Entries')
        elif self.normed_to_bin_width:
            self.axis.get_yaxis().set_label_text('# Entries per Bin / Bin Width')
        else:
            self.axis.get_yaxis().set_label_text('# Entries per Bin')

        return self


class Box(Plotter):
    """
    Create a boxplot
    """
    #: @var x_axis_label
    #: Label on x axis

    def __init__(self, figure=None, axis=None, x_axis_label=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        """
        super().__init__(figure=figure, axis=axis)

        #: Label on x axis
        self.x_axis_label = x_axis_label

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
            # weight = data[weight_column][mask]
            b2.B2WARNING("Weights are currently not used in boxplot, due to limitations in matplotlib")

        if len(x) == 0:
            b2.B2WARNING("Ignore empty boxplot.")
            return self

        # we don't plot outliers as they cause the file size to explode if large datasets are used
        p = self.axis.boxplot(x, sym='k.', whis=1.5, vert=False, patch_artist=True, showmeans=True, widths=1,
                              boxprops=dict(facecolor='blue', alpha=0.5), showfliers=False,
                              # medianprobs=dict(color='blue'),
                              # meanprobs=dict(color='red'),
                              )
        self.plots.append(p)
        self.labels.append(column)
        if not self.x_axis_label:
            self.x_axis_label = column
        r"""
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
        super().__init__(figure, axis)
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

        self.xmin, self.xmax = numpy.nanmin(
            numpy.append(
                hists.bin_centers, self.xmin)), numpy.nanmax(
            numpy.append(
                hists.bin_centers, self.xmax))
        self.ymin, self.ymax = numpy.nanmin(numpy.append(difference - difference_error, self.ymin)
                                            ), numpy.nanmax(numpy.append(difference + difference_error, self.ymax))

        self.set_errorbar_options({'fmt': '-o'})
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
        self.setAxisLimits(factor=0.01)
        self.axis.set_title("Difference Plot")
        self.axis.get_yaxis().set_major_locator(matplotlib.ticker.MaxNLocator(5))
        self.axis.get_xaxis().set_label_text(self.x_axis_label)
        self.axis.get_yaxis().set_label_text('Diff.')
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

    def __init__(self, figure=None, dpi=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param dpi dpi for the matplotlib figure, if None default is used
        """
        #: set default dpi
        self.dpi = dpi
        if figure is None:
            #: create figure
            self.figure = matplotlib.figure.Figure(figsize=(12, 8), dpi=self.dpi)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(5, 1)
        #: define first subplot
        self.axis = self.figure.add_subplot(gs[:3, :])
        #: define second subplot
        self.axis_d1 = self.figure.add_subplot(gs[3, :], sharex=self.axis)
        #: define third subplot
        self.axis_d2 = self.figure.add_subplot(gs[4, :], sharex=self.axis)

        super().__init__(self.figure, self.axis)

    def add(self, data, column, train_mask, test_mask, signal_mask, bckgrd_mask, weight_column=None):
        """
        Add a new overtraining plot, I recommend to draw only one overtraining plot at the time,
        otherwise there are too many curves in the plot to recognize anything in the plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param train_mask boolean numpy.array defining which events are training events
        @param test_mask boolean numpy.array defining which events are test events
        @param signal_mask boolean numpy.array defining which events are signal events
        @param bckgrd_mask boolean numpy.array defining which events are background events
        @param weight_column column in data containing the weights for each event
        """
        distribution = Distribution(self.figure, self.axis, normed_to_all_entries=True)
        self.axis.set_yscale('log')

        distribution.set_plot_options(self.plot_kwargs)
        distribution.set_errorbar_options(self.errorbar_kwargs)
        distribution.set_errorband_options(self.errorband_kwargs)
        distribution.add(data, column, test_mask & signal_mask, weight_column)
        distribution.add(data, column, test_mask & bckgrd_mask, weight_column)

        distribution.set_plot_options(
            {'color': distribution.plots[0][0][0].get_color(), 'linestyle': '-', 'lw': 4, 'drawstyle': 'steps-mid'})
        distribution.set_fill_options({'color': distribution.plots[0][0][0].get_color(), 'alpha': 0.5, 'step': 'post'})
        distribution.set_errorbar_options(None)
        distribution.set_errorband_options(None)
        distribution.add(data, column, train_mask & signal_mask, weight_column)
        distribution.set_plot_options(
            {'color': distribution.plots[1][0][0].get_color(), 'linestyle': '-', 'lw': 4, 'drawstyle': 'steps-mid'})
        distribution.set_fill_options({'color': distribution.plots[1][0][0].get_color(), 'alpha': 0.5, 'step': 'post'})
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
                b2.B2WARNING("Cannot calculate kolmogorov smirnov test for signal due to missing data")
            else:
                ks = scipy.stats.ks_2samp(data[column][train_mask & signal_mask], data[column][test_mask & signal_mask])
                props = dict(boxstyle='round', edgecolor='gray', facecolor='white', linewidth=0.1, alpha=0.5)
                self.axis_d1.text(0.1, 0.9, r'signal (train - test) difference $p={:.2f}$'.format(ks[1]),  bbox=props,
                                  verticalalignment='top', horizontalalignment='left', transform=self.axis_d1.transAxes)
            if len(data[column][train_mask & bckgrd_mask]) == 0 or len(data[column][test_mask & bckgrd_mask]) == 0:
                b2.B2WARNING("Cannot calculate kolmogorov smirnov test for background due to missing data")
            else:
                ks = scipy.stats.ks_2samp(data[column][train_mask & bckgrd_mask], data[column][test_mask & bckgrd_mask])
                props = dict(boxstyle='round', edgecolor='gray', facecolor='white', linewidth=0.1, alpha=0.5)
                self.axis_d2.text(0.1, 0.9, r'background (train - test) difference $p={:.2f}$'.format(ks[1]),
                                  bbox=props,
                                  verticalalignment='top', horizontalalignment='left', transform=self.axis_d2.transAxes)
        except ImportError:
            b2.B2WARNING("Cannot calculate kolmogorov smirnov test please install scipy!")

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

    def __init__(self, figure=None, axis=None, normed=False, range_in_std=None, x_axis_label=None):
        """
        Creates a new figure and axis if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param axis default draw axis which is used
        @param normed true if the histograms should be normed before drawing
        @param range_in_std show only the data in a windows around +- range_in_std * standard_deviation around the mean
        """
        super().__init__(figure, axis)
        #: Normalize histograms before drawing them
        self.normed = normed
        #: Show only a certain range in terms of standard deviations of the data
        self.range_in_std = range_in_std
        #: create empty list for box axes
        self.box_axes = []
        #: The distribution plot
        self.distribution = Distribution(self.figure, self.axis, normed_to_all_entries=self.normed, range_in_std=self.range_in_std)
        #: x axis label
        self.x_axis_label = x_axis_label

    def add(self, data, column, mask=None, weight_column=None, label=None):
        """
        Add a new distribution plot, with additional information like a boxplot compared to
        the ordinary Distribution plot.
        @param data pandas.DataFrame containing all data
        @param column which is used to calculate distribution histogram
        @param mask boolean numpy.array defining which events are used for the distribution histogram
        @param weight_column column in data containing the weights for each event
        @param label label for the plot legend
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
        box = Box(self.figure, box_axis, x_axis_label=self.x_axis_label)
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

    def __init__(self, figure=None, dpi=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param dpi dpi for the matplotlib figure, if None default is used
        """
        #: set default dpi
        self.dpi = dpi
        if figure is None:
            #: create figure
            self.figure = matplotlib.figure.Figure(figsize=(12, 8), dpi=self.dpi)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(3, 2)
        #: define first subplot
        self.axis = self.figure.add_subplot(gs[0, :])
        #: define second subplot
        self.axis_d1 = self.figure.add_subplot(gs[1, :], sharex=self.axis)
        #: define third subplot
        self.axis_d2 = self.figure.add_subplot(gs[2, :], sharex=self.axis)

        super().__init__(self.figure, self.axis)

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
            b2.B2WARNING("Ignore empty Correlation.")
            return self

        axes = [self.axis, self.axis_d1, self.axis_d2]

        for i, (l, m) in enumerate([('.', signal_mask | bckgrd_mask), ('S', signal_mask), ('B', bckgrd_mask)]):
            if weight_column is not None:
                weights = numpy.array(data[weight_column][m])
            else:
                weights = numpy.ones(len(data[column][m]))

            xrange = numpy.percentile(data[column][m], [5, 95])
            isfinite = numpy.isfinite(data[column][m])
            if not numpy.all(isfinite):
                xrange = numpy.percentile(data[column][m][isfinite], [5, 95])
            elif numpy.all(numpy.isnan(data[column][m])):
                b2.B2WARNING("All data is NaN, cannot calculate range and ignore Correlation.")
                return self

            colormap = plt.get_cmap('coolwarm')
            tmp, x = numpy.histogram(data[column][m], bins=100,
                                     range=xrange, density=True, weights=weights)
            bin_center = ((x + numpy.roll(x, 1)) / 2)[1:]
            axes[i].plot(bin_center, tmp, color='black', lw=1)

            for quantil in numpy.arange(5, 100, 5):
                cut = numpy.percentile(data[cut_column][m], quantil)
                sel = data[cut_column][m] >= cut
                y, x = numpy.histogram(data[column][m][sel], bins=100,
                                       range=xrange, density=True, weights=weights[sel])
                bin_center = ((x + numpy.roll(x, 1)) / 2)[1:]
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

        def norm(x):
            width = (numpy.max(x) - numpy.min(x))
            if width <= 0:
                return numpy.zeros(x.shape)
            return (x - numpy.min(x)) / width * 100

        importance_matrix = numpy.vstack([norm(data[column]) for column in columns]).T
        im = self.axis.imshow(
            importance_matrix[::-1],  # <- reverse rows
            cmap=plt.cm.RdBu,
            vmin=0.0,
            vmax=100.0,
            aspect='equal',
            interpolation='nearest',
            origin='upper'
        )

        num_y, num_x = importance_matrix.shape

        # Adjust font size based on matrix size
        base_font_size = 14
        font_size = max(6, base_font_size * min(1.0, 25 / max(num_x, num_y)))

        # Tick positions and labels
        self.axis.set_xticks(numpy.arange(num_x))
        self.axis.set_yticks(numpy.arange(num_y))

        self.axis.set_xticklabels(columns, rotation=90, fontsize=font_size)
        self.axis.set_yticklabels(reversed(variables), fontsize=font_size)

        self.axis.tick_params(top=True, bottom=False, labeltop=True, labelbottom=False)

        # Add text annotations
        for y in range(num_y):
            for x in range(num_x):
                value = importance_matrix[-1-y, x]  # Reverse y-axis for correct annotation
                txt = self.axis.text(
                    x, y, f'{value:.0f}',
                    ha='center', va='center',
                    fontsize=font_size,
                    color='white'
                )
                txt.set_path_effects([PathEffects.withStroke(linewidth=3, foreground='black')])

        # Colorbar
        cb = self.figure.colorbar(im, ax=self.axis, ticks=[0.0, 100.0], orientation='vertical')
        cb.ax.set_yticklabels(['low', 'high'])
        cb.solids.set_rasterized(True)

        # Layout tightening
        self.axis.set_xlim(-0.5, num_x - 0.5)
        self.axis.set_ylim(num_y - 0.5, -0.5)  # origin='upper' flips y

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

    def __init__(self, figure=None, dpi=None):
        """
        Creates a new figure if None is given, sets the default plot parameters
        @param figure default draw figure which is used
        @param dpi dpi for the matplotlib figure, if None default is used
        """
        #: set default dpi
        self.dpi = dpi
        if figure is None:
            #: create figure
            self.figure = matplotlib.figure.Figure(figsize=(12, 8), dpi=self.dpi)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(8, 2)
        #: add signal subplot
        self.signal_axis = self.figure.add_subplot(gs[:6, 0])
        #: add background subplot
        self.bckgrd_axis = self.figure.add_subplot(gs[:6, 1], sharey=self.signal_axis)
        #: Colorbar axis contains the colorbar
        self.colorbar_axis = self.figure.add_subplot(gs[7, :])
        #: Usual axis object which every Plotter object needs, here it is just a dummy
        self.axis = self.signal_axis

        super().__init__(self.figure, self.axis)

    def add(self, data, columns, signal_mask, bckgrd_mask):
        """
        Add a new correlation plot.
        @param data pandas.DataFrame containing all data
        @param columns which are used to calculate the correlations
        """
        num_vars = len(columns)
        font_size = max(4, min(14, 200 // num_vars))  # Scale font size

        signal_corr = numpy.corrcoef(numpy.vstack([data[column][signal_mask] for column in columns])) * 100
        bckgrd_corr = numpy.corrcoef(numpy.vstack([data[column][bckgrd_mask] for column in columns])) * 100

        signal_heatmap = self.signal_axis.imshow(
            signal_corr[::-1, ::-1],  # <- reverse rows and columns
            cmap=plt.cm.RdBu,
            vmin=-100.0,
            vmax=100.0,
            origin='upper',
            aspect='auto',
            interpolation='nearest')
        self.bckgrd_axis.imshow(
            bckgrd_corr[::-1, ::-1],  # <- reverse rows and columns
            cmap=plt.cm.RdBu,
            vmin=-100.0,
            vmax=100.0,
            origin='upper',
            aspect='auto',
            interpolation='nearest')

        # Tick positions
        tick_positions = numpy.arange(num_vars)

        # Signal ticks
        self.signal_axis.set_xlabel('Signal')
        self.signal_axis.set_xticks(tick_positions)
        self.signal_axis.set_yticks(tick_positions)
        self.signal_axis.set_xticklabels(reversed(columns), rotation=90, fontsize=font_size)
        self.signal_axis.set_yticklabels(reversed(columns), fontsize=font_size)
        self.signal_axis.xaxis.tick_top()
        self.signal_axis.invert_yaxis()

        # Background ticks
        self.bckgrd_axis.set_xlabel('Background')
        self.bckgrd_axis.set_xticks(tick_positions)
        self.bckgrd_axis.set_yticks(tick_positions)
        self.bckgrd_axis.set_xticklabels(reversed(columns), rotation=90, fontsize=font_size)
        self.bckgrd_axis.set_yticklabels(reversed(columns), fontsize=font_size)
        self.bckgrd_axis.xaxis.tick_top()
        self.bckgrd_axis.invert_yaxis()

        # Add annotation text
        for y in range(num_vars):
            for x in range(num_vars):
                txt = self.signal_axis.text(x, y, f'{signal_corr[-1-y, -1-x]:.0f}',
                                            ha='center', va='center',
                                            fontsize=font_size,
                                            color='white')
                txt.set_path_effects([PathEffects.withStroke(linewidth=3, foreground='k')])
                txt = self.bckgrd_axis.text(x, y, f'{bckgrd_corr[-1-y, -1-x]:.0f}',
                                            ha='center', va='center',
                                            fontsize=font_size,
                                            color='white')
                txt.set_path_effects([PathEffects.withStroke(linewidth=3, foreground='k')])

        # Colorbar
        cb = self.figure.colorbar(signal_heatmap, cax=self.colorbar_axis,
                                  ticks=[-100, 0, 100], orientation='horizontal')
        cb.solids.set_rasterized(True)
        cb.ax.set_xticklabels(['negative', 'uncorrelated', 'positive'])

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
