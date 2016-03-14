#!/usr/bin/env python
# -*- coding: utf-8 -*-

from tracking.validation.plot import ValidationPlot, compose_axis_label

# get error function as a np.ufunc vectorised for numpy array
from tracking.validation.utilities import erf, root_save_name

import math
import collections

import numpy as np


class PullAnalysis(object):
    default_outlier_z_score = 5.0
    default_plot_name = "{plot_name_prefix}_{subplot_name}{plot_name_postfix}"
    default_plot_title = "{subplot_title} of {quantity_name}{plot_title_postfix}"

    def __init__(
        self,
        quantity_name,
        unit=None,
        outlier_z_score=None,
        absolute=False,
        contact='',
        plot_name=None,
        plot_title=None,
        plot_name_prefix='',  # depricated use plot_name instead
        plot_name_postfix='',  # depricated use plot_name instead
        plot_title_postfix='',  # depricated use plot_title instead
    ):
        """Performs a comparision of an estimated quantity to their truths by generating standardized validation plots."""

        self.quantity_name = quantity_name
        self.unit = unit

        if outlier_z_score is None:
            self.outlier_z_score = self.default_outlier_z_score
        else:
            self.outlier_z_score = outlier_z_score

        self.absolute = absolute

        self.plot_name = plot_name
        self.plot_title = plot_title

        self.plot_name_prefix = plot_name_prefix or root_save_name(quantity_name)
        self.plot_name_postfix = plot_name_postfix
        self.plot_title_postfix = plot_title_postfix

        self._contact = contact
        self.plots = collections.OrderedDict()

    def analyse(
        self,
        truths,
        estimates,
        variances=None,
    ):
        """Compares the concrete estimate to the truth and generates plots of the estimates, residuals, pulls and p-values.
        Close indicates if the figure shall be closed after they are saved."""

        # truths can contain NaN entries if no MC track could be matched

        quantity_name = self.quantity_name

        axis_label = compose_axis_label(quantity_name, self.unit)

        plot_name_prefix = self.plot_name_prefix
        outlier_z_score = self.outlier_z_score

        absolute = self.absolute
        # Compare only the absolute value by taking the absolute of hte curvature truth
        # and flip the sign of the estimate
        if absolute:
            absolute_truths = truths.copy()
            absolute_estimates = estimates.copy()

            flip_sign_for = truths < 0
            absolute_truths[flip_sign_for] = -truths[flip_sign_for]
            absolute_estimates[flip_sign_for] = -estimates[flip_sign_for]

            truths = absolute_truths
            estimates = absolute_estimates

            quantity_name = 'absolute ' + quantity_name

        residuals = estimates - truths

        if variances is not None:
            sigmas = np.sqrt(variances)
            pulls = np.divide(residuals, sigmas)
            p_values = 1.0 - erf(np.abs(pulls))

        plot_name = self.plot_name
        if plot_name is None:
            plot_name = self.default_plot_name

        plot_name = plot_name.format(subplot_name='{subplot_name}',
                                     quantity_name=quantity_name,
                                     plot_name_prefix=plot_name_prefix,
                                     plot_name_postfix=self.plot_name_postfix)

        plot_title = self.plot_title
        if plot_title is None:
            plot_title = self.default_plot_title

        plot_title = plot_title.format(subplot_title='{subplot_title}',
                                       quantity_name=quantity_name,
                                       plot_title_postfix=self.plot_title_postfix)

        # Truths #
        ##########

        # Distribution of truths
        truths_hist_name = plot_name.format(subplot_name="truths")
        truths_hist = ValidationPlot(truths_hist_name)
        truths_hist.hist(truths,
                         outlier_z_score=outlier_z_score)
        truths_hist.xlabel = axis_label
        truths_hist.title = plot_title.format(subplot_title='True distribution')

        self.plots['truths'] = truths_hist

        # Estimates #
        #############

        # Distribution of estimates
        estimates_hist_name = plot_name.format(subplot_name="estimates")
        estimates_hist = ValidationPlot(estimates_hist_name)
        estimates_hist.hist(estimates,
                            outlier_z_score=outlier_z_score)
        estimates_hist.xlabel = axis_label
        estimates_hist.title = plot_title.format(subplot_title='Estimates distribution')

        self.plots['estimates'] = estimates_hist

        # Diagonal plots #
        ##################

        # Estimates versus truths scatter plot
        estimates_by_truths_scatter_name = plot_name.format(subplot_name="diag_scatter")
        estimates_by_truths_scatter = ValidationPlot(estimates_by_truths_scatter_name)
        estimates_by_truths_scatter.scatter(truths,
                                            estimates,
                                            outlier_z_score=outlier_z_score)
        estimates_by_truths_scatter.xlabel = 'True ' + axis_label
        estimates_by_truths_scatter.ylabel = 'Estimated ' + axis_label
        estimates_by_truths_scatter.title = plot_title.format(subplot_title='Diagonal scatter plot')

        self.plots['diag_scatter'] = estimates_by_truths_scatter

        # Estimates versus truths profile plot
        estimates_by_truths_profile_name = plot_name.format(subplot_name="diag_profile")
        estimates_by_truths_profile = ValidationPlot(estimates_by_truths_profile_name)
        estimates_by_truths_profile.profile(truths,
                                            estimates,
                                            outlier_z_score=outlier_z_score)
        estimates_by_truths_profile.xlabel = 'True ' + axis_label
        estimates_by_truths_profile.ylabel = 'Estimated ' + axis_label
        estimates_by_truths_profile.title = plot_title.format(subplot_title='Diagonal profile')
        estimates_by_truths_profile.fit_diag()

        self.plots['diag_profile'] = estimates_by_truths_profile

        # Residuals #
        #############

        # Distribution of the residuals
        residuals_hist_name = plot_name.format(subplot_name="residuals")
        residuals_hist = ValidationPlot(residuals_hist_name)
        residuals_hist.hist(residuals,
                            outlier_z_score=outlier_z_score)
        residuals_hist.xlabel = axis_label
        residuals_hist.title = plot_title.format(subplot_title='Residual distribution')

        self.plots['residuals'] = residuals_hist

        # Variances #
        #############
        if variances is not None:

            # Distribution of sigmas
            sigmas_hist_name = plot_name.format(subplot_name="sigmas")
            sigmas_hist = ValidationPlot(sigmas_hist_name)
            sigmas_hist.hist(sigmas,
                             lower_bound=0,
                             outlier_z_score=outlier_z_score)
            sigmas_hist.xlabel = "#sigma_" + axis_label
            sigmas_hist.title = plot_title.format(subplot_title='Estimated variance distribution')

            self.plots['sigmas'] = sigmas_hist

        # Pulls #
        #########
        if variances is not None:

            # Distribution of pulls
            pulls_hist_name = plot_name.format(subplot_name="pulls")
            pulls_hist = ValidationPlot(pulls_hist_name)
            pulls_hist.hist(pulls, outlier_z_score=outlier_z_score)
            pulls_hist.xlabel = axis_label
            pulls_hist.title = plot_title.format(subplot_title='Pull distribution')
            pulls_hist.fit_gaus()

            self.plots['pulls'] = pulls_hist

        # P-Values #
        ############
        if variances is not None:

            # Distribution of p_values
            p_values_hist_name = plot_name.format(subplot_name="p-values")
            p_values_hist = ValidationPlot(p_values_hist_name)
            p_values_hist.hist(p_values, lower_bound=0, upper_bound=1)
            p_values_hist.xlabel = axis_label
            p_values_hist.title = plot_title.format(subplot_title='P-value distribution')
            p_values_hist.fit_const()

            self.plots['p_values'] = p_values_hist

        # Forward the contract to all plots by reassigning the contact.
        self.contact = self.contact

    @property
    def contact(self):
        return self._contact

    @contact.setter
    def contact(self, contact):
        self._contact = contact
        for validation_plot in self.plots.values():
            validation_plot.contact = contact

    def write(self, tDirectory=None):
        # Write all validation plot to the given Root directory
        for validation_plot in self.plots.values():
            validation_plot.write(tDirectory)
