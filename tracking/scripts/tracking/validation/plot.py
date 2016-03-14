#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import itertools
import collections
import array
import numpy as np
from functools import reduce

import ROOT

from tracking.root_utils import root_cd, root_save_name

import tracking.validation.statistics as statistics

import logging


def get_logger():
    """Get for the logging.Logger instance of this module

    Returns
    -------
    logging.Logger
        Logger instance of this module
    """
    return logging.getLogger(__name__)

#: A map from quanity name symbols to their usual units in Belle II standard units.
units_by_quantity_name = {
    'x': 'cm',
    'y': 'cm',
    'z': 'cm',
    'E': 'GeV',
    'px': 'GeV',
    'p_{x}': 'GeV',
    'py': 'GeV',
    'p_{y}': 'GeV',
    'pz': 'GeV',
    'p_{z}': 'GeV',
    'pt': 'GeV',
    'p_{t}': 'GeV',
    'd0': 'cm',
    'd_{0}': 'cm',
    'phi0': None,
    '#phi_{0}': None,
    'omega': '1/cm',
    '#omega': '1/cm',
    'z0': 'cm',
    'z_{0}': 'cm',
    'tan_lambda': None,
    'tan #lambda': None}


def get_unit(quantity_name):
    """Infers the unit of a quantity from its name.

    Assumes the standard Belle II unit system.

    Currently looks up the quantity string from units_by_quantity_name.

    Parameters
    ----------
    quantity_name : str
        Name of a quantity (E.g. pt, x, ...)

    Returns
    -------
    str
    """

    unit = units_by_quantity_name.get(quantity_name, None)
    return unit


def compose_axis_label(quantity_name, unit=None):
    """Formats a quantity name and a unit to a label for a plot axes.

    If the unit is not given to is tried to infer it
    from the quantity name by the get_unit function.

    Parameters
    ----------
    quantity_name : str
        Name of the quantity to be displayed at the axes
    unit : str, optional
        The unit of the quantity. Defaults to get_unit(quantity_name)

    Returns
    -------
    str
    """

    if unit is None:
        unit = get_unit(quantity_name)

    if unit is None:
        axis_label = quantity_name
    else:
        axis_label = '%s (%s)' % (quantity_name, unit)

    return axis_label


#: A class for storing a named additional statistic to a histogram
StatsEntry = ROOT.TParameter(float)


