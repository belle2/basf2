#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import itertools
import collections
import array
import numpy as np

import ROOT

from tracking.validation.utilities import root_cd, root_save_name

import logging


def get_logger():
    return logging.getLogger(__name__)


default_max_n_unique_for_discrete = 20


def iqr(array_like):
    """Computes the interquantile range, hence the distance from the 25% to 75% quantile."""

    return np.percentile(array_like, 75) - np.percentile(array_like, 25)


normal_iqr_to_std_factor = 2.0 * math.sqrt(2.0) * math.erf(0.5)


def trimmed_std(array_like):
    """A trimmed estimate for the standard deviation of a normal distribution
    contaminated by outliers using the interquanitle range times the appropriate factor.
    """

    return normal_iqr_to_std_factor * iqr(array_like)


def truncated_mean(array_like, r=0.23):
    """Calculates the truncated mean, where 2r is the fraction of data to be taken into account.

    The truncated mean is a robust estimator for the central value of a symmetric distribution.
    The (1-r) upper and the (1-r) lower values are left out and only the central remaining 2r fraction enters
    the normal calculation of the mean. The default value of r is the text book value, which produces an approximatelly
    82%-efficient estimate of the mean for normal, central value of the cauchy and the double-exponential
    distribution.
    """

    truncation = 1 - 2 * r

    if truncation >= 1 or truncation <= 0:
        raise ValueError('Value of r must be between 0 and 0.5')

    array = np.array(array_like)

    lower_percentile = 100 * truncation / 2.0
    upper_percentile = 100 * (1.0 - truncation / 2.0)

    (lower_bound, upper_bound) = np.percentile(array, (lower_percentile,
                                                       upper_percentile))

    weights = (array >= lower_bound) & (array <= upper_bound)

    truncated_array = array[weights]

    return np.mean(truncated_array)


def cubic_root(x):
    return pow(float(x), 1.0 / 3.0)


def rice_n_bin(n_data):
    return np.ceil(2.0 * cubic_root(n_data))


def rice_exceptional_values(xs):
    unique_xs, indices = np.unique(xs, return_inverse=True)
    # Note: The indices are such that unique_xs[indices] == xs

    unique_xs_count = np.bincount(indices)

    exceptional_xs = []

    while True:
        n_data = np.sum(unique_xs_count)
        n_exceptional = 1.0 * n_data / rice_n_bin(n_data)
        excpetional_indices = unique_xs_count > n_exceptional

        if np.any(excpetional_indices):
            exceptional_xs.extend(unique_xs[excpetional_indices])
            unique_xs_count[excpetional_indices] = 0
        else:
            break

    return np.array(sorted(exceptional_xs))


