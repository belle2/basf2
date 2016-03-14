#!/usr/bin/env python
# -*- coding: utf-8 -*-

from tracking.validation.plot import ValidationPlot, compose_axis_label
from tracking.validation.fom import ValidationFiguresOfMerit

import tracking.validation.scores as scores

# get error function as a np.ufunc vectorised for numpy array
from tracking.validation.utilities import erf, root_save_name
import tracking.validation.statistics as statistics

import math
import collections
import numbers
import copy

import numpy as np

from tracking.validation.tolerate_missing_key_formatter import TolerateMissingKeyFormatter


formatter = TolerateMissingKeyFormatter()


class ClassificationAnalysis(object):

    def __init__(
        self,
        contact,
        quantity_name,
        cut_direction=None,
        cut=None,
        lower_bound=None,
        upper_bound=None,
        outlier_z_score=None,
        allow_discrete=None,
        unit=None
    ):
        """Performs a comparision of an estimated quantity to their truths by generating standardized validation plots."""

        self._contact = contact
        self.quantity_name = quantity_name

        self.plots = {}
        self.fom = None

        self.cut_direction = cut_direction
        self.cut = cut

        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.outlier_z_score = outlier_z_score
        self.allow_discrete = allow_discrete
        self.unit = unit

    def analyse(
        self,
        estimates,
        truths
    ):
        """Compares the concrete estimate to the truth and efficiency, purity and background rejection
        as figure of merit and plots the selection as a stacked plot over the truths.

        Parameters
        ----------
        estimates : array_like
            Selection variable to compare to the truths
        truths : array_like
            Binary true class values.
        """

        quantity_name = self.quantity_name
        axis_label = compose_axis_label(quantity_name, self.unit)

        plot_name = "{quantity_name}_{subplot_name}"
        plot_name = formatter.format(plot_name, quantity_name=quantity_name)

        # Some different things become presentable depending on the estimates
        estimate_is_binary = statistics.is_binary_series(estimates)

        if estimate_is_binary:
            binary_estimates = estimates
            cut_value = 0.5
            cut_direction = -1  # reject low values

        elif self.cut is not None:
            if isinstance(self.cut, numbers.Number):
                cut_value = self.cut
                cut_direction = self.cut_direction
                cut_classifier = CutClassifier(cut_direction=cut_direction, cut_value=cut_value)

            else:
                cut_classifier = self.cut
                cut_classifier = cut_classifier.clone()

            cut_classifier.fit(estimates, truths)
            binary_estimates = cut_classifier.predict(estimates)
            cut_direction = cut_classifier.cut_direction
            cut_value = cut_classifier.cut_value

            if not isinstance(self.cut, numbers.Number):
                print formatter.format(plot_name, subplot_name="cut_classifier"), "summary"
                cut_classifier.describe(estimates, truths)

        else:
            cut_value = None
            cut_direction = self.cut_direction

        # Figures of merit
        if cut_value is not None:
            fom_name = formatter.format(plot_name, subplot_name="classification_figures_of_merits")
            fom_description = "Efficiency, purity and background rejection of the classifiction with {quantity_name}".format(
                quantity_name=quantity_name
            )

            fom_check = "Check that the classifcation quality stays stable."

            fom_title = "Summary of the classification quality with {quantity_name}".format(
                quantity_name=quantity_name
            )

            classification_fom = ValidationFiguresOfMerit(
                name=fom_name,
                title=fom_title,
                description=fom_description,
                check=fom_check,
                contact=self.contact,
            )

            efficiency = scores.efficiency(truths, binary_estimates)
            purity = scores.purity(truths, binary_estimates)
            background_rejection = scores.background_rejection(truths, binary_estimates)

            classification_fom['cut_value'] = cut_value
            classification_fom['cut_direction'] = cut_direction
            classification_fom['efficiency'] = efficiency
            classification_fom['purity'] = purity
            classification_fom['background_rejection'] = background_rejection

            self.fom = classification_fom

        # Stacked histogram
        signal_bkg_histogram_name = formatter.format(plot_name, subplot_name="signal_bkg_histogram")
        signal_bkg_histogram = ValidationPlot(signal_bkg_histogram_name)
        signal_bkg_histogram.hist(
            estimates,
            stackby=truths,
            lower_bound=self.lower_bound,
            upper_bound=self.upper_bound,
            outlier_z_score=self.outlier_z_score,
            allow_discrete=self.allow_discrete,
        )
        signal_bkg_histogram.xlabel = axis_label

        self.plots['signal_bkg'] = signal_bkg_histogram

        # Purity profile
        purity_profile_name = formatter.format(plot_name, subplot_name="purity_profile")

        purity_profile = ValidationPlot(purity_profile_name)
        purity_profile.profile(
            estimates,
            truths,
            lower_bound=self.lower_bound,
            upper_bound=self.upper_bound,
            outlier_z_score=self.outlier_z_score,
            allow_discrete=self.allow_discrete,
        )

        purity_profile.xlabel = axis_label
        purity_profile.ylabel = 'purity'

        self.plots["purity"] = purity_profile

        if not estimate_is_binary and cut_direction is not None:
            n_data = len(estimates)
            n_signals = scores.signal_amount(truths, estimates)
            n_bkgs = n_data - n_signals

            sorting_indices = np.argsort(estimates)
            if cut_direction < 0:  # reject low
                # Keep a reference to keep the content alive
                orginal_sorting_indices = sorting_indices
                sorting_indices = sorting_indices[::-1]

            sorted_truths = truths[sorting_indices]
            sorted_estimates = estimates[sorting_indices]

            sorted_n_accepted_signals = np.cumsum(sorted_truths, dtype=float)
            sorted_efficiencies = sorted_n_accepted_signals / n_signals

            sorted_n_rejected_signals = n_signals - sorted_n_accepted_signals
            sorted_n_rejects = np.arange(len(estimates) + 1, 1, -1)
            sorted_n_rejected_bkgs = sorted_n_rejects - sorted_n_rejected_signals
            sorted_bkg_rejections = sorted_n_rejected_bkgs / n_bkgs

            # Efficiency by cut value #
            # ####################### #
            efficiency_by_cut_profile_name = formatter.format(plot_name, subplot_name="efficiency_by_cut")

            efficiency_by_cut_profile = ValidationPlot(efficiency_by_cut_profile_name)
            efficiency_by_cut_profile.profile(
                sorted_estimates,
                sorted_efficiencies,
                lower_bound=self.lower_bound,
                upper_bound=self.upper_bound,
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )

            efficiency_by_cut_profile.xlabel = "cut " + axis_label
            efficiency_by_cut_profile.ylabel = "efficiency"

            self.plots["efficiency_by_cut"] = efficiency_by_cut_profile

            # Background rejection over cut value #
            # ################################### #
            bkg_rejection_by_cut_profile_name = formatter.format(plot_name, subplot_name="bkg_rejection_by_cut")
            bkg_rejection_by_cut_profile = ValidationPlot(bkg_rejection_by_cut_profile_name)
            bkg_rejection_by_cut_profile.profile(
                sorted_estimates,
                sorted_bkg_rejections,
                lower_bound=self.lower_bound,
                upper_bound=self.upper_bound,
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )

            bkg_rejection_by_cut_profile.xlabel = "cut " + axis_label
            bkg_rejection_by_cut_profile.ylabel = "background rejection"

            self.plots["bkg_rejection_by_cut"] = bkg_rejection_by_cut_profile

            # Purity over efficiency #
            # ###################### #
            purity_over_efficiency_profile_name = formatter.format(plot_name, subplot_name="purity_over_efficiency")
            purity_over_efficiency_profile = ValidationPlot(purity_over_efficiency_profile_name)
            purity_over_efficiency_profile.profile(
                sorted_efficiencies,
                sorted_truths,
                cumulation_direction=1,
                lower_bound=0,
                upper_bound=1
            )
            purity_over_efficiency_profile.xlabel = 'efficiency'
            purity_over_efficiency_profile.ylabel = 'purity'

            self.plots["purity_over_efficiency"] = purity_over_efficiency_profile

            # Efficiency over background rejection #
            # #################################### #
            efficiency_over_bkg_rejection_profile_name = formatter.format(plot_name, subplot_name="efficiency_over_bkg_rejection")
            efficiency_over_bkg_rejection_profile = ValidationPlot(efficiency_over_bkg_rejection_profile_name)
            efficiency_over_bkg_rejection_profile.profile(
                sorted_bkg_rejections,
                sorted_efficiencies,
                lower_bound=0,
                upper_bound=1
            )

            efficiency_over_bkg_rejection_profile.xlabel = "bkg rejection"
            efficiency_over_bkg_rejection_profile.ylabel = "efficiency"

            self.plots["efficiency_over_bkg_rejection"] = efficiency_over_bkg_rejection_profile

        self.contact = self.contact

    @property
    def contact(self):
        return self._contact

    @contact.setter
    def contact(self, contact):
        self._contact = contact

        for plot in self.plots.values():
            plot.contact = contact

        if self.fom:
            self.fom.contact = contact

    def write(self, tdirectory=None):
        for plot in self.plots.values():
            plot.write(tdirectory)

        if self.fom:
            self.fom.write(tdirectory)