class ValidationPlot(object):

    """Class for generating a validation plot for the Belle II validation page.

    Typically it generates plots from values stored in numpy arrays and feeds them into
    plot ROOT classes for storing them.

    It implements an automatic binning procedure based on the rice rule and
    robust z score outlier detection.

    It also keeps track of additional statistics typically neglected by ROOT such as a count
    for the non finit values such as NaN, +Inf, -Inf.

    The special attributes for the Belle II validation page like
    * title
    * contract
    * description
    * check
    are exposed as properties of this class.
    """

    #: A an index that reference to a dot spacing such that the line is almost invisible for scatter
    very_sparse_dots_line_style_index = 28
    # This line style is defined in the set_tstyle method below.

    def __init__(self, name):
        """Constructor of the ValidationPlot

        Parameters
        ----------
        name : str
            A unique name to be used as the name of the ROOT object to be generated
        """

        #: A unique name to be used as the name of the ROOT object to be generated
        self.name = root_save_name(name)

        #: Description of the plot purpose for display on the validation page
        self._description = ''

        #: Detailed check instructions for display on the validation page
        self._check = ''

        #: Contact email address for display on the validation page
        self._contact = ''

        #: X axes label of the validation plot
        self._xlabel = ''

        #: Y axes label of the validation plot
        self._ylabel = ''

        #: Title of the validation plot
        self._title = ''

        #: The main plot object, may contain one or more (in case of stacked pltos) histograms
        self.plot = None

        #: A list of the histograms that make up the plot
        self.histograms = []

        # per default all plots are expert and must be set to non-expert explicitly
        self._is_expert = True

        # custom levels for pvalue warnings and errors
        self.pvalue_warn = None
        self.pvalue_error = None

    def hist(self,
             xs,
             weights=None,
             stackby=None,
             bins=None,
             lower_bound=None,
             upper_bound=None,
             outlier_z_score=None,
             include_exceptionals=True,
             allow_discrete=False,
             cumulation_direction=None,
             is_expert=True):
        """Fill the plot with a one dimensional histogram."""

        th1_factory = ROOT.TH1D
        self._is_expert = is_expert

        self.create_1d(th1_factory,
                       xs,
                       weights=weights,
                       stackby=stackby,
                       bins=bins,
                       lower_bound=lower_bound,
                       upper_bound=upper_bound,
                       outlier_z_score=outlier_z_score,
                       include_exceptionals=include_exceptionals,
                       allow_discrete=allow_discrete,
                       cumulation_direction=cumulation_direction)

        if not self.ylabel:
            self.ylabel = 'count'

        return self

    def profile(self,
                xs,
                ys,
                weights=None,
                stackby=None,
                bins=None,
                lower_bound=None,
                upper_bound=None,
                outlier_z_score=None,
                include_exceptionals=True,
                allow_discrete=False,
                cumulation_direction=None,
                is_expert=True):
        """Fill the plot with a one dimensional profile of one variable over another."""

        th1_factory = ROOT.TProfile
        self._is_expert = is_expert

        self.create_1d(th1_factory,
                       xs,
                       ys,
                       weights=weights,
                       stackby=stackby,
                       bins=bins,
                       lower_bound=lower_bound,
                       upper_bound=upper_bound,
                       outlier_z_score=outlier_z_score,
                       include_exceptionals=include_exceptionals,
                       allow_discrete=allow_discrete,
                       cumulation_direction=cumulation_direction)

        if not self.ylabel and self.is_binary(ys):
            self.ylabel = 'probability'

        return self

    def scatter(self,
                xs,
                ys,
                stackby=None,
                lower_bound=(None, None),
                upper_bound=(None, None),
                outlier_z_score=(None, None),
                include_exceptionals=(True, True),
                max_n_data=100000):
        """Fill the plot with a (unbinned) two dimensional scatter plot"""

        x_lower_bound, y_lower_bound = self.unpack_2d_param(lower_bound)
        x_upper_bound, y_upper_bound = self.unpack_2d_param(upper_bound)
        x_outlier_z_score, y_outlier_z_score = self.unpack_2d_param(outlier_z_score)
        x_include_exceptionals, y_include_exceptionals = self.unpack_2d_param(include_exceptionals)

        x_lower_bound, x_upper_bound = self.determine_range(
            xs,
            lower_bound=x_lower_bound,
            upper_bound=x_upper_bound,
            outlier_z_score=x_outlier_z_score,
            include_exceptionals=x_include_exceptionals
        )

        y_lower_bound, y_upper_bound = self.determine_range(
            ys,
            lower_bound=y_lower_bound,
            upper_bound=y_upper_bound,
            outlier_z_score=y_outlier_z_score,
            include_exceptionals=y_include_exceptionals
        )

        graph = ROOT.TGraph()

        graph.SetName(self.name)
        graph.SetMarkerStyle(6)
        graph.GetHistogram().SetOption("AP")  # <-- this has no effect?

        # Trying to make the graph lines invisible
        color_index = 0  # white
        # Transperent white
        graph.SetLineColorAlpha(color_index, 0)
        graph.SetLineStyle(self.very_sparse_dots_line_style_index)

        # Transport the lower and upper bound as ranges of the axis
        graph.GetXaxis().SetLimits(x_lower_bound, x_upper_bound)
        graph.GetYaxis().SetLimits(y_lower_bound, y_upper_bound)

        self.create(graph,
                    xs=xs,
                    ys=ys,
                    stackby=stackby,
                    reverse_stack=False)

        return self

    def hist2d(self,
               xs,
               ys,
               weights=None,
               stackby=None,
               bins=(None, None),
               lower_bound=(None, None),
               upper_bound=(None, None),
               outlier_z_score=(None, None),
               include_exceptionals=(True, True),
               allow_discrete=(False, False)):
        """Fill the plot with a two dimensional histogram"""

        name = self.name

        x_bins, y_bins = self.unpack_2d_param(bins)
        x_lower_bound, y_lower_bound = self.unpack_2d_param(lower_bound)
        x_upper_bound, y_upper_bound = self.unpack_2d_param(upper_bound)
        x_outlier_z_score, y_outlier_z_score = self.unpack_2d_param(outlier_z_score)
        x_include_exceptionals, y_include_exceptionals = self.unpack_2d_param(include_exceptionals)
        x_allow_discrete, y_allow_discrete = self.unpack_2d_param(allow_discrete)

        x_bin_edges, x_bin_labels = self.determine_bin_edges(xs,
                                                             bins=x_bins,
                                                             lower_bound=x_lower_bound,
                                                             upper_bound=x_upper_bound,
                                                             outlier_z_score=x_outlier_z_score,
                                                             include_exceptionals=x_include_exceptionals,
                                                             allow_discrete=x_allow_discrete)

        y_bin_edges, y_bin_labels = self.determine_bin_edges(ys,
                                                             bins=y_bins,
                                                             lower_bound=y_lower_bound,
                                                             upper_bound=y_upper_bound,
                                                             outlier_z_score=y_outlier_z_score,
                                                             include_exceptionals=y_include_exceptionals,
                                                             allow_discrete=y_allow_discrete)

        n_x_bins = len(x_bin_edges) - 1
        n_y_bins = len(y_bin_edges) - 1
        histogram = ROOT.TH2D(name,
                              '',
                              n_x_bins,
                              x_bin_edges,
                              n_y_bins,
                              y_bin_edges)

        if x_bin_labels:
            get_logger().info("Scatter plot %s is discrete in x.", name)
            x_taxis = histogram.GetXaxis()
            for i_x_bin, x_bin_label in enumerate(x_bin_labels):
                x_taxis.SetBinLabel(i_x_bin + 1, x_bin_label)
            self.add_stats_entry(histogram, "dx", 0)

        else:
            x_bin_width = x_bin_edges[1] - x_bin_edges[0]
            self.add_stats_entry(histogram, "dx", x_bin_width)

        if y_bin_labels:
            get_logger().info("Scatter plot %s is discrete in y.", name)
            y_taxis = histogram.GetYaxis()
            for i_y_bin, y_bin_label in enumerate(y_bin_labels):
                y_taxis.SetBinLabel(i_y_bin + 1, y_bin_label)
            self.add_stats_entry(histogram, "dy", 0)

        else:
            y_bin_width = y_bin_edges[1] - y_bin_edges[0]
            self.add_stats_entry(histogram, "dy", y_bin_width)

        self.create(histogram, xs, ys=ys, weights=weights, stackby=stackby)

        # Adjust the discrete bins after the filling to be equidistant
        if x_bin_labels:
            for histogram in self.histograms:
                x_taxis = histogram.GetXaxis()
                x_bin_edges = array.array("d", list(range(len(x_bin_labels) + 1)))
                x_taxis.Set(n_x_bins, x_bin_edges)

        # Adjust the discrete bins after the filling to be equidistant
        if y_bin_labels:
            for histogram in self.histogram:
                x_taxis = histogram.GetXaxis()
                y_bin_edges = array.array("d", list(range(len(y_bin_labels) + 1)))
                y_taxis.Set(n_y_bins, y_bin_edges)

        return self

    def fit_gaus(self):
        """Fit a gaus belle curve to a one dimensional histogram"""
        title = "gaus"
        formula = "gaus"
        fit_tf1 = ROOT.TF1("Fit", formula)
        fit_tf1.SetTitle(title)
        fit_tf1.SetParName(0, "n")
        fit_tf1.SetParName(1, "mean")
        fit_tf1.SetParName(2, "std")
        self.fit(fit_tf1, 'LM')

    def fit_line(self):
        """Fit a general line to a one dimensional histogram"""
        title = "line"
        formula = "x++1"
        fit_tf1 = ROOT.TF1("Fit", formula)
        fit_tf1.SetTitle(title)
        fit_tf1.SetParName(0, "slope")
        fit_tf1.SetParName(1, "intercept")
        self.fit(fit_tf1, 'M')

    def fit_const(self):
        """Fit a constant function to a one dimensional histogram"""
        title = "const"
        formula = "[0]"
        fit_tf1 = ROOT.TF1("Fit", formula)
        fit_tf1.SetTitle(title)
        fit_tf1.SetParName(0, "intercept")
        self.fit(fit_tf1, 'M')

    def fit_diag(self):
        """Fit a diagonal line through the origin to a one dimensional histogram"""
        title = "diag"
        formula = "[0]*x"
        fit_tf1 = ROOT.TF1("Fit", formula)
        fit_tf1.SetTitle(title)
        fit_tf1.SetParName(0, "slope")
        self.fit(fit_tf1, 'M')

    def fit(self, formula, options):
        """Fit a user defined function to a one dimensional histogram"""
        plot = self.plot
        if plot is None:
            raise RuntimeError('Validation plot must be filled before it can be fitted.')

        if not isinstance(plot, ROOT.TH1D):
            raise RuntimeError('Fitting is currently implemented / tested for one dimensional, non stacked validation plots.')

        histogram = plot

        xaxis = histogram.GetXaxis()
        n_bins = xaxis.GetNbins()
        lower_bound = xaxis.GetBinLowEdge(1)
        upper_bound = xaxis.GetBinUpEdge(n_bins)

        if isinstance(formula, ROOT.TF1):
            fit_tf1 = formula
            fit_tf1.SetRange(lower_bound, upper_bound)
        else:
            fit_tf1 = ROOT.TF1("Fit",
                               formula,
                               lower_bound,
                               upper_bound)
        get_logger().info('Fitting with %s', fit_tf1.GetExpFormula())

        # Make sure the fitted function is not automatically added since we want to do that one our own.
        # Look for the documentation of TH1::Fit() for details of the options.
        if 'N' not in options:
            options += 'N'

            histogram.Fit(fit_tf1, options)

        self.set_fit_tf1(histogram, fit_tf1)

    def show(self):
        """Show the plot."""
        if self.plot:
            self.plot.Draw()
        else:
            raise ValueError("Can not show a validation plot that has not been filled.")

    def write(self, tdirectory=None):
        """Write the plot to file

        Parameters
        ----------
        tdirectory : ROOT.TDirectory, optional
            ROOT directory to which the plot should be written.
            If omitted write to the current directory
        """
        if not self.plot:
            raise ValueError("Can not write a validation plot that has not been filled.")

        with root_cd(tdirectory):
            ValidationPlot.set_tstyle()
            if self.plot not in self.histograms:
                self.plot.Write()

            for histogram in self.histograms:
                # always disable ROOT's stat plot because it hides items
                meta_options = ["nostats"]

                # add expert option, if requested
                if self.is_expert:
                    meta_options.append("expert")

                # give a custom pvalue warning / error zone if requested
                if self.pvalue_error is not None:
                    meta_options.append("pvalue-error={}".format(self.pvalue_error))
                if self.pvalue_warn is not None:
                    meta_options.append("pvalue-warn={}".format(self.pvalue_warn))

                meta_options_str = reduce(lambda x, y: x + y + ",", meta_options, "")[:-1]
                histogram.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', meta_options_str))
                histogram.Write()

    @property
    def is_expert(self):
        """Getter method if an plot plot is marked as expert plot"""
        return self._is_expert

    @property
    def title(self):
        """Getter for the plot title"""
        return self._title

    @title.setter
    def title(self, title):
        """Setter for the plot title"""
        self._title = title
        if self.plot:
            self.plot.SetTitle(title)
        for histogram in self.histograms:
            histogram.SetTitle(title)

    @property
    def xlabel(self):
        """Getter for the axis label at the x axis"""
        return self._xlabel

    @xlabel.setter
    def xlabel(self, xlabel):
        """Setter for the axis label at the x axis"""
        self._xlabel = xlabel
        for histogram in self.histograms:
            histogram.GetXaxis().SetTitle(xlabel)

    @property
    def ylabel(self):
        """Getter for the axis label at the y axis"""
        return self._ylabel

    @ylabel.setter
    def ylabel(self, ylabel):
        """Setter for the axis label at the y axis"""
        self._ylabel = ylabel
        for histogram in self.histograms:
            histogram.GetYaxis().SetTitle(ylabel)

    @property
    def contact(self):
        """Getter for the contact email address to be displayed on the validation page"""
        return self._contact

    @contact.setter
    def contact(self, contact):
        """Setter for the contact email address to be displayed on the validation page"""
        self._contact = contact
        for histogram in self.histograms:
            found_obj = histogram.FindObject('Contact')
            if not found_obj:
                tnamed = ROOT.TNamed("Contact", contact)
                histogram.GetListOfFunctions().Add(tnamed)
                found_obj = histogram.FindObject('Contact')
            found_obj.SetTitle(contact)

    @property
    def description(self):
        """Getter for the description to be displayed on the validation page"""
        return self._description

    @description.setter
    def description(self, description):
        """Setter for the description to be displayed on the validation page"""
        self._description = description
        for histogram in self.histograms:
            found_obj = histogram.FindObject('Description')
            if not found_obj:
                tnamed = ROOT.TNamed("Description", description)
                histogram.GetListOfFunctions().Add(tnamed)
                found_obj = histogram.FindObject('Description')
            found_obj.SetTitle(description)

    @property
    def check(self):
        """Getter for the check to be displayed on the validation page"""
        return self._check

    @check.setter
    def check(self, check):
        """Setter for the check to be displayed on the validation page"""
        self._check = check
        for histogram in self.histograms:
            found_obj = histogram.FindObject('Check')
            if not found_obj:
                tnamed = ROOT.TNamed("Check", check)
                histogram.GetListOfFunctions().Add(tnamed)
                found_obj = histogram.FindObject('Check')
            found_obj.SetTitle(check)

    # Implementation details #
    # ###################### #

    @staticmethod
    def unpack_2d_param(param):
        """Unpacks a function parameter for the two dimensional plots.

        If it is a pair the first parameter shall apply to the x coordinate
        the second to the y coordinate. In this case the pair is returned as two values

        If something else is given the it is assumed that this parameter should equally apply
        to both coordinates. In this case the same values is return twice as a pair.

        Parameters
        ----------
        param : pair or single value
            Function parameter for a two dimensional plot

        Returns
        -------
        pair
            A pair of values being the parameter for the x coordinate and
            the y coordinate respectively
        """
        try:
            if len(param) == 2:
                x_param, y_param = param
        except TypeError:
            x_param = param
            y_param = param
        return x_param, y_param

    @staticmethod
    def is_binary(xs):
        """Determine if the data consists of boolean values"""
        return statistics.is_binary_series(xs)

    @staticmethod
    def is_discrete(xs, max_n_unique=None):
        """Determine if the data consists of discrete values"""
        return statistics.is_discrete_series(xs, max_n_unique=max_n_unique)

    @staticmethod
    def get_exceptional_values(xs):
        """Find exceptionally frequent values

        Parameters
        ----------
        xs : np.array (1d)
            Data series

        Returns
        -------
        np.array (1d)
            A list of the found exceptional values.
        """
        return statistics.rice_exceptional_values(xs)

    @staticmethod
    def get_robust_mean_and_std(xs):
        """Does an estimation of mean and standard deviation robust against outliers.

        Parameters
        ----------
        xs : np.array (1d)
            Data series

        Returns
        -------
        float, float
           Pair of mean and standard deviation
        """
        x_mean = statistics.truncated_mean(xs)
        x_std = statistics.trimmed_std(xs)
        return x_mean, x_std

    @staticmethod
    def format_bin_label(value):
        """Formats a value to be placed at a tick on an axis."""
        if np.isfinite(value) and value == np.round(value):
            return str(int(value))
        else:
            formated_value = "{:.5g}".format(value)

            # if the label is to long, switch to shorter "e" format
            if len(formated_value) > 8:
                formated_value = "{:.3e}".format(value)
            return formated_value

    def create_1d(self,
                  th1_factory,
                  xs,
                  ys=None,
                  weights=None,
                  bins=None,
                  stackby=None,
                  lower_bound=None,
                  upper_bound=None,
                  outlier_z_score=None,
                  include_exceptionals=True,
                  allow_discrete=False,
                  cumulation_direction=None):
        """Combined factory method for creating a one dimensional histogram or a profile plot."""
        name = self.name

        # Coerce values to a numpy array. Do not copy if already a numpy array.
        xs = np.array(xs, copy=False)

        if ys is not None:
            ys = np.array(ys, copy=False)

        if weights is not None:
            weights = np.array(weights, copy=False)

        bin_edges, bin_labels = self.determine_bin_edges(xs,
                                                         bins=bins,
                                                         lower_bound=lower_bound,
                                                         upper_bound=upper_bound,
                                                         outlier_z_score=outlier_z_score,
                                                         include_exceptionals=include_exceptionals,
                                                         allow_discrete=allow_discrete)

        n_bins = len(bin_edges) - 1
        histogram = th1_factory(name, '', n_bins, bin_edges)

        if bin_labels:
            get_logger().info("One dimensional plot %s is discrete in x.", name)
            x_taxis = histogram.GetXaxis()
            for i_bin, bin_label in enumerate(bin_labels):
                x_taxis.SetBinLabel(i_bin + 1, bin_label)
            self.add_stats_entry(histogram, "dx", 0)

        else:
            bin_width = bin_edges[1] - bin_edges[0]
            self.add_stats_entry(histogram, "dx", bin_width)

        self.create(histogram,
                    xs,
                    ys=ys,
                    weights=weights,
                    stackby=stackby,
                    cumulation_direction=cumulation_direction,
                    reverse_stack=True)
        # Reverse the stack to have the signal distribution at the bottom

        # Adjust the discrete bins after the filling to be equidistant
        if bin_labels:
            for histogram in self.histograms:
                x_taxis = histogram.GetXaxis()
                bin_edges = array.array("d", list(range(len(bin_labels) + 1)))
                x_taxis.Set(n_bins, bin_edges)

    def create(self,
               histogram_template,
               xs,
               ys=None,
               weights=None,
               stackby=None,
               cumulation_direction=None,
               reverse_stack=None):

        histograms = []

        if stackby is None:
            histogram = histogram_template
            self.fill_into(histogram, xs, ys, weights=weights)
            if cumulation_direction is not None:
                histogram = self.cumulate(histogram, cumulation_direction=cumulation_direction)
            histograms.append(histogram)
            plot = histogram

        else:
            stackby = np.array(stackby, copy=False)
            name = histogram_template.GetName()

            histograms = self.fill_into_grouped(histogram_template,
                                                xs,
                                                ys=ys,
                                                weights=weights,
                                                groupbys=stackby,
                                                groupby_label="stack")

            if cumulation_direction is not None:
                histograms = [self.cumulate(histogram, cumulation_direction=cumulation_direction)
                              for histogram in histograms]

            if isinstance(histogram_template, (ROOT.TProfile, ROOT.TGraph)):
                plot = ROOT.TMultiGraph()
            else:
                plot = ROOT.THStack()

            plot.SetName(name + "_stacked")

            # Add the histogram in reverse order such
            # that the signal usually is on the bottom an well visible
            if reverse_stack:
                for histogram in reversed(histograms):
                    if isinstance(histogram, ROOT.TProfile):
                        histogram = self.convert_tprofile_to_tgrapherrors(histogram)
                        plot.Add(histogram, "APZ")
                    else:
                        plot.Add(histogram)
            else:
                for histogram in histograms:
                    if isinstance(histogram, ROOT.TProfile):
                        histogram = self.convert_tprofile_to_tgrapherrors(histogram)
                        plot.Add(histogram, "APZ")
                    else:
                        plot.Add(histogram)

        self.histograms = histograms
        self.plot = plot
        self.attach_attributes()

    @classmethod
    def convert_tprofile_to_tgrapherrors(cls, tprofile):
        if isinstance(tprofile, ROOT.TGraph):
            return tprofile

        x_taxis = tprofile.GetXaxis()
        n_bins = x_taxis.GetNbins()

        bin_ids_with_underflow = list(range(n_bins + 1))
        bin_ids_without_underflow = list(range(1, n_bins + 1))

        bin_centers = np.array([x_taxis.GetBinCenter(i_bin) for i_bin in bin_ids_without_underflow])
        bin_widths = np.array([x_taxis.GetBinWidth(i_bin) for i_bin in bin_ids_without_underflow])
        bin_x_errors = bin_widths / 2.0

        # Now for the histogram content not including the underflow
        bin_contents = np.array([tprofile.GetBinContent(i_bin) for i_bin in bin_ids_without_underflow])
        bin_y_errors = np.array([tprofile.GetBinError(i_bin) for i_bin in bin_ids_without_underflow])

        tgrapherrors = ROOT.TGraphErrors(n_bins, bin_centers, bin_contents, bin_x_errors, bin_y_errors)

        tgrapherrors.GetHistogram().SetOption("APZ")  # <-- This has no effect?

        tgrapherrors.SetLineColor(tprofile.GetLineColor())
        tgrapherrors.SetLineColor(tprofile.GetLineColor())

        # Copy all functions and stats entries
        for tobject in tprofile.GetListOfFunctions():
            tgrapherrors.GetListOfFunctions().Add(tobject.Clone())

        # Add stats entries that are displayed for profile plots
        cls.add_stats_entry(tgrapherrors, 'count', tprofile.GetEntries())

        stats_values = np.array([np.nan] * 6)
        tprofile.GetStats(stats_values)

        sum_w = stats_values[0]
        sum_w2 = stats_values[1]
        sum_wx = stats_values[2]
        sum_wx2 = stats_values[3]
        sum_wy = stats_values[4]
        sum_wy2 = stats_values[5]

        cls.add_stats_entry(tgrapherrors,
                            "x avg",
                            sum_wx / sum_w)

        cls.add_stats_entry(tgrapherrors,
                            "x std",
                            np.sqrt(sum_wx2 * sum_w - sum_wx * sum_wx) / sum_w)

        cls.add_stats_entry(tgrapherrors,
                            "y avg",
                            sum_wy / sum_w)

        cls.add_stats_entry(tgrapherrors,
                            "y std",
                            np.sqrt(sum_wy2 * sum_w - sum_wy * sum_wy) / sum_w)

        cls.add_stats_entry(tgrapherrors,
                            'cov',
                            tgrapherrors.GetCovariance())

        print(" Corr ", tgrapherrors.GetCorrelationFactor())

        cls.add_stats_entry(tgrapherrors,
                            'corr',
                            tgrapherrors.GetCorrelationFactor())

        return tgrapherrors

    def fill_into_grouped(self,
                          histogram_template,
                          xs,
                          ys=None,
                          weights=None,
                          groupbys=None,
                          groupby_label="group"):
        """Fill data into similar histograms in groups indicated by a groupby array"""

        histograms = []
        unique_groupbys = np.unique(groupbys)
        name = histogram_template.GetName()

        for i_value, value in enumerate(unique_groupbys):
            if np.isnan(value):
                indices_for_value = np.isnan(groupbys)
            else:
                indices_for_value = groupbys == value

            # Make a copy of the empty histogram
            histogram_for_value = histogram_template.Clone(name + '_' + str(value))
            i_root_color = i_value + 1

            self.set_color(histogram_for_value, i_root_color)

            if groupby_label:
                self.add_stats_entry(histogram_for_value, groupby_label, value)

            self.fill_into(histogram_for_value,
                           xs,
                           ys=ys,
                           weights=weights,
                           filter=indices_for_value)

            histograms.append(histogram_for_value)

        return histograms

    def set_color(self, tobject, root_i_color):
        """Set the color of the ROOT object.

        By default the line color of a TGraph should be invisible, so do not change it
        For other objects set the marker and the line color

        Parameters
        ----------
        tobject : Plotable object inheriting from TAttLine and TAttMarker such as TGraph or TH1
            Object of which the color should be set.
        root_i_color : int
            Color index of the ROOT color table
        """
        if isinstance(tobject, ROOT.TGraph):
            tobject.SetMarkerColor(root_i_color)
        else:
            tobject.SetLineColor(root_i_color)
            tobject.SetMarkerColor(root_i_color)

    def fill_into(self, plot, xs, ys=None, weights=None, filter=None):
        """Fill the data into the plot object"""
        if isinstance(plot, ROOT.TGraph):
            if ys is None:
                raise ValueError("ys are required for filling a graph")
            self.fill_into_tgraph(plot, xs, ys, filter=filter)
        else:
            self.fill_into_th1(plot, xs, ys, weights=weights, filter=filter)

    def fill_into_tgraph(self, graph, xs, ys, filter=None):
        """Fill the data into a TGraph"""

        # Save some ifs by introducing a dummy slicing as a non filter
        if filter is None:
            filter = slice(None)

        xs = xs[filter]
        ys = ys[filter]

        max_n_data = 100000
        x_n_data = len(xs)
        y_n_data = len(ys)

        if max_n_data:
            if x_n_data > max_n_data or y_n_data > max_n_data:
                get_logger().warning("Number of points in scatter graph %s exceed limit %s" %
                                     (self.name, max_n_data))

                get_logger().warning("Cropping  %s" % max_n_data)

                xs = xs[0:max_n_data]
                ys = ys[0:max_n_data]

        x_axis = graph.GetXaxis()
        y_axis = graph.GetYaxis()

        x_lower_bound = x_axis.GetXmin()
        x_upper_bound = x_axis.GetXmax()

        y_lower_bound = y_axis.GetXmin()
        y_upper_bound = y_axis.GetXmax()

        x_underflow_indices = xs < x_lower_bound
        x_overflow_indices = xs > x_upper_bound

        y_underflow_indices = ys < x_lower_bound
        y_overflow_indices = ys > x_upper_bound

        plot_indices = ~(np.isnan(xs) |
                         x_underflow_indices |
                         x_overflow_indices |
                         np.isnan(ys) |
                         y_underflow_indices |
                         y_overflow_indices)

        n_plot_data = np.sum(plot_indices)
        plot_xs = xs[plot_indices]
        plot_ys = ys[plot_indices]

        graph.Set(int(n_plot_data))
        for i, (x, y) in enumerate(zip(plot_xs, plot_ys)):
            graph.SetPoint(i, x, y)

        self.add_stats_entry(graph, 'count', np.sum(np.isfinite(xs)))

        self.add_nan_inf_stats(graph, 'x', xs)
        self.add_nan_inf_stats(graph, 'y', ys)

        x_n_underflow = np.sum(x_underflow_indices)
        if x_n_underflow:
            self.add_stats_entry(graph, 'x underf.', x_n_underflow)

        x_n_overflow = np.sum(x_overflow_indices)
        if x_n_overflow:
            self.add_stats_entry(graph, 'x overf.', x_n_overflow)

        y_n_underflow = np.sum(y_underflow_indices)
        if y_n_underflow:
            self.add_stats_entry(graph, 'y underf.', y_n_underflow)

        y_n_overflow = np.sum(y_overflow_indices)
        if y_n_overflow:
            self.add_stats_entry(graph, 'y overf.', y_n_overflow)

        self.add_stats_entry(graph, 'x avg', graph.GetMean(1))
        self.add_stats_entry(graph, 'x std', graph.GetRMS(1))

        self.add_stats_entry(graph, 'y avg', graph.GetMean(2))
        self.add_stats_entry(graph, 'y std', graph.GetRMS(2))

        self.add_stats_entry(graph, 'cov', graph.GetCovariance())
        self.add_stats_entry(graph, 'corr', graph.GetCorrelationFactor())

    def fill_into_th1(self, histogram, xs, ys=None, weights=None, filter=None):
        """Fill the histogram blocking non finite values

        Parameters
        ----------
        histogram : ROOT.TH1
            The histogram to be filled
        xs : numpy.ndarray (1d)
            Data for the first axes
        ys : numpy.ndarray (1d), optional
            Data for the second axes
        weights : numpy.ndarray (1d), optional
            Weight of the individual points. Defaults to one for each
        filter : numpy.ndarray, optional
            Boolean index array indicating which entries shall be taken.
        """
        # Save some ifs by introducing a dummy slicing as a non filter
        if filter is None:
            filter = slice(None)

        xs = xs[filter]
        # Count the nan and inf values in x
        self.add_nan_inf_stats(histogram, 'x', xs)
        finite_filter = np.isfinite(xs)

        if ys is not None:
            ys = ys[filter]
            # Count the nan and inf values in y
            self.add_nan_inf_stats(histogram, 'y', ys)
            finite_filter &= np.isfinite(ys)

        if weights is None:
            weights = itertools.repeat(1.0)
        else:
            weights = weights[filter]
            self.add_nan_inf_stats(histogram, 'w', weights)
            finite_filter &= np.isfinite(weights)

        # Now fill the actual histogram
        Fill = histogram.Fill
        if ys is None:
            for (x, weight, passes) in zip(xs, weights, finite_filter):
                if passes:
                    Fill(float(x), float(weight))
        else:
            for (x, y, weight, passes) in zip(xs, ys, weights, finite_filter):
                if passes:
                    Fill(float(x), float(y), float(weight))

        self.set_additional_stats_tf1(histogram)

    @classmethod
    def add_nan_inf_stats(cls, histogram, name, xs):
        """ Extracts the counts of non finite floats from a series
        and adds them as additional statistics to the histogram.

        Parameters
        ----------
        histogram : derived from ROOT.TH1 or ROOT.TGraph
            Something having a GetListOfFunctions method that
        name : str
            A label for the data series to be prefixed to the entries.
        xs : numpy.ndarray (1d)
            Data from which the non finit floats should be counted.
        """
        n_nans = np.isnan(xs).sum()
        if n_nans > 0:
            cls.add_stats_entry(histogram, name + ' nan', n_nans)

        n_positive_inf = np.sum(xs == np.inf)
        if n_positive_inf > 0:
            cls.add_stats_entry(histogram, name + ' +inf', n_positive_inf)

        n_negative_inf = np.sum(xs == -np.inf)
        if n_negative_inf > 0:
            cls.add_stats_entry(histogram, name + ' -inf', n_negative_inf)

    @classmethod
    def add_stats_entry(cls, histogram, label, value):
        """Add a new additional statistics to the histogram.

        Parameters
        ----------
        histogram : derived from ROOT.TH1 or ROOT.TGraph
            Something having a GetListOfFunctions method that holds the additional statistics
        label : str
            Label of the statistic
        value : float
            Value of the statistic
        """
        stats_entry = StatsEntry(str(label), float(value))
        histogram.GetListOfFunctions().Add(stats_entry)
        cls.set_additional_stats_tf1(histogram)

    @classmethod
    def get_additional_stats(cls, histogram):
        """Get the additional statistics from the histogram and return them a dict.

        Parameters
        ----------
        histogram : derived from ROOT.TH1 or ROOT.TGraph
            Something having a GetListOfFunctions method that holds the additional statistics

        Returns
        -------
        collection.OrderedDict
            A map of labels to values for the additional statistics
        """
        additional_stats = collections.OrderedDict()
        for tobject in histogram.GetListOfFunctions():
            if isinstance(tobject, StatsEntry):
                stats_entry = tobject
                label = stats_entry.GetName()
                value = stats_entry.GetVal()
                additional_stats[label] = value
        return additional_stats

    @classmethod
    def cumulate(cls, histogram, cumulation_direction=None):
        """Cumulates the histogram inplace.

        Parameters
        ----------
        histogram : ROOT.TH1 or ROOT.TProfile
            Filled histogram to be cumulated
        cumulation_direction : int, optional
            Direction is indicated by the sign.
            Positive means from left to right, negative means from right to left.
            If now cumulation direction is given return the histogram as is.

        Returns
        -------
        ROOT.TH1
            Cumulated histogram potentially altered inplace.
        """
        if not cumulation_direction:
            return histogram

        cumulate_backward = cumulation_direction < 0
        cumulate_forward = not cumulate_backward

        if isinstance(histogram, ROOT.TH2):
            raise ValueError("Cannot cumulate a two dimensional histogram.")

        if isinstance(histogram, ROOT.TH3):
            raise ValueError("Cannot cumulate a three dimensional histogram.")

        if not isinstance(histogram, ROOT.TH1):
            raise ValueError("Can only cumulate a one dimensional histogram.")

        if isinstance(histogram, ROOT.TProfile):
            tprofile = histogram
            # Turn the profile histogram into graph where we can set the new content and errors
            tgraph = cls.convert_tprofile_to_tgrapherrors(histogram)
            tgraph.SetName(tprofile.GetName())

            n_bins = histogram.GetNbinsX()
            n_points = n_bins
            cumulated_content = 0.0
            cumulated_entries = 0
            cumulated_std = 0.0

            # Always include the overflow bins.
            i_bins = list(range(0, n_bins + 2))
            if not cumulate_forward:
                i_bins = reversed(i_bins)

            for i_bin in i_bins:
                i_point = i_bin - 1
                bin_content = tprofile.GetBinContent(i_bin)
                bin_entries = tprofile.GetBinEffectiveEntries(i_bin)
                bin_std = tprofile.GetBinError(i_bin)

                if bin_entries != 0:
                    cumulated_content = (
                        1.0 * (cumulated_entries * cumulated_content + bin_entries * bin_content) /
                        (cumulated_entries + bin_entries)
                    )

                    cumulated_std = (
                        math.hypot(cumulated_entries * cumulated_std, bin_entries * bin_std) /
                        (cumulated_entries + bin_entries)
                    )

                    cumulated_entries = cumulated_entries + bin_entries
                else:
                    # empty bin does not add anything to the cumulation
                    pass

                if i_point >= 0 and i_point < n_points:
                    x = tgraph.GetX()[i_point]
                    # x = ROOT.Double()
                    # y = ROOT.Double()
                    # tgraph.GetPoint(i_point, x, y)
                    tgraph.SetPoint(i_point, x, cumulated_content)

                    x_error = tgraph.GetErrorX(i_point)
                    tgraph.SetPointError(i_point, x_error, cumulated_std)
            return tgraph

        else:
            # Always include the overflow bins.
            n_bins = histogram.GetNbinsX()
            cumulated_content = 0.0

            i_bins = list(range(0, n_bins + 2))
            if not cumulate_forward:
                i_bins = reversed(i_bins)

            for i_bin in i_bins:
                bin_content = histogram.GetBinContent(i_bin)
                cumulated_content += bin_content
                histogram.SetBinContent(i_bin, cumulated_content)

            return histogram

    def determine_bin_edges(self,
                            xs,
                            bins=None,
                            lower_bound=None,
                            upper_bound=None,
                            outlier_z_score=None,
                            include_exceptionals=True,
                            allow_discrete=False):
        """Deducing bin edges from a data series.

        Parameters
        ----------
        xs : numpy.ndarray (1d)
            Data point for which a binning should be found.
        bins : list(float) or int or None, optional
            Preset bin edges or preset number of desired bins.
            The default, None, means the bound should be extracted from data.
            The rice rule is used the determine the number of bins.
            If a list of floats is given return them immediatly.
        lower_bound : float or None, optional
            Preset lower bound of the binning range.
            The default, None, means the bound should be extracted from data.
        upper_bound : float or None, optional
            Preset upper bound of the binning range.
            The default, None, means the bound should be extracted from data.
        outlier_z_score : float or None, optional
            Threshold z-score of outlier detection.
            The default, None, means no outlier detection.
        include_exceptionals : bool, optional
            If the outlier detection is active this switch indicates,
            if values detected as exceptionally frequent shall be included
            nevertheless into the binning range. Default is True,
            which means exceptionally frequent values as included
            even if they are detected as outliers.

        Returns
        -------
        np.array (1d), list(str)
            Pair of bin edges and labels deduced from the series.
            Second element is None if the series is not detected as discrete.
        """
        debug = get_logger().debug
        debug('Determine binning for plot named %s', self.name)

        if isinstance(bins, collections.Iterable):
            # Bins is considered as an array
            # Construct a float array forwardable to root.
            bin_edges = bins
            bin_edges = array.array('d', bin_edges)
            bin_labels = None
            return bin_edges, bin_labels

        # If bins is not an iterable assume it is the number of bins or None
        if bins is None:
            n_bins = None
        else:
            # Check that bins can be coerced to an integer.
            n_bins = int(bins)

            # Do not allow negative bin numbers
            if not n_bins > 0:
                message = 'Cannot accept n_bins=%s as number of bins, because it is not a number greater than 0.' % bins
                raise ValueError(message)

        # Coerce values to a numpy array. Do not copy if already a numpy array.
        xs = np.array(xs, copy=False)

        if self.is_binary(xs) or (allow_discrete and self.is_discrete(xs)):
            # This covers also the case
            debug('Discrete binning values encountered')
            finite_xs = xs[np.isfinite(xs)]
            unique_xs = np.unique(finite_xs)

            # Crop the unique values between the lower and upper bound
            if lower_bound is None:
                if len(unique_xs) == 0:
                    if upper_bound is None:
                        lower_bound = 0
                    else:
                        lower_bound = upper_bound - 1
                else:
                    lower_bound = np.min(unique_xs)
            else:
                unique_xs = unique_xs[unique_xs >= lower_bound]

            if upper_bound is None:
                if len(unique_xs) == 0:
                    upper_bound = lower_bound + 1
                else:
                    upper_bound = np.min(unique_xs)
            else:
                unique_xs = unique_xs[unique_xs <= upper_bound]

            if n_bins is None:
                n_bins = len(unique_xs) or 1

            if len(unique_xs) > 0 and n_bins >= len(unique_xs):
                # Construct a float array forwardable to root.
                bin_edges = array.array('d', unique_xs)
                format_bin_label = self.format_bin_label
                bin_labels = [format_bin_label(value) for value in bin_edges]
                bin_edges.append(bin_edges[-1] + 1)
                return bin_edges, bin_labels

            else:
                # Ambiguous case what to do in case of a number of requested bins
                # that is lower than the number of unique values?

                # Continue with an equistant binning for now.
                pass

            debug('Lower bound %s', lower_bound)
            debug('Upper bound %s', upper_bound)
            debug('N bins %s', n_bins)

        else:
            bin_range = self.determine_bin_range(xs,
                                                 n_bins=n_bins,
                                                 lower_bound=lower_bound,
                                                 upper_bound=upper_bound,
                                                 outlier_z_score=outlier_z_score,
                                                 include_exceptionals=include_exceptionals)

            n_bins, lower_bound, upper_bound = bin_range

        n_bin_edges = n_bins + 1
        if lower_bound != upper_bound:
            # Correct the upper bound such that all values are strictly smaller than the upper bound
            # Make one step in single precision in the positive direction
            bin_edges = np.linspace(lower_bound, upper_bound, n_bin_edges)

            # Reinforce the upper and lower bound to be exact
            # Also expand the upper bound by an epsilon
            # to prevent the highest value in xs from going in the overflow bin
            bin_edges[0] = lower_bound
            bin_edges[-1] = np.nextafter(upper_bound, np.inf)
            debug('Bins %s', bin_edges)

        else:
            # Fall back if the array contains only one value
            bin_edges = [lower_bound, upper_bound + 1]

        # Construct a float array forwardable to root.
        bin_edges = array.array('d', bin_edges)
        debug('Bins %s for %s', bin_edges, self.name)
        return bin_edges, None

    def determine_bin_range(self,
                            xs,
                            n_bins=None,
                            lower_bound=None,
                            upper_bound=None,
                            outlier_z_score=None,
                            include_exceptionals=True):
        """Calculates the number of bins, the lower bound and the upper bound from a given data series
        estimating the values that are not given.

        If the outlier_z_score is given the method tries to exclude outliers that exceed a certain z-score.
        The z-score is calculated (x - x_mean) / x_std. The be robust against outliers the necessary
        mean and std deviation are based on truncated mean and a trimmed std calculated from the inter
        quantile range (IQR).

        If additional include_exceptionals is true the method tries to find exceptional values in the series
        and always include them in the range if it finds any.
        Exceptional values means exact values that appear often in the series for whatever reason.
        Possible reasons include
        * Interal / default values
        * Failed evaluation conditions
        * etc.
        which should be not cropped away automatically if you are locking on the quality of your data.

        Parameters
        ----------
        xs : numpy.ndarray (1d)
            Data point for which a binning should be found.
        n_bins : int or None, optional
            Preset number of desired bins. The default, None, means the bound should be extracted from data.
            The rice rule is used the determine the number of bins.
        lower_bound : float or None, optional
            Preset lower bound of the binning range.
            The default, None, means the bound should be extracted from data.
        upper_bound : float or None, optional
            Preset upper bound of the binning range.
            The default, None, means the bound should be extracted from data.
        outlier_z_score : float or None, optional
            Threshold z-score of outlier detection.
            The default, None, means no outlier detection.
        include_exceptionals : bool, optional
            If the outlier detection is active this switch indicates,
            if values detected as exceptionally frequent shall be included
            nevertheless into the binning range. Default is True,
            which means exceptionally frequent values as included
            even if they are detected as outliers.

        Returns
        -------
        n_bins, lower_bound, upper_bound : int, float, float
            A triple of found number of bins, lower bound and upper bound of the binning range.
        """

        finite_xs_indices = np.isfinite(xs)
        if np.any(finite_xs_indices):
            finite_xs = xs[finite_xs_indices]
        else:
            finite_xs = xs

        lower_bound, upper_bound = self.determine_range(finite_xs,
                                                        lower_bound=lower_bound,
                                                        upper_bound=upper_bound,
                                                        outlier_z_score=outlier_z_score,
                                                        include_exceptionals=include_exceptionals)

        if n_bins is None:
            # Assume number of bins according to the rice rule.
            # The number of data points should not include outliers.
            n_data = np.sum((lower_bound <= finite_xs) & (finite_xs <= upper_bound))
            rice_n_bins = int(statistics.rice_n_bin(n_data))
            n_bins = rice_n_bins

        else:
            n_bins = int(n_bins)
            # Do not allow negative bin numbers
            if not n_bins > 0:
                message = 'Cannot accept n_bins=%s as number of bins, because it is not a number greater than 0.' % bins
                raise ValueError(message)

        return n_bins, lower_bound, upper_bound

    def determine_range(self,
                        xs,
                        lower_bound=None,
                        upper_bound=None,
                        outlier_z_score=None,
                        include_exceptionals=True):
        """
        Parameters
        ----------
        xs : numpy.ndarray (1d)
            Data point for which a binning should be found.
        lower_bound : float or None, optional
            Preset lower bound of the binning range.
            The default, None, means the bound should be extracted from data.
        upper_bound : float or None, optional
            Preset upper bound of the binning range.
            The default, None, means the bound should be extracted from data.
        outlier_z_score : float or None, optional
            Threshold z-score of outlier detection.
            The default, None, means no outlier detection.
        include_exceptionals : bool, optional
            If the outlier detection is active this switch indicates,
            if values detected as exceptionally frequent shall be included
            nevertheless into the binning range. Default is True,
            which means exceptionally frequent values as included
            even if they are detected as outliers.

        Returns
        -------
        lower_bound, upper_bound : float, float
            A pair of found lower bound and upper bound of series.
        """
        debug = get_logger().debug

        finite_xs_indices = np.isfinite(xs)
        if np.any(finite_xs_indices):
            finite_xs = xs[finite_xs_indices]
        else:
            finite_xs = xs

        # Prepare for the estimation of outliers
        if outlier_z_score is not None and (lower_bound is None or upper_bound is None):

            x_mean, x_std = self.get_robust_mean_and_std(finite_xs)

            lower_exceptional_x = np.nan
            upper_exceptional_x = np.nan

            if include_exceptionals:
                exceptional_xs = self.get_exceptional_values(finite_xs)
                if len(exceptional_xs):
                    lower_exceptional_x = np.min(exceptional_xs)
                    upper_exceptional_x = np.max(exceptional_xs)

        # Find the lower bound, if it is not given.
        if lower_bound is None:
            lower_bound = np.min(finite_xs)
            # Clip the lower bound by outliers that exceed the given z score
            if outlier_z_score is not None:
                # The lower bound at which outliers exceed the given z score
                lower_outlier_bound = x_mean - outlier_z_score * x_std

                # Clip the lower bound such that it concides with an actual value,
                # which prevents empty bins from being produced
                indices_above_lower_outlier_bound = finite_xs >= lower_outlier_bound

                if np.any(indices_above_lower_outlier_bound):
                    lower_bound = np.min(finite_xs[indices_above_lower_outlier_bound])

                    # However we want to include at least the exceptional values in the range if there are any.
                    lower_bound = np.nanmin([lower_bound, lower_exceptional_x])

                debug('Lower bound after outlier detection')
                debug('Lower bound %s', lower_bound)
                debug('Lower outlier bound %s', lower_outlier_bound)

        # Find the upper bound, if it is not given
        if upper_bound is None:
            upper_bound = np.max(finite_xs)
            if outlier_z_score is not None:
                # The upper bound at which outliers exceed the given z score
                upper_outlier_bound = x_mean + outlier_z_score * x_std

                # Clip the upper bound such that it concides with an actual value,
                # which prevents empty bins from being produced
                indices_below_upper_outlier_bound = finite_xs <= upper_outlier_bound

                if np.any(indices_below_upper_outlier_bound):
                    upper_bound = np.max(finite_xs[indices_below_upper_outlier_bound])

                    # However we want to include at least the exceptional values in the range if there are any.
                    upper_bound = np.nanmax([upper_bound, upper_exceptional_x])

                debug('Upper bound after outlier detection')
                debug('Upper bound %s', upper_bound)
                debug('Upper outlier bound %s', upper_outlier_bound)

        return lower_bound, upper_bound

    @classmethod
    def set_additional_stats_tf1(cls, histogram):
        """Combining fit TF1 with the additional statistics and attach them to the histogram.

        Parameters
        ----------
        histogram : ROOT.TH1 or ROOT.TGraph or ROOT.TMultiGraph
            Something having a GetListOfFunctions method that should hold
            the combined fit and additional statistics function.
        """
        additional_stats_tf1 = cls.create_additional_stats_tf1(histogram)
        cls.set_tf1(histogram, additional_stats_tf1)

    @classmethod
    def set_fit_tf1(cls, histogram, fit_tf1):
        """Combining fit TF1 with the additional statistics and attach them to the histogram.

        Parameters
        ----------
        histogram : ROOT.TH1 or ROOT.TGraph or ROOT.TMultiGraph
            Something having a GetListOfFunctions method that should hold
            the combined fit and additional statistics function.
        """
        additional_stats_tf1 = cls.create_additional_stats_tf1(histogram)
        combined_tf1 = cls.combine_fit_and_additional_stats(fit_tf1, additional_stats_tf1)
        cls.set_tf1(histogram, combined_tf1)

    @classmethod
    def set_tf1(cls, histogram, tf1):
        """Set the attached TF1 of the histogram.

        Parameters
        ----------
        histogram : ROOT.TH1 or ROOT.TGraph or ROOT.TMultiGraph
            Something having a GetListOfFunctions method that should hold
            the combined fit and additional statistics function.
        """
        # Delete any functions formally added
        cls.delete_tf1(histogram)
        if tf1 is not None:
            tf1.SetName("FitAndStats")
            histogram.GetListOfFunctions().Add(tf1)

    @classmethod
    def delete_tf1(cls, histogram):
        """Delete the attached TF1 from the histogram

        Parameters
        ----------
        histogram : ROOT.TH1 or ROOT.TGraph
            Something having a GetListOfFunctions method that holds the fit function
        """
        tf1 = histogram.FindObject("FitAndStats")
        if tf1:
            function_list = histogram.GetListOfFunctions()
            function_list.Remove(tf1)

    @classmethod
    def create_additional_stats_tf1(cls, histogram):
        """Create a TF1 with the additional statistics from the histogram as parameters.

        Parameters
        ----------
        histogram : ROOT.TH1 or ROOT.TGraph
            Something having a GetListOfFunctions method that holds the additional statistics.

        Returns
        -------
        ROOT.TF1
            Function with the additional statistics as parameters.
        """

        additional_stats = cls.get_additional_stats(histogram)
        if not additional_stats:
            return None

        # Create dummy function, which displays additional statistics in the legend, when added to a histogram.
        # Dummy range to serve the functions
        lower_bound = 0
        upper_bound = 0

        # Create a formula which is zero in all cases but has space for n parameters
        # Formula string looks like 0*[0]+0*[1]+0*[2]+...
        formula_string = '+'.join('0*[' + str(i) + ']' for i in range(len(additional_stats)))

        # Compose a function that carries the addtional information
        additional_stats_tf1 = ROOT.TF1("Stats", formula_string, lower_bound, upper_bound)

        for (i, (label, value)) in enumerate(additional_stats.items()):
            # root 6 does not like labels with spaces ..
            # bug report:  https://sft.its.cern.ch/jira/browse/ROOT-7460
            # therefor this workaround:
            label = label.replace(" ", "-")
            additional_stats_tf1.SetParName(i, label)
            additional_stats_tf1.FixParameter(i, value)

        return additional_stats_tf1

    @classmethod
    def combine_fit_and_additional_stats(cls, fit_tf1, additional_stats_tf1):
        """Combine the fit function and the function carrying the additional statistics to one function.

        Parameters
        ----------
        fit_tf1 : ROOT.TF1
            The fit function
        additional_stats_tf1 : ROOT.TF1
            The function carrying the additional statistics as parameters

        Returns
        -------
        ROOT.TF1
        """
        if additional_stats_tf1 is None:
            return fit_tf1

        # Combine both TF1 functions
        # Get the lower and upper bound of the fit
        # Use the pass-by reference containers from pyROOT to be able to call the function
        lower_bound = ROOT.Double()
        upper_bound = ROOT.Double()
        fit_tf1.GetRange(lower_bound, upper_bound)
        title = fit_tf1.GetTitle()

        combined_formula = additional_stats_tf1.GetName() + '+' + fit_tf1.GetName()
        combined_tf1 = ROOT.TF1("Combined", combined_formula, lower_bound, upper_bound)
        combined_tf1.SetTitle(title)

        # Transfer the fitted parameters
        chi2 = fit_tf1.GetChisquare()
        combined_tf1.SetChisquare(chi2)

        ndf = fit_tf1.GetNDF()
        combined_tf1.SetNDF(ndf)

        n_stats_parameters = additional_stats_tf1.GetNpar()
        n_fit_parameters = fit_tf1.GetNpar()
        cls.copy_tf1_parameters(additional_stats_tf1, combined_tf1)
        cls.copy_tf1_parameters(fit_tf1, combined_tf1, offset=n_stats_parameters)

        return combined_tf1

    @classmethod
    def copy_tf1_parameters(cls, tf1_source, tf1_target, offset=0):
        """Copy the parameters of one TF1 to  another.

        Parameters
        ----------
        tf1_source : ROOT.TF1
            Take parameters from here
        tf1_target : ROOT.TF1
            Copy them to here.
        offset : int, optional
            Index of the first target parameter to which to copy.
        """
        n_parameters = tf1_source.GetNpar()

        # Helper variables for pyROOT's mechanism to call functions by reference
        lower_bound = ROOT.Double()
        upper_bound = ROOT.Double()

        for i_parameter in range(n_parameters):
            i_source = i_parameter
            i_target = i_parameter + offset

            tf1_target.SetParameter(i_target,
                                    tf1_source.GetParameter(i_source))
            tf1_target.SetParError(i_target, tf1_source.GetParError(i_source))
            tf1_target.SetParName(i_target, tf1_source.GetParName(i_source))

            tf1_source.GetParLimits(i_source, lower_bound, upper_bound)
            tf1_target.SetParLimits(i_target, lower_bound, upper_bound)

    def attach_attributes(self):
        """Reassign the special attributes of the plot forwarding them to the ROOT plot."""
        # Forward the attributes to the plot by auto assignment
        self.check = self.check
        self.contact = self.contact
        self.description = self.description

        self.xlabel = self.xlabel
        self.ylabel = self.ylabel
        self.title = self.title

    @classmethod
    def set_tstyle(cls):
        """Set the style such that the additional stats entries are shown by the TBrowser"""
        belle2_validation_style_name = "belle2_validation_style"
        belle2_validation_tstyle = ROOT.gROOT.GetStyle(belle2_validation_style_name)
        if not belle2_validation_tstyle:
            belle2_validation_tstyle = ROOT.TStyle(belle2_validation_style_name, belle2_validation_style_name)

            opt_fit = 112
            belle2_validation_tstyle.SetOptFit(opt_fit)

            opt_stat = 111111
            belle2_validation_tstyle.SetOptStat(opt_stat)
            ROOT.gROOT.SetStyle(belle2_validation_style_name)

            belle2_validation_tstyle.SetLineStyleString(cls.very_sparse_dots_line_style_index, "4 2000")

        else:
            belle2_validation_tstyle.cd()