units_by_quantity_name = {
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


def compose_axis_label(quantity_name, unit=None):
    if unit is None:
        unit = units_by_quantity_name.get(quantity_name, None)

    if unit is None:
        axis_label = quantity_name
    else:
        axis_label = '%s (%s)' % (quantity_name, unit)

    return axis_label


class ValidationPlot(object):

    def __init__(self, name):
        self.name = root_save_name(name)

        self._description = ''
        self._check = ''
        self._contact = ''

        self._xlabel = ''
        self._ylabel = ''
        self._title = ''

        self.plot = None
        self.histograms = []

        self.additional_stats_by_name = {}

    def hist(self,
             xs,
             weights=None,
             stackby=None,
             bins=None,
             lower_bound=None,
             upper_bound=None,
             outlier_z_score=None,
             include_exceptionals=True,
             allow_discrete=False):

        th1_factory = ROOT.TH1D

        self.create_1d(th1_factory,
                       xs,
                       weights=weights,
                       stackby=stackby,
                       bins=bins,
                       lower_bound=lower_bound,
                       upper_bound=upper_bound,
                       outlier_z_score=outlier_z_score,
                       include_exceptionals=include_exceptionals,
                       allow_discrete=allow_discrete)

        if self.ylabel is None:
            self.ylabel = 'Count'

        return self

    def profile(self,
                xs,
                ys,
                weights=None,
                bins=None,
                lower_bound=None,
                upper_bound=None,
                outlier_z_score=None,
                include_exceptionals=True,
                allow_discrete=False):

        th1_factory = ROOT.TProfile

        self.create_1d(th1_factory,
                       xs,
                       ys,
                       weights=weights,
                       bins=bins,
                       lower_bound=lower_bound,
                       upper_bound=upper_bound,
                       outlier_z_score=outlier_z_score,
                       include_exceptionals=include_exceptionals,
                       allow_discrete=allow_discrete)

        if self.ylabel is None and self.is_binary(ys):
            self.ylabel = 'Probability'

        return self

    def scatter(self,
                xs,
                ys,
                weights=None,
                bins=(None, None),
                lower_bound=(None, None),
                upper_bound=(None, None),
                outlier_z_score=(None, None),
                include_exceptionals=(True, True),
                allow_discrete=(False, False)):

        name = self.name

        try:
            if len(bins) == 2:
                (x_bins, y_bins) = bins
        except TypeError:
            x_bins = bins
            y_bins = bins

        try:
            if len(lower_bound) == 2:
                (x_lower_bound, y_lower_bound) = lower_bound
        except TypeError:
            x_lower_bound = lower_bound
            y_lower_bound = lower_bound

        try:
            if len(upper_bound) == 2:
                (x_upper_bound, y_upper_bound) = upper_bound
        except TypeError:
            x_upper_bound = upper_bound
            y_upper_bound = upper_bound

        try:
            if len(outlier_z_score) == 2:
                (x_outlier_z_score, y_outlier_z_score) = outlier_z_score
        except TypeError:
            x_outlier_z_score = outlier_z_score
            y_outlier_z_score = outlier_z_score

        try:
            if len(include_exceptionals) == 2:
                (x_include_exceptionals, y_include_exceptionals) = include_exceptionals
        except TypeError:
            x_include_exceptionals = include_exceptionals
            y_include_exceptionals = include_exceptionals

        try:
            if len(allow_discrete) == 2:
                (x_allow_discrete, y_allow_discrete) = allow_discrete
        except TypeError:
            x_allow_discrete = allow_discrete
            y_allow_discrete = allow_discrete

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

        if y_bin_labels:
            get_logger().info("Scatter plot %s is discrete in y.", name)
            y_taxis = histogram.GetYaxis()
            for i_y_bin, y_bin_label in enumerate(y_bin_labels):
                y_taxis.SetBinLabel(i_y_bin + 1, y_bin_label)

        histograms = []

        self.fill_into(histogram, xs, ys, weights=weights)

        histograms.append(histogram)
        plot = histogram

        # Adjust the discrete bins after the filling to be equidistant
        if x_bin_labels:
            x_taxis = histogram.GetXaxis()
            x_bin_edges = array.array("d", range(len(x_bin_labels) + 1))
            x_taxis.Set(n_x_bins, x_bin_edges)

        # Adjust the discrete bins after the filling to be equidistant
        if y_bin_labels:
            x_taxis = histogram.GetXaxis()
            y_bin_edges = array.array("d", range(len(y_bin_labels) + 1))
            y_taxis.Set(n_y_bins, y_bin_edges)

        self.histograms = histograms
        self.plot = plot

        # Add description, check, contact and titles to the histogram
        self.attach_attributes()

        return self

    def fit_gaus(self):
        self.fit('gaus', 'LM')

    def fit_line(self):
        self.fit('x++1', 'M')

    def fit_const(self):
        self.fit('[0]', 'M')

    def fit_diag(self):
        self.fit('[0]*x', 'M')

    def fit(self, formula, options):
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
        if self.plot:
            self.plot.Draw()
        else:
            raise ValueError("Can not show a validation plot that has not been filled.")

    def write(self, tdirectory=None):
        if not self.plot:
            raise ValueError("Can not write a validation plot that has not been filled.")
        with root_cd(tdirectory):
            ValidationPlot.set_tstyle()
            if self.plot not in self.histograms:
                self.plot.Write()

            for histogram in self.histograms:
                histogram.Write()

    @property
    def title(self):
        return self._title

    @title.setter
    def title(self, title):
        self._title = title
        if self.plot:
            self.plot.SetTitle(title)
        for histogram in self.histograms:
            histogram.SetTitle(title)

    @property
    def xlabel(self):
        return self._xlabel

    @xlabel.setter
    def xlabel(self, xlabel):
        self._xlabel = xlabel
        for histogram in self.histograms:
            histogram.SetXTitle(xlabel)

    @property
    def ylabel(self):
        return self._ylabel

    @ylabel.setter
    def ylabel(self, ylabel):
        self._ylabel = ylabel
        for histogram in self.histograms:
            histogram.SetYTitle(ylabel)

    @property
    def contact(self):
        return self._contact

    @contact.setter
    def contact(self, contact):
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
        return self._description

    @description.setter
    def description(self, description):
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
        return self._check

    @check.setter
    def check(self, check):
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
    def is_binary(xs):
        is_boolean = all(isinstance(x, bool) for x in xs)
        is_one_or_zero = all(x == 0 or x == 1 or not np.isfinite(x)
                             for x in xs)

        return is_boolean or is_one_or_zero

    @staticmethod
    def is_discrete(xs, max_n_unique=default_max_n_unique_for_discrete):
        # FIXME: improve test for discrete variable
        unique_xs = np.unique(xs)
        if len(unique_xs) < max_n_unique:
            return True

    @staticmethod
    def get_exceptional_values(xs):
        return rice_exceptional_values(xs)

    @staticmethod
    def get_robust_mean_and_std(xs):
        x_mean = truncated_mean(xs)
        x_std = trimmed_std(xs)
        return x_mean, x_std

    @staticmethod
    def format_bin_label(value):
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
                  allow_discrete=False):

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

        histograms = []

        if stackby is None:
            self.fill_into(histogram, xs, ys, weights=weights)

            histograms.append(histogram)
            plot = histogram

        else:
            # No TProfile allowed here
            if isinstance(histogram, ROOT.TProfile):
                raise ValueError("Cannot stack TProfile histograms.")

            stackby = np.array(stackby, copy=False)

            histograms = self.fill_into_grouped(histogram, xs, ys, weights=weights, groupby=stackby)

            plot = ROOT.THStack(name + "_stacked", '')

            # Add the histogram in reverse order such that the signal usually is on the bottom an well visible
            for histogram in reversed(histograms):
                plot.Add(histogram)

            # Set the reference histogram to be the lowest (usually the signal histogram)
            if histograms:
                plot.SetHistogram(histograms[-1])

        # Adjust the discrete bins after the filling to be equidistant
        if bin_labels:
            for histogram in histograms:
                x_taxis = histogram.GetXaxis()
                bin_edges = array.array("d", range(len(bin_labels) + 1))
                x_taxis.Set(n_bins, bin_edges)

        self.histograms = histograms
        self.plot = plot

        self.attach_attributes()

    def fill_into_grouped(self, histogram_template, xs, ys=None, weights=None, groupby=None):
        histograms = []
        unique_groupby = np.unique(groupby)
        name = histogram_template.GetName()

        for i_value, value in enumerate(unique_groupby):
            if np.isnan(value):
                indices_for_value = np.isnan(groupby)
            else:
                indices_for_value = groupby == value

            # Make a copy of the empty histogram
            histogram_for_value = histogram_template.Clone(name + '_' + str(value))
            i_root_color = i_value + 1

            histogram_for_value.SetLineColor(i_root_color)

            self.fill_into(histogram_for_value, xs, weights=weights, filter=indices_for_value)
            histograms.append(histogram_for_value)

        return histograms

    def fill_into(self, histogram, xs, ys=None, weights=None, filter=None):
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

    def add_nan_inf_stats(self, histogram, name, values):
        n_nans = np.isnan(values).sum()
        if n_nans > 0:
            self.add_stats_entry(histogram, name + ' nan', n_nans)

        n_positive_inf = np.sum(values == np.inf)
        if n_positive_inf > 0:
            self.add_stats_entry(histogram, name + ' +inf', n_positive_inf)

        n_negative_inf = np.sum(values == -np.inf)
        if n_negative_inf > 0:
            self.add_stats_entry(histogram, name + ' -inf', n_negative_inf)

    def add_stats_entry(self, histogram, label, value):
        additional_stats = self.additional_stats_by_name.setdefault(histogram.GetName(), {})
        additional_stats[label] = value

    def get_additional_stats(self, histogram):
        return self.additional_stats_by_name.get(histogram.GetName(), {})

    def determine_bin_edges(self,
                            xs,
                            bins=None,
                            lower_bound=None,
                            upper_bound=None,
                            outlier_z_score=None,
                            include_exceptionals=True,
                            allow_discrete=False):

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
        filling in the values that are not given.

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
            Preset lower bound of the binning range. The default, None, means the bound should be extracted from data.
        upper_bound : float or None, optional
            Preset upper bound of the binning range. The default, None, means the bound should be extracted from data.
        outlier_z_score : float or None, optional
            Threshold z-score of outlier detection. The default, None, means no outlier detection.
        include_exceptionals : bool, optional
            If the outlier detection is active this switch indicates, if  values detected as exceptional shall be included
            nevertheless into the binning range. Default is True, which means excpetional values as included even if they are
            detected as outliers.

        Returns
        -------
        n_bins, lower_bound, upper_bound : int, float, float
            A triple of found number of bins, lower bound and upper bound of the binning range.
        """

        debug = get_logger().debug

        finite_xs = xs[np.isfinite(xs)]

        # Prepare for the estimation of outliers
        if outlier_z_score is not None and (lower_bound is None
                                            or upper_bound is None):

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

                    # However we want to include at least the exceptional values in the range if there is any.
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

                    # However we want to include at least the exceptional values in the range if there is any.
                    upper_bound = np.nanmax([upper_bound, upper_exceptional_x])

                debug('Upper bound after outlier detection')
                debug('Upper bound %s', upper_bound)
                debug('Upper outlier bound %s', upper_outlier_bound)

        if n_bins is None:
            # Assume number of bins according to the rice rule.
            # The number of data points should not include outliers.
            n_data = np.sum((lower_bound <= finite_xs) & (finite_xs <= upper_bound))
            rice_n_bins = int(rice_n_bin(n_data))
            n_bins = rice_n_bins

        else:
            n_bins = int(n_bins)
            # Do not allow negative bin numbers
            if not n_bins > 0:
                message = 'Cannot accept n_bins=%s as number of bins, because it is not a number greater than 0.' % bins
                raise ValueError(message)

        return n_bins, lower_bound, upper_bound

    def set_fit_tf1(self, histogram, fit_tf1):
        additional_stats_tf1 = self.create_additional_stats_tf1(histogram)
        combined_tf1 = self.combine_fit_and_additional_stats(fit_tf1, additional_stats_tf1)
        self.set_tf1(histogram, combined_tf1)

    def set_tf1(self, histogram, tf1):
        # Delete any functions formally added
        self.delete_tf1(histogram)
        tf1.SetName("FitAndStats")
        histogram.GetListOfFunctions().Add(tf1)

    def delete_tf1(self, histogram):
        tf1 = histogram.FindObject("FitAndStats")
        if tf1:
            function_list = histogram.GetListOfFunctions()
            function_list.Remove(tf1)

    @staticmethod
    def copy_tf1_parameters(tf1_source, tf1_target, offset=0):
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

    def create_additional_stats_tf1(self, histogram):
        additional_stats = self.get_additional_stats(histogram)
        if not additional_stats:
            return None

        # Create dummy function, which displays additional statistics in the legend, when added to a histogram.
        # Dummy range to serve the functions
        lower_bound = 0
        upper_bound = 0

        # Create a formula which is zero in all cases but has space for n parameters
        # Formula string looks like 0*[0]+0*[1]+0*[2]+...
        formula_string = '+'.join('0*[' + str(i) + ']' for i in
                                  range(len(additional_stats)))

        # Compose a function that carries the addtional information
        additional_stats_tf1 = ROOT.TF1("Stats", formula_string, lower_bound, upper_bound)

        for (i, (label, value)) in enumerate(additional_stats.items()):
            additional_stats_tf1.SetParName(i, label)
            additional_stats_tf1.FixParameter(i, value)

        return additional_stats_tf1

    def combine_fit_and_additional_stats(self, fit_tf1, additional_stats_tf1):
        if additional_stats_tf1 is None:
            return fit_tf1

        # Combine both TF1 functions
        # Get the lower and upper bound of the fit
        # Use the pass-by reference containers from pyROOT to be able to call the function
        lower_bound = ROOT.Double()
        upper_bound = ROOT.Double()
        fit_tf1.GetRange(lower_bound, upper_bound)

        combined_formula = additional_stats_tf1.GetName() + '+' + fit_tf1.GetName()
        combined_tf1 = ROOT.TF1("Combined", combined_formula, lower_bound, upper_bound)

        # Transfer the fitted parameters
        chi2 = fit_tf1.GetChisquare()
        combined_tf1.SetChisquare(chi2)

        ndf = fit_tf1.GetNDF()
        combined_tf1.SetNDF(ndf)

        n_stats_parameters = additional_stats_tf1.GetNpar()
        n_fit_parameters = fit_tf1.GetNpar()
        self.copy_tf1_parameters(additional_stats_tf1, combined_tf1)
        self.copy_tf1_parameters(fit_tf1, combined_tf1, offset=n_stats_parameters)

        return combined_tf1

    def attach_attributes(self):
        # Forward the attributes to the plot by auto assignment
        self.check = self.check
        self.contact = self.contact
        self.description = self.description

        self.xlabel = self.xlabel
        self.ylabel = self.ylabel
        self.title = self.title

    @staticmethod
    def set_tstyle():
        belle2_validation_style_name = "belle2_validation_style"
        belle2_validation_tstyle = ROOT.gROOT.GetStyle(belle2_validation_style_name)
        if not belle2_validation_tstyle:
            belle2_validation_tstyle = ROOT.TStyle(belle2_validation_style_name, belle2_validation_style_name)

            opt_fit = 0112
            belle2_validation_tstyle.SetOptFit(opt_fit)

            opt_stat = 111111
            belle2_validation_tstyle.SetOptStat(opt_stat)
            ROOT.gROOT.SetStyle(belle2_validation_style_name)

        else:
            belle2_validation_tstyle.cd()


def test():
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
    validation_histogram.fit_gaus()
    validation_histogram.show()

    # Test stacked plotting
    # Make a random selection of 10%
    stackby = np.random.binomial(1.0, 0.10, 1000)
    stacked_validation_histogram = ValidationPlot('test_stacked_hist')
    stacked_validation_histogram.hist(normal_distributed_values, stackby=stackby)

    # Test a profile with a diagonal fit
    x = np.linspace(-1, 1, 1000)
    y = x.copy()
    x[0] = np.nan
    diagonal_plot = ValidationPlot('test_diag')
    diagonal_plot.profile(x, y, bins=50)
    diagonal_plot.fit_line()

    tfile = ROOT.TFile('test.root', 'RECREATE')

    validation_histogram.write(tfile)

    with root_cd("expert") as tdirectory1:
        diagonal_plot.write(tdirectory1)

    with root_cd("stacked") as tdirectory2:
        stacked_validation_histogram.write(tdirectory2)

    tfile.Close()

    tfile = ROOT.TFile('test.root')
    tBrowser = ROOT.TBrowser()
    tBrowser.BrowseObject(tfile)
    raw_input()
    tfile.Close()


if __name__ == '__main__':
    test()