class CutClassifier(object):

    """Simple classifier cutting on a single variable"""

    def __init__(self, cut_direction=1, cut_value=np.nan):
        self.cut_direction_ = cut_direction
        self.cut_value_ = cut_value

    @property
    def cut_direction(self):
        return self.cut_direction_

    @property
    def cut_value(self):
        return self.cut_value_

    def clone(self):
        return copy.copy(self)

    def determine_cut_value(self, estimates, truths):
        return self.cut_value_  # do not change cut value from constructed one

    def fit(self, estimates, truths):
        self.cut_value_ = self.determine_cut_value(estimates, truths)
        return self

    def predict(self, estimates):
        if self.cut_value_ is None:
            raise ValueError("Cut value not set. Forgot to fit?")

        if self.cut_direction_ < 0:
            binary_estimates = estimates >= self.cut_value_
        else:
            binary_estimates = estimates <= self.cut_value_

        return binary_estimates

    def describe(self, estimates, truths):
        if self.cut_direction_ < 0:
            print "Cut accepts >= ", self.cut_value_, 'with'
        else:
            print "Cut accepts <= ", self.cut_value_, 'with'

        binary_estimates = self.predict(estimates)

        efficiency = scores.efficiency(truths, binary_estimates)
        purity = scores.purity(truths, binary_estimates)
        background_rejection = scores.background_rejection(truths, binary_estimates)

        print "efficiency", efficiency
        print "purity", purity
        print "background_rejection", background_rejection


