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

import numpy as np


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
                if self.cut_direction < 0:
                    binary_estimates = estimates >= self.cut
                else:
                    binary_estimates = estimates <= self.cut

            else:
                raise ValueError("cut is not a number")
        else:
            cut_value = None
            cut_direction = self.cut_direction

        # Figures of merit
        if cut_value is not None:
            fom_name = "{quantity_name}_classification_figures_of_merits".format(
                quantity_name=quantity_name
            )

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

            classification_fom['efficiency'] = efficiency
            classification_fom['purity'] = purity
            classification_fom['background_rejection'] = background_rejection

            self.fom = classification_fom

        # Stacked histogram
        plot_name = "{quantity_name}_signal_background_histogram".format(
            quantity_name=quantity_name
        )

        signal_background_histogram = ValidationPlot(plot_name)
        signal_background_histogram.hist(
            estimates,
            stackby=truths,
            lower_bound=self.lower_bound,
            upper_bound=self.upper_bound,
            outlier_z_score=self.outlier_z_score
        )
        signal_background_histogram.xlabel = axis_label

        self.plots['signal_background'] = signal_background_histogram

        # Purity profile
        plot_name = "{quantity_name}_purity_profile".format(
            quantity_name=quantity_name
        )

        purity_profile = ValidationPlot(plot_name)
        purity_profile.profile(
            estimates,
            truths,
            lower_bound=self.lower_bound,
            upper_bound=self.upper_bound,
            outlier_z_score=self.outlier_z_score
        )

        purity_profile.xlabel = axis_label
        purity_profile.ylabel = 'purity'

        self.plots["purity"] = purity_profile

        if not estimate_is_binary and cut_direction is not None:

            # Purity over efficiency #
            # ###################### #
            n_signal = scores.signal_amount(truths, estimates)

            sorting_indices = np.argsort(estimates)
            if cut_direction < 0:  # reject low
                # Keep a reference to keep the content alive
                org_sorting_indices = sorting_indices
                sorting_indices = sorting_indices[::-1]

            sorted_truths = truths[sorting_indices]
            sorted_selected_signal = np.cumsum(sorted_truths, dtype=float)
            sorted_efficiencies = sorted_selected_signal / n_signal

            plot_name = "{quantity_name}_purity_over_efficiency_profile".format(
                quantity_name=quantity_name
            )

            purity_over_efficiency_profile = ValidationPlot(plot_name)
            purity_over_efficiency_profile.profile(
                sorted_efficiencies, sorted_truths,
                cumulation_direction=1,
            )

            purity_over_efficiency_profile.xlabel = 'efficiency'
            purity_over_efficiency_profile.ylabel = 'purity'

            self.plots["purity_over_efficiency"] = purity_over_efficiency_profile

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