def test():
    """Simple test methode"""
    ValidationPlot.set_tstyle()

    # Test a histogram plot with some nan and inf values
    normal_distributed_values = np.random.randn(1000)

    for i in range(10):
        normal_distributed_values[i] = np.nan

    for i in range(10, 20):
        normal_distributed_values[i] = np.inf

    for i in range(20, 30):
        normal_distributed_values[i] = -np.inf

    validation_histogram = ValidationPlot('test_hist')
    validation_histogram.hist(normal_distributed_values)
    validation_histogram.title = 'A normal distribution'
    validation_histogram.xlabel = 'normal'
    validation_histogram.ylabel = 'frequency'
    validation_histogram.fit_gaus()

    # Test for a cumulated histogram - cumulation from left to right
    cumulated_histogram = ValidationPlot('test_cum_hist')
    cumulated_histogram.hist(normal_distributed_values, cumulation_direction=1)
    cumulated_histogram.title = 'A cumulated normal distribution'
    cumulated_histogram.xlabel = 'normal'
    cumulated_histogram.ylabel = 'cdf'
    cumulated_histogram.show()

    # Test stacked histogram
    # Make a random selection of 40%
    stackby = np.random.binomial(1.0, 0.40, 1000)
    stacked_validation_histogram = ValidationPlot('test_stacked_hist')
    stacked_validation_histogram.hist(normal_distributed_values, stackby=stackby)

    # Make a scatterplot with two species.
    x = np.random.randn(1000)
    y = 3 * np.random.randn(1000)
    ones = np.ones_like(x)
    angle = np.pi / 2

    x1 = np.where(stackby != 0, np.cos(angle) * ones, ones) * x + np.where(stackby != 0, np.sin(angle) * ones, ones) * y
    y1 = np.where(stackby != 0, np.sin(angle) * ones, ones) * x - np.where(stackby != 0, np.cos(angle) * ones, ones) * y

    stacked_validation_scatter = ValidationPlot('test_stacked_scatter')
    stacked_validation_scatter.scatter(x1, y1, stackby=stackby)

    # Make a profile plot with the two species
    stacked_validation_profile = ValidationPlot('test_stacked_profile')
    stacked_validation_profile.profile(x1, y1, stackby=stackby)

    # Make a two dimensional histogram with two species
    stacked_validation_hist2d = ValidationPlot('test_stacked_hist2d')
    stacked_validation_hist2d.hist2d(x1, y1, stackby=stackby)

    # Test a profile with a diagonal fit
    x = np.linspace(-1, 1, 1000)
    y = x.copy()
    x[0] = np.nan
    diagonal_plot = ValidationPlot('test_diag')
    diagonal_plot.profile(x, y, bins=50)
    diagonal_plot.fit_line()

    # Test if cumulation works for profile plots
    cumulated_profile = ValidationPlot('test_cum_profile')
    cumulated_profile.profile(x, y, bins=50, cumulation_direction=1)

    tfile = ROOT.TFile('test.root', 'RECREATE')

    validation_histogram.write(tfile)

    with root_cd("expert") as tdirectory1:
        diagonal_plot.write(tdirectory1)
        cumulated_profile.write(tdirectory1)
        cumulated_histogram.write(tdirectory1)

    with root_cd("stacked") as tdirectory2:
        stacked_validation_histogram.write(tdirectory2)
        stacked_validation_scatter.write()
        stacked_validation_profile.write()
        stacked_validation_hist2d.write()

    tfile.Close()

    tfile = ROOT.TFile('test.root')
    tBrowser = ROOT.TBrowser()
    tBrowser.BrowseObject(tfile)
    input()
    tfile.Close()


if __name__ == '__main__':
    test()
