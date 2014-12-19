#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import itertools
import collections
import array
import numpy as np

import ROOT

import logging


def get_logger():
    return logging.getLogger(__name__)


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
    'tan #lambda': None,
    }


def compose_axis_label(quantity_name, unit=None):
    if unit is None:
        unit = units_by_quantity_name.get(quantity_name, None)

    if unit is None:
        axis_label = quantity_name
    else:
        axis_label = '%s (%s)' % (quantity_name, unit)

    return axis_label


def root_save_name(name):
    """Strips all meta characters that might be unsafe to use as a ROOT name"""

    deletechars = r"/$\#{}"
    name = name.replace(' ', '_').replace('-', '_').translate(None,
            deletechars)
    return name


class ValidationPlot(object):

    def __init__(self, name):

        self.name = name

        self._description = ''
        self._check = ''
        self._contact = ''

        self._xlabel = ''
        self._ylabel = ''
        self._title = ''

        self.additional_stats = collections.OrderedDict()

        self.tf1_additional_stats = None
        self.tf1_fit = None
        self.tf1_combined = None

        self.histogram = None

    @staticmethod
    def is_binary(xs):
        is_boolean = all(isinstance(x, bool) for x in xs)
        is_one_or_zero = all(x == 0 or x == 1 or not np.isfinite(x) for x in
                             xs)
        return is_boolean or is_one_or_zero

    @classmethod
    def determine_bin_edges(
        cls,
        xs,
        bins=None,
        lower_bound=None,
        upper_bound=None,
        outlier_z_score=None,
        ):

        # Coerce values to a numpy array. Do not copy if already a numpy array.
        xs = np.array(xs, copy=False)

        if isinstance(bins, collections.Iterable):
            # Bins is considered as an array
            # Construct a float array forwardable to root.
            bin_edges = bins
            bin_edges = array.array('f', bin_edges)
            return bin_edges

        if cls.is_binary(xs):
            # if xs is a binary variable setup the histogram to
            # have only two bins with the right boundaries.
            if lower_bound is None:
                lower_bound = -0.5

            if upper_bound is None:
                upper_bound = 1.5

            if bins is None:
                n_bins = 2
            else:
                n_bins = int(bins) or 1
        else:

            finite_indices = np.isfinite(xs)

            if outlier_z_score is not None and (lower_bound is None
                    or upper_bound is None):
                # Prepare for the estimation of outliers
                x_mean = truncated_mean(xs)
                x_std = trimmed_std(xs)

            if lower_bound is None:
                lower_bound = np.min(xs[finite_indices])

                # Clip the lower bound by outliers that exceed the given z score
                if outlier_z_score is not None:
                    outlier_lower_bound = x_mean - outlier_z_score * x_std
                    if outlier_lower_bound > lower_bound:
                        lower_bound = outlier_lower_bound

            if upper_bound is None:
                upper_bound = np.max(xs[finite_indices])
                # Correct the upper bound such that all values are strictly smaller than the upper bound
                upper_bound = np.nextafter(upper_bound, np.inf)

                # Clip the upper bound by outliers that exceed the given z score
                if outlier_z_score is not None:
                    outlier_upper_bound = x_mean + outlier_z_score * x_std
                    if outlier_upper_bound < upper_bound:
                        upper_bound = outlier_upper_bound

            if bins is None:
                # Assume number of bins according to the rice rule.
                # The number of data points should not include outliers.
                n_data = np.sum((lower_bound <= xs) & (xs <= upper_bound))
                rice_n_bins = int(np.ceil(2.0 * pow(n_data, 1.0 / 3.0)))
                n_bins = rice_n_bins
            else:

                n_bins = int(bins)
                # Do not allow negative bin numbers
                if not n_bins > 0:
                    message = 'bins=%s which is not a number greater than 0.' \
                        % bins
                    raise ValueError(message)

        n_bin_edges = n_bins + 1
        if lower_bound != upper_bound:
            bin_edges = np.linspace(lower_bound, upper_bound, n_bin_edges)

            # Reinforce the upper and lower bound to be exact
            bin_edges[0] = lower_bound
            bin_edges[-1] = upper_bound
        else:

            # Fall back if the array contains only one value
            bin_edges = [lower_bound, upper_bound + 1]

        # Construct a float array forwardable to root.
        bin_edges = array.array('f', bin_edges)
        return bin_edges

    def fill(
        self,
        xs,
        ys=None,
        weights=None,
        bins=None,
        ):
        """Legacy use hist or profile instead."""

        if ys is None:
            return self.hist(xs, weights=weights, bins=bins)
        else:
            return self.profile(xs, ys, weights=weights, bins=bins)

    def hist(
        self,
        xs,
        weights=None,
        bins=None,
        lower_bound=None,
        upper_bound=None,
        outlier_z_score=None,
        ):

        name = self.name

        # Coerce values to a numpy array. Do not copy if already a numpy array.
        xs = np.array(xs, copy=False)
        if weights is not None:
            weights = np.array(weights, copy=False)

        bin_edges = self.determine_bin_edges(xs, bins=bins,
                lower_bound=lower_bound, upper_bound=upper_bound,
                outlier_z_score=outlier_z_score)

        n_bins = len(bin_edges) - 1
        histogram = ROOT.TH1F(name, '', n_bins, bin_edges)
        self.histogram = histogram

        finite_indices = ~np.isnan(xs)
        finite_selections = finite_indices
        if weights is None:
            weights = itertools.repeat(1.0)

        Fill = histogram.Fill
        for (x, weight, select) in zip(xs, weights, finite_selections):
            if select:
                Fill(float(x), float(weight))

        # Count the nan and inf values in x
        self.add_nan_inf_stats('x', xs)

        if self.ylabel is None:
            self.ylabel = 'Count'

        self.attach_attributes()
        return self

    def profile(
        self,
        xs,
        ys,
        weights=None,
        bins=None,
        lower_bound=None,
        upper_bound=None,
        outlier_z_score=None,
        ):

        name = self.name

        # Coerce values to a numpy array. Do not copy if already a numpy array.
        xs = np.array(xs, copy=False)
        ys = np.array(ys, copy=False)
        if weights is not None:
            weights = np.array(weights, copy=False)

        bin_edges = self.determine_bin_edges(xs, bins=bins,
                lower_bound=lower_bound, upper_bound=upper_bound,
                outlier_z_score=outlier_z_score)

        # Use determine bins to find the lower and upper bound with correct handling of nan and inf
        (y_lower_bound, y_upper_bound) = self.determine_bin_edges(ys, bins=1)

        n_bins = len(bin_edges) - 1
        histogram = ROOT.TProfile(name, '', n_bins, bin_edges)
        self.histogram = histogram

        finite_indices = ~np.isnan(xs) & np.isfinite(ys)
        finite_selections = finite_indices
        if weights is None:
            weights = itertools.repeat(1.0)

        Fill = histogram.Fill
        for (x, y, weight, select) in zip(xs, ys, weights, finite_selections):
            if select:
                Fill(float(x), float(y), float(weight))

        # Expand the range for better view of the plot
        y_range = y_upper_bound - y_lower_bound
        histogram.SetMaximum(y_upper_bound + 0.1 * y_range)
        histogram.SetMinimum(y_lower_bound)

        # Count the nan and inf values in x
        self.add_nan_inf_stats('x', xs)

        # Count the nan and inf values in y
        self.add_nan_inf_stats('y', ys)

        if self.is_binary(ys):
            self.ylabel = 'Probability'

        self.attach_attributes()
        return self

    def scatter(
        self,
        xs,
        ys,
        weights=None,
        bins=(None, None),
        lower_bound=(None, None),
        upper_bound=(None, None),
        outlier_z_score=(None, None),
        ):

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

        x_bin_edges = self.determine_bin_edges(xs, bins=x_bins,
                lower_bound=x_lower_bound, upper_bound=x_upper_bound,
                outlier_z_score=x_outlier_z_score)

        y_bin_edges = self.determine_bin_edges(ys, bins=y_bins,
                lower_bound=y_lower_bound, upper_bound=y_upper_bound,
                outlier_z_score=y_outlier_z_score)

        n_x_bins = len(x_bin_edges) - 1
        n_y_bins = len(y_bin_edges) - 1
        histogram = ROOT.TH2F(
            name,
            '',
            n_x_bins,
            x_bin_edges,
            n_y_bins,
            y_bin_edges,
            )
        self.histogram = histogram

        finite_indices = ~np.isnan(xs) & ~np.isnan(ys)
        finite_selections = finite_indices
        if weights is None:
            weights = itertools.repeat(1.0)

        Fill = histogram.Fill
        for (x, y, weight, select) in zip(xs, ys, weights, finite_selections):
            if select:
                Fill(float(x), float(y), float(weight))

        # Count the nan and inf values in x
        self.add_nan_inf_stats('x', xs)

        # Count the nan and inf values in y
        self.add_nan_inf_stats('y', ys)

        # Add description, check, contact and titles to the histogram
        self.attach_attributes()

        return self

    def add_nan_inf_stats(self, name, values):
        n_nans = np.isnan(values).sum()
        if n_nans > 0:
            self.add_stats_entry(name + ' nan', n_nans)

        n_positive_inf = np.sum(values == np.inf)
        if n_positive_inf > 0:
            self.add_stats_entry(name + ' +inf', n_positive_inf)

        n_negative_inf = np.sum(values == -np.inf)
        if n_negative_inf > 0:
            self.add_stats_entry(name + ' -inf', n_negative_inf)

    def attach_attributes(self):
        histogram = self.histogram

        # Now attach the additional information
        root_description = ROOT.TNamed('Description', self.description)
        root_check = ROOT.TNamed('Check', self.check)
        root_contact = ROOT.TNamed('Contact', self.contact)

        histogram.GetListOfFunctions().Add(root_description)
        histogram.GetListOfFunctions().Add(root_check)
        histogram.GetListOfFunctions().Add(root_contact)

        title = (self.title if self.title else self.name)
        histogram.SetTitle(title)

        xlabel = self.xlabel
        histogram.SetXTitle(xlabel)

        ylabel = self.ylabel
        histogram.SetYTitle(ylabel)

    @property
    def title(self):
        return self._title

    @title.setter
    def title(self, title):
        self._title = title
        if self.histogram is not None:
            self.histogram.SetTitle(title)

    @property
    def xlabel(self):
        return self._xlabel

    @xlabel.setter
    def xlabel(self, xlabel):
        self._xlabel = xlabel
        if self.histogram is not None:
            self.histogram.SetXTitle(xlabel)

    @property
    def ylabel(self):
        return self._ylabel

    @ylabel.setter
    def ylabel(self, ylabel):
        self._ylabel = ylabel
        if self.histogram is not None:
            self.histogram.SetYTitle(ylabel)

    @property
    def contact(self):
        return self._contact

    @contact.setter
    def contact(self, contact):
        self._contact = contact
        if self.histogram is not None:
            found_obj = self.histogram.FindObject('Contact')
            if found_obj:
                found_obj.SetTitle(contact)
            else:
                raise KeyError("Could not find 'Contact' in histogram")

    @property
    def description(self):
        return self._description

    @description.setter
    def description(self, description):
        self._description = description
        if self.histogram is not None:
            found_obj = self.histogram.FindObject('Description')
            if found_obj:
                found_obj.SetTitle(description)
            else:
                raise KeyError("Could not find 'Description' in histogram")

    @property
    def check(self):
        return self._check

    @check.setter
    def check(self, check):
        self._check = check
        if self.histogram is not None:
            found_obj = self.histogram.FindObject('Check')
            if found_obj:
                found_obj.SetTitle(check)
            else:
                raise KeyError("Could not find 'Check' in histogram")

    def show(self):
        self.histogram.Draw()

    def write(self, tDirectory=None):
        if tDirectory is None:
            # Write histogram to current directory
            self.histogram.Write()
        else:
            # Temporarily switch the current directory
            save_directory = ROOT.gDirectory
            try:
                tDirectory.cd()
                self.histogram.Write()
            finally:
                save_directory.cd()

    @property
    def tf1_fit_name(self):
        fit_name = self.name + '_fit'
        return fit_name

    @property
    def tf1_additional_stats_name(self):
        additional_stats_name = self.name + '_additonal_stats'
        return additional_stats_name

    @property
    def tf1_combined_name(self):
        combined_name = self.name + '_fit_and_additonal_stats'
        return combined_name

    def add_stats_entry(self, label, value):
        self.additional_stats[label] = value
        self.create_tf1_additional_stats()

    def create_tf1_additional_stats(self):
        # Add dummy function to the histogram to display additional information in the legend
        # Dummy range to serve the functions
        lower_bound = 0
        upper_bound = 0

        # Create a formula which is zero in all cases but has space for n parameters
        # Formula string looks like 0*[0]+0*[1]+0*[2]+...
        formula_string = '+'.join('0*[' + str(i) + ']' for i in
                                  range(len(self.additional_stats)))

        # Compose a function that carries the addtional information
        tf1_additional_stats = ROOT.TF1(self.tf1_additional_stats_name,
                formula_string, lower_bound, upper_bound)

        for (i, (label, value)) in enumerate(self.additional_stats.items()):
            tf1_additional_stats.SetParName(i, label)
            tf1_additional_stats.FixParameter(i, value)

        self.tf1_additional_stats = tf1_additional_stats

        self.combine_fit_and_additional_stats()

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

    def combine_fit_and_additional_stats(self):
        tf1_fit = self.tf1_fit
        tf1_additional_stats = self.tf1_additional_stats

        if tf1_fit is not None and tf1_additional_stats is not None:
            # Combine both TF1 functions

            # Get the lower and upper bound of the fit
            # Use the pass-by reference containers from pyROOT to be able to call the function
            lower_bound = ROOT.Double()
            upper_bound = ROOT.Double()
            tf1_fit.GetRange(lower_bound, upper_bound)

            combined_formula = self.tf1_additional_stats_name + '+' \
                + self.tf1_fit_name
            tf1_combined = ROOT.TF1(self.tf1_combined_name, combined_formula,
                                    lower_bound, upper_bound)

            # Transfer the fitted parameters
            chi2 = tf1_fit.GetChisquare()
            tf1_combined.SetChisquare(chi2)

            ndf = tf1_fit.GetNDF()
            tf1_combined.SetNDF(ndf)

            n_stats_parameters = tf1_additional_stats.GetNpar()
            n_fit_parameters = tf1_fit.GetNpar()
            self.copy_tf1_parameters(tf1_additional_stats, tf1_combined)
            self.copy_tf1_parameters(tf1_fit, tf1_combined,
                                     offset=n_stats_parameters)
        elif tf1_fit is None and tf1_additional_stats is None:

            # Nothing to combine
            tf1_combined = None
        elif tf1_fit is None or tf1_additional_stats is None:

            # Make a copy of the one that is not None
            tf1_combined = ROOT.TF1(tf1_fit or tf1_additional_stats)
            tf1_combined.SetName(self.tf1_combined_name)

        self.tf1_combined = tf1_combined

        if self.histogram is not None:
            # Remove old tf1 and insert new
            function_list = self.histogram.GetListOfFunctions()
            old_tf1_combined = \
                self.histogram.FindObject(self.tf1_combined_name)

            if old_tf1_combined is not None:
                function_list.Remove(old_tf1_combined)

            # Add the new histogram if there is one
            if tf1_combined is not None:
                function_list.Add(tf1_combined)

    def fit(self, formula, options):
        histogram = self.histogram
        if histogram is None:
            raise RuntimeError('Validation plot must be filled before it can be fitted.'
                               )

        if isinstance(histogram, ROOT.TH2F):
            raise RuntimeError('Fitting is currently implemented / tested for one dimensional validation plots.'
                               )

        xaxis = histogram.GetXaxis()
        n_bins = xaxis.GetNbins()
        lower_bound = xaxis.GetBinLowEdge(1)
        upper_bound = xaxis.GetBinUpEdge(n_bins)

        tf1_fit = ROOT.TF1(self.tf1_fit_name, formula, lower_bound,
                           upper_bound)
        get_logger().info('Fitting with %s', tf1_fit.GetExpFormula())

        # Make sure the fitted function is not automatically added since we want to do that one our own.
        # Look for the documentation of TH1::Fit() for details of the options.
        if 'N' not in options:
            options += 'N'

        histogram.Fit(tf1_fit, options)

        self.tf1_fit = tf1_fit
        self.combine_fit_and_additional_stats()

    def fit_gaus(self):
        self.fit('gaus', 'LM')

    def fit_line(self):
        self.fit('x++1', 'M')

    def fit_const(self):
        self.fit('[0]', 'M')

    def fit_diag(self):
        self.fit('[0]*x', 'M')


def test():
    opt_fit = 0112
    ROOT.gStyle.SetOptFit(opt_fit)

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

    # Test a profile with a diagonal fit
    x = np.linspace(-1, 1, 1000)
    x[0] = np.nan
    diagonal_plot = ValidationPlot('test_diag')
    diagonal_plot.profile(x, x, bins=50)
    diagonal_plot.fit_line()

    tFile = ROOT.TFile('test.root', 'RECREATE')
    tDirectory = tFile.mkdir('expert', 'Expert')
    tDirectory.cd()
    ROOT.gStyle.SetOptFit(opt_fit)
    tFile.cd()
    validation_histogram.write(tFile)
    diagonal_plot.write(tDirectory)

    tBrowser = ROOT.TBrowser()
    tBrowser.BrowseObject(diagonal_plot.histogram)
    raw_input()
    tFile.Close()


if __name__ == '__main__':
    test()

