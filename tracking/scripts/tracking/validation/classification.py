#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import collections
import numbers
import copy

from . import scores
from . import statistics

from .plot import ValidationPlot, compose_axis_label
from .fom import ValidationFiguresOfMerit
from .tolerate_missing_key_formatter import TolerateMissingKeyFormatter

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

        self.plots = collections.OrderedDict()
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
        truths,
        auxiliaries={}
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

        signals = truths != 0

        # Some different things become presentable depending on the estimates
        estimate_is_binary = statistics.is_binary_series(estimates)

        if estimate_is_binary:
            binary_estimates = estimates != 0
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
            binary_estimates = cut_classifier.predict(estimates) != 0
            cut_direction = cut_classifier.cut_direction
            cut_value = cut_classifier.cut_value

            if not isinstance(self.cut, numbers.Number):
                print(formatter.format(plot_name, subplot_name="cut_classifier"), "summary")
                cut_classifier.describe(estimates, truths)

        else:
            cut_value = None
            cut_direction = self.cut_direction

        lower_bound = self.lower_bound
        upper_bound = self.upper_bound

        # Stacked histogram
        signal_bkg_histogram_name = formatter.format(plot_name, subplot_name="signal_bkg_histogram")
        signal_bkg_histogram = ValidationPlot(signal_bkg_histogram_name)
        signal_bkg_histogram.hist(
            estimates,
            stackby=truths,
            lower_bound=lower_bound,
            upper_bound=upper_bound,
            outlier_z_score=self.outlier_z_score,
            allow_discrete=self.allow_discrete,
        )
        signal_bkg_histogram.xlabel = axis_label

        if lower_bound is None:
            lower_bound = signal_bkg_histogram.lower_bound

        if upper_bound is None:
            upper_bound = signal_bkg_histogram.upper_bound

        self.plots['signal_bkg'] = signal_bkg_histogram

        # Purity profile
        purity_profile_name = formatter.format(plot_name, subplot_name="purity_profile")

        purity_profile = ValidationPlot(purity_profile_name)
        purity_profile.profile(
            estimates,
            truths,
            lower_bound=lower_bound,
            upper_bound=upper_bound,
            outlier_z_score=self.outlier_z_score,
            allow_discrete=self.allow_discrete,
        )

        purity_profile.xlabel = axis_label
        purity_profile.ylabel = 'purity'
        self.plots["purity"] = purity_profile

        # Try to guess the cur direction form the correlation
        if cut_direction is None:
            purity_grapherrors = ValidationPlot.convert_tprofile_to_tgrapherrors(purity_profile.plot)
            correlation = purity_grapherrors.GetCorrelationFactor()
            if correlation > 0.1:
                print("Determined cut direction", -1)
                cut_direction = -1  # reject low values
            elif correlation < -0.1:
                print("Determined cut direction", 1)
                cut_direction = +1  # reject high values

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
        # Auxiliary hists
        for aux_name, aux_values in auxiliaries.items():
            if statistics.is_single_value_series(aux_values) or aux_name == quantity_name:
                continue

            aux_axis_label = compose_axis_label(aux_name)

            # Signal + bkg distribution over estimate and auxiliary variable #
            # ############################################################## #
            signal_bkg_aux_hist2d_name = formatter.format(plot_name, subplot_name=aux_name + '_signal_bkg_aux2d')
            signal_bkg_aux_hist2d = ValidationPlot(signal_bkg_aux_hist2d_name)
            signal_bkg_aux_hist2d.hist2d(
                aux_values,
                estimates,
                stackby=truths,
                lower_bound=(None, lower_bound),
                upper_bound=(None, upper_bound),
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )

            aux_lower_bound = signal_bkg_aux_hist2d.lower_bound[0]
            aux_upper_bound = signal_bkg_aux_hist2d.upper_bound[0]

            signal_bkg_aux_hist2d.xlabel = aux_axis_label
            signal_bkg_aux_hist2d.ylabel = axis_label
            self.plots[signal_bkg_aux_hist2d_name] = signal_bkg_aux_hist2d

            # Figures of merit as function of the auxiliary variables
            if cut_value is not None:

                # Auxiliary purity profile #
                # ######################## #
                aux_purity_profile_name = formatter.format(plot_name, subplot_name=aux_name + "_aux_purity_profile")
                aux_purity_profile = ValidationPlot(aux_purity_profile_name)
                aux_purity_profile.profile(
                    aux_values[binary_estimates],
                    truths[binary_estimates],
                    outlier_z_score=self.outlier_z_score,
                    allow_discrete=self.allow_discrete,
                    lower_bound=aux_lower_bound,
                    upper_bound=aux_upper_bound,
                )

                aux_purity_profile.xlabel = aux_axis_label
                aux_purity_profile.ylabel = 'purity'
                self.plots[aux_purity_profile_name] = aux_purity_profile

                # Auxiliary efficiency profile #
                # ############################ #
                aux_efficiency_profile_name = formatter.format(plot_name, subplot_name=aux_name + "_aux_efficiency_profile")
                aux_efficiency_profile = ValidationPlot(aux_efficiency_profile_name)
                aux_efficiency_profile.profile(
                    aux_values[signals],
                    binary_estimates[signals],
                    outlier_z_score=self.outlier_z_score,
                    allow_discrete=self.allow_discrete,
                    lower_bound=aux_lower_bound,
                    upper_bound=aux_upper_bound,
                )

                aux_efficiency_profile.xlabel = aux_axis_label
                aux_efficiency_profile.ylabel = 'efficiency'
                self.plots[aux_efficiency_profile_name] = aux_efficiency_profile

                # Auxiliary bkg rejection profile #
                # ############################### #
                aux_bkg_rejection_profile_name = formatter.format(plot_name, subplot_name=aux_name + "_aux_bkg_rejection_profile")
                aux_bkg_rejection_profile = ValidationPlot(aux_bkg_rejection_profile_name)
                aux_bkg_rejection_profile.profile(
                    aux_values[~signals],
                    ~binary_estimates[~signals],
                    outlier_z_score=self.outlier_z_score,
                    allow_discrete=self.allow_discrete,
                    lower_bound=aux_lower_bound,
                    upper_bound=aux_upper_bound,
                )

                aux_bkg_rejection_profile.xlabel = aux_axis_label
                aux_bkg_rejection_profile.ylabel = 'bkg rejection'
                self.plots[aux_bkg_rejection_profile_name] = aux_bkg_rejection_profile

        cut_abs = False
        if cut_direction is None:
            purity_grapherrors = ValidationPlot.convert_tprofile_to_tgrapherrors(purity_profile.plot,
                                                                                 abs_x=True)
            correlation = purity_grapherrors.GetCorrelationFactor()
            if correlation > 0.1:
                print("Determined absolute cut direction", -1)
                cut_direction = -1  # reject low values
                cut_abs = True
            elif correlation < -0.1:
                print("Determined absolute cut direction", 1)
                cut_direction = +1  # reject high values
                cut_abs = True

        if cut_abs:
            estimates = np.abs(estimates)
            cut_x_label = "cut " + compose_axis_label("abs(" + quantity_name + ")", self.unit)
            lower_bound = 0
        else:
            cut_x_label = "cut " + axis_label

        # Quantile plots
        if not estimate_is_binary and cut_direction is not None:
            # Signal estimate quantiles over auxiliary variable #
            # ################################################# #
            if cut_direction > 0:
                quantiles = [0.5, 0.90, 0.99]
            else:
                quantiles = [0.01, 0.10, 0.5]

            for aux_name, aux_values in auxiliaries.items():
                if statistics.is_single_value_series(aux_values) or aux_name == quantity_name:
                    continue

                aux_axis_label = compose_axis_label(aux_name)

                signal_quantile_aux_profile_name = formatter.format(plot_name, subplot_name=aux_name + '_signal_quantiles_aux2d')
                signal_quantile_aux_profile = ValidationPlot(signal_quantile_aux_profile_name)
                signal_quantile_aux_profile.hist2d(
                    aux_values[signals],
                    estimates[signals],
                    quantiles=quantiles,
                    bins=('flat', None),
                    lower_bound=(None, lower_bound),
                    upper_bound=(None, upper_bound),
                    outlier_z_score=self.outlier_z_score,
                    allow_discrete=self.allow_discrete,
                )
                signal_quantile_aux_profile.xlabel = aux_axis_label
                signal_quantile_aux_profile.ylabel = cut_x_label
                self.plots[signal_quantile_aux_profile_name] = signal_quantile_aux_profile

        # ROC plots
        if not estimate_is_binary and cut_direction is not None:
            n_data = len(estimates)
            n_signals = scores.signal_amount(truths, estimates)
            n_bkgs = n_data - n_signals

            # work around for numpy sorting nan values as high but we want it as low depending on the cut direction
            if cut_direction < 0:  # reject low
                sorting_indices = np.argsort(-estimates)
            else:
                sorting_indices = np.argsort(estimates)

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
                lower_bound=lower_bound,
                upper_bound=upper_bound,
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )

            efficiency_by_cut_profile.xlabel = cut_x_label
            efficiency_by_cut_profile.ylabel = "efficiency"

            self.plots["efficiency_by_cut"] = efficiency_by_cut_profile

            # Background rejection over cut value #
            # ################################### #
            bkg_rejection_by_cut_profile_name = formatter.format(plot_name, subplot_name="bkg_rejection_by_cut")
            bkg_rejection_by_cut_profile = ValidationPlot(bkg_rejection_by_cut_profile_name)
            bkg_rejection_by_cut_profile.profile(
                sorted_estimates,
                sorted_bkg_rejections,
                lower_bound=lower_bound,
                upper_bound=upper_bound,
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )

            bkg_rejection_by_cut_profile.xlabel = cut_x_label
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

            # Cut over efficiency #
            # ################### #
            cut_over_efficiency_profile_name = formatter.format(plot_name, subplot_name="cut_over_efficiency")
            cut_over_efficiency_profile = ValidationPlot(cut_over_efficiency_profile_name)
            cut_over_efficiency_profile.profile(
                sorted_efficiencies,
                sorted_estimates,
                lower_bound=0,
                upper_bound=1,
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )
            cut_over_efficiency_profile.set_minimum(lower_bound)
            cut_over_efficiency_profile.set_maximum(upper_bound)
            cut_over_efficiency_profile.xlabel = 'efficiency'
            cut_over_efficiency_profile.ylabel = cut_x_label

            self.plots["cut_over_efficiency"] = cut_over_efficiency_profile

            # Cut over bkg_rejection #
            # ###################### #
            cut_over_bkg_rejection_profile_name = formatter.format(plot_name, subplot_name="cut_over_bkg_rejection")
            cut_over_bkg_rejection_profile = ValidationPlot(cut_over_bkg_rejection_profile_name)
            cut_over_bkg_rejection_profile.profile(
                sorted_bkg_rejections,
                sorted_estimates,
                lower_bound=0,
                upper_bound=1,
                outlier_z_score=self.outlier_z_score,
                allow_discrete=self.allow_discrete,
            )
            cut_over_bkg_rejection_profile.set_minimum(lower_bound)
            cut_over_bkg_rejection_profile.set_maximum(upper_bound)
            cut_over_bkg_rejection_profile.xlabel = 'bkg_rejection'
            cut_over_bkg_rejection_profile.ylabel = cut_x_label

            self.plots["cut_over_bkg_rejection"] = cut_over_bkg_rejection_profile

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

        for plot in list(self.plots.values()):
            plot.contact = contact

        if self.fom:
            self.fom.contact = contact

    def write(self, tdirectory=None):
        for plot in list(self.plots.values()):
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
            print("Cut accepts >= ", self.cut_value_, 'with')
        else:
            print("Cut accepts <= ", self.cut_value_, 'with')

        binary_estimates = self.predict(estimates)

        efficiency = scores.efficiency(truths, binary_estimates)
        purity = scores.purity(truths, binary_estimates)
        background_rejection = scores.background_rejection(truths, binary_estimates)

        print("efficiency", efficiency)
        print("purity", purity)
        print("background_rejection", background_rejection)


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