def cut_at_background_rejection(background_rejection=0.5, cut_direction=1):
    return CutAtBackgroundRejectionClassifier(background_rejection, cut_direction)


class CutAtBackgroundRejectionClassifier(CutClassifier):

    def __init__(self, background_rejection=0.5, cut_direction=1):
        super(CutAtBackgroundRejectionClassifier, self).__init__(cut_direction=cut_direction, cut_value=np.nan)
        self.background_rejection = background_rejection

    def determine_cut_value(self, estimates, truths):
        n_data = len(estimates)
        n_signals = scores.signal_amount(truths, estimates)
        n_bkgs = n_data - n_signals

        sorting_indices = np.argsort(estimates)
        if self.cut_direction_ < 0:  # reject low
            # Keep a reference to keep the content alive
            orginal_sorting_indices = sorting_indices
            sorting_indices = sorting_indices[::-1]

        sorted_truths = truths[sorting_indices]
        sorted_estimates = estimates[sorting_indices]

        sorted_n_accepted_signals = np.cumsum(sorted_truths, dtype=float)
        sorted_efficiencies = sorted_n_accepted_signals / n_signals

        sorted_n_rejected_signals = n_signals - sorted_n_accepted_signals
        sorted_n_rejects = np.arange(len(estimates) + 1, 1, -1)
        sorted_n_rejected_bkgs = sorted_n_rejects - sorted_n_rejected_signals
        sorted_bkg_rejections = sorted_n_rejected_bkgs / n_bkgs

        cut_index, = np.searchsorted(sorted_bkg_rejections[::-1], (self.background_rejection,), side='right')

        cut_value = sorted_estimates[-cut_index - 1]
        return cut_value
