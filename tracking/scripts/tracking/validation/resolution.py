#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from .plot import ValidationPlot, compose_axis_label, get_unit

# get error function as a np.ufunc vectorised for numpy array
from tracking.root_utils import root_save_name

import collections

import numpy as np

from .tolerate_missing_key_formatter import TolerateMissingKeyFormatter

formatter = TolerateMissingKeyFormatter()


class ResolutionAnalysis(object):
    """Perform resolution analysis"""

    #: default Z-score (for outlier detection)
    default_outlier_z_score = 5.0
    #: default minimum number of entries
    default_min_required_entries = 50
    #: default plot name
    default_plot_name = "{plot_name_prefix}_{subplot_name}{plot_name_postfix}"
    #: default plot title
    default_plot_title = "{subplot_title} of {quantity_name}{plot_title_postfix}"
    #: default list of plots to create
    default_which_plots = [
        "resolution",
    ]

    #: by default, create expert plots
    default_is_expert = True

    def __init__(
        self,
        quantity_name,
        bin_spacing,  # can be [0,0.5,1.0] will have 2 bins
        bin_name,
        bin_unit=None,
        unit=None,
        outlier_z_score=None,
        contact='',
        plot_name=None,
        plot_title=None,
        min_required_entries=None,  # minimum number of entries in a bin for the resolution fit
        plot_name_prefix='',  # depricated use plot_name instead
        plot_name_postfix='',  # depricated use plot_name instead
        plot_title_postfix='',  # depricated use plot_title instead
        referenceFileName=None,  # if set binnings of histograms will be read from corresponding histogram in this file
    ):
        """Performs a comparison of an estimated quantity to their truths by generating standardized validation plots."""

        #: cached name of the quantity in the truth-classification analysis
        self.quantity_name = quantity_name
        #: cached measurement unit for this truth-classification analysis
        self.unit = unit or get_unit(quantity_name)
        #: cached value of the histogram bin spacing
        self.bin_spacing = bin_spacing
        #: cached value of the bin name
        self.bin_name = bin_name
        #: cached value of the bin measurement unit
        self.bin_unit = bin_unit

        if outlier_z_score is None:
            #: cached value of the Z-score (for outlier detection)
            self.outlier_z_score = self.default_outlier_z_score
        else:
            self.outlier_z_score = outlier_z_score

        #: cached value of the minimum number of entries
        self.min_required_entries = min_required_entries
        if self.min_required_entries is None:
            self.min_required_entries = self.default_min_required_entries

        #: cached value of the base name of the plot
        self.plot_name = plot_name
        #: cached value of the plot title
        self.plot_title = plot_title

        #: cached value of the prefix prepended to the plot name
        self.plot_name_prefix = plot_name_prefix or root_save_name(quantity_name)
        #: cached value of the suffix appended to the plot name
        self.plot_name_postfix = plot_name_postfix
        #: cached value of the suffix appended to the plot title
        self.plot_title_postfix = plot_title_postfix

        #: cached value of the contact person
        self._contact = contact
        #: cached value of the dictionary of plots to be created
        self.plots = collections.OrderedDict()

        #: cached value of the reference filename
        self.referenceFileName = referenceFileName

    def analyse(
        self,
        bin_values,
        truths,
        estimates,
        which_plots=None,
        is_expert=None
    ):
        """Compares the concrete estimate to the truth and generates plots of the resolution

        Parameters
        ----------
        bin_values : array_like(float
            The parametr used for binning
        truths : array_like(float)
            Sample of the true values
        estimates : array_like(float)
            Corresponding estimations
        """

        if is_expert is None:
            is_expert = self.default_is_expert

        if which_plots is None:
            which_plots = self.default_which_plots

        quantity_name = self.quantity_name

        # axis_label = compose_axis_label(quantity_name, self.unit)

        plot_name_prefix = self.plot_name_prefix
        outlier_z_score = self.outlier_z_score

        plot_name = self.plot_name
        if plot_name is None:
            plot_name = self.default_plot_name

        plot_name = formatter.format(plot_name,
                                     quantity_name=quantity_name,
                                     plot_name_prefix=plot_name_prefix,
                                     plot_name_postfix=self.plot_name_postfix)

        plot_title = self.plot_title
        if plot_title is None:
            plot_title = self.default_plot_title

        plot_title = formatter.format(plot_title,
                                      quantity_name=quantity_name,
                                      plot_title_postfix=self.plot_title_postfix)

        # compute residuals
        residuals = estimates - truths

        # Resolution #
        #############
        if "resolution" in which_plots:

            # creating plots for all configured bins
            res_histogram = []
            resolution_values = []

            for i in range(len(self.bin_spacing) - 1):
                lower_bin = self.bin_spacing[i]
                upper_bin = self.bin_spacing[i + 1]
                assert (lower_bin < upper_bin)
                bin_center = lower_bin + (upper_bin - lower_bin) / 2.0
                assert (len(bin_values) == len(residuals))

                # compile a list of values which are in this bin
                sel_residuals = collections.deque()

                for i in range(len(bin_values)):
                    if bin_values[i] >= lower_bin and bin_values[i] < upper_bin:
                        sel_residuals.append(residuals[i])

                residuals_hist_name = formatter.format(plot_name, subplot_name="residuals") + \
                    "{}_to_{}".format(lower_bin, upper_bin)
                vplot = ValidationPlot(residuals_hist_name, self.referenceFileName)
                vplot.hist(sel_residuals,
                           outlier_z_score=outlier_z_score,
                           is_expert=is_expert)
                vplot.xlabel = compose_axis_label("#Delta " + quantity_name + " (estimate - truth)", self.unit)
                vplot.title = formatter.format(plot_title, subplot_title='Residual distribution')

                # this values will stay None if no fit could be performed
                gaus_sigma = None
                gaus_sigma_err = None

                # check if the minimum number of entries are in the histogram
                if vplot.histograms[0].GetEntries() >= self.min_required_entries:
                    fit_res = vplot.fit_gaus(z_score=1)

                    # extract fit result from ROOT's TFitResut
                    params = fit_res.GetParams()
                    errs = fit_res.Errors()

                    # gaus_mean = params[1]
                    gaus_sigma = params[2]
                    gaus_sigma_err = errs[2]

                    res_histogram += [(lower_bin, upper_bin, bin_center, vplot)]
                    self.plots['residuals' + residuals_hist_name] = vplot

                # store the fit results
                resolution_values += [(lower_bin, upper_bin, bin_center, gaus_sigma, gaus_sigma_err)]

            resolution_graph_name = formatter.format(plot_name, subplot_name="resolution")
            resolution_graph = ValidationPlot(resolution_graph_name, self.referenceFileName)

            # compile all requried data going into the final TGraphErrors
            xs = []
            xs_err = []
            ys = []
            ys_err = []

            for v in resolution_values:
                # could be None if no fit was possible for this bin
                if v[3]:
                    xs += [v[2]]
                    xs_err = [0.0]
                    ys += [v[3]]
                    ys_err = [v[4]]

            # convert to numpy array before giving to the plotting code
            resolution_graph.grapherrors((np.array(xs), np.array(xs_err)), (np.array(ys), np.array(ys_err)),
                                         is_expert=is_expert)
            resolution_graph.xlabel = compose_axis_label(self.bin_name, self.bin_unit)
            resolution_graph.ylabel = compose_axis_label(self.quantity_name, self.unit)
            resolution_graph.title = formatter.format(plot_title, subplot_title='Resolution')

            self.plots[resolution_graph_name] = resolution_graph

        #: Forward the contact to all plots by reassigning the contact.
        self.contact = self.contact

    @property
    def contact(self):
        """Get the contact person's name"""
        return self._contact

    @contact.setter
    def contact(self, contact):
        """Set the contact person's name"""
        self._contact = contact
        for validation_plot in list(self.plots.values()):
            validation_plot.contact = contact

    def write(self, tDirectory=None):
        """Write all validation plot to the given Root directory"""
        for validation_plot in list(self.plots.values()):
            validation_plot.write(tDirectory)
