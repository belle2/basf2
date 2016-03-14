#!/usr/bin/env python
# -*- coding: utf-8 -*-

from tracking.validation.plot import ValidationPlot, compose_axis_label, \
    root_save_name

# get error function as a np.ufunc vectorised for numpy array
from tracking.validation.utilities import erf

import math
import collections

import numpy as np


class PullAnalysis:

    def __init__(
        self,
        quantity_name,
        unit=None,
        outlier_z_score=5.0,
        plot_name_prefix='',
        plot_name_postfix='',
        plot_title_postfix='',
        ):
        """Performs a comparision of an estimated quantity to their truths by generating standardized validation plots."""

        self.quantity_name = quantity_name
        self.unit = unit
        self.outlier_z_score = outlier_z_score

        deletechars = r"/$\#"
        save_quantity_name = quantity_name.replace(' ', '_').translate(None,
                deletechars)
        self.plot_name_prefix = plot_name_prefix \
            or root_save_name(quantity_name)
        self.plot_name_postfix = plot_name_postfix
        self.plot_title_postfix = plot_title_postfix

        self._contact = ''
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

        residuals = estimates - truths

        if variances is not None:
            sigmas = np.sqrt(variances)
            pulls = residuals / sigmas
            p_values = 1.0 - erf(np.abs(pulls))

        # Truths #
        ##########

        # Distribution of truths
        truths_hist = ValidationPlot('%s_truths%s' % (plot_name_prefix,
                                     self.plot_name_postfix))
        truths_hist.hist(truths, outlier_z_score=outlier_z_score)
        truths_hist.xlabel = axis_label
        truths_hist.title = 'True distribution of %s%s' % (quantity_name,
                self.plot_title_postfix)

        self.plots['truths'] = truths_hist

        # Estimates #
        #############

        # Distribution of estimates
        estimates_hist = ValidationPlot('%s_estimates%s' % (plot_name_prefix,
                self.plot_name_postfix))
        estimates_hist.hist(estimates, outlier_z_score=outlier_z_score)
        estimates_hist.xlabel = axis_label
        estimates_hist.title = 'Distribution of %s estimates%s' \
            % (quantity_name, self.plot_title_postfix)

        self.plots['estimates'] = estimates_hist

        # Diagonal plots #
        ##################

        # Estimates versus truths scatter plot
        estimates_by_truths_scatter = ValidationPlot('%s_diag_scatter%s'
                % (plot_name_prefix, self.plot_name_postfix))
        estimates_by_truths_scatter.scatter(truths, estimates,
                outlier_z_score=outlier_z_score)
        estimates_by_truths_scatter.xlabel = 'True ' + axis_label
        estimates_by_truths_scatter.ylabel = 'Estimated ' + axis_label
        estimates_by_truths_scatter.title = 'Diagonal %s scatter plot%s' \
            % (quantity_name, self.plot_title_postfix)

        self.plots['diag_scatter'] = estimates_by_truths_scatter

        # Estimates versus truths profile plot
        estimates_by_truths_profile = ValidationPlot('%s_diag_profile%s'
                % (plot_name_prefix, self.plot_name_postfix))
        estimates_by_truths_profile.profile(truths, estimates,
                outlier_z_score=outlier_z_score)
        estimates_by_truths_profile.xlabel = 'True ' + axis_label
        estimates_by_truths_profile.ylabel = 'Estimated ' + axis_label
        estimates_by_truths_profile.title = 'Diagonal %s profile plot%s' \
            % (quantity_name, self.plot_title_postfix)

        estimates_by_truths_profile.fit_diag()

        self.plots['diag_profile'] = estimates_by_truths_profile

        # Residuals #
        #############

        # Distribution of the residuals
        residuals_hist = ValidationPlot('%s_residuals%s' % (plot_name_prefix,
                self.plot_name_postfix))
        residuals_hist.hist(residuals, outlier_z_score=outlier_z_score)
        residuals_hist.xlabel = axis_label
        residuals_hist.title = 'Distribution of %s residuals%s' \
            % (quantity_name, self.plot_title_postfix)

        self.plots['residuals'] = residuals_hist

        # Variances #
        #############
        if variances is not None:

            # Distribution of sigmas
            sigmas_hist = ValidationPlot('%s_sigmas%s' % (plot_name_prefix,
                    self.plot_name_postfix))
            sigmas_hist.hist(sigmas, lower_bound=0,
                             outlier_z_score=outlier_z_score)
            sigmas_hist.xlabel = axis_label
            sigmas_hist.title = 'Distribution of #sigma(%s)%s' \
                % (quantity_name, self.plot_title_postfix)

            self.plots['sigmas'] = sigmas_hist

        # Pulls #
        #########
        if variances is not None:

            # Distribution of pulls
            pulls_hist = ValidationPlot('%s_pulls%s' % (plot_name_prefix,
                    self.plot_name_postfix))
            pulls_hist.xlabel = axis_label
            pulls_hist.title = 'Distribution of %s pulls%s' % (quantity_name,
                    self.plot_title_postfix)
            pulls_hist.hist(pulls, outlier_z_score=outlier_z_score)

            pulls_hist.fit_gaus()

            self.plots['pulls'] = pulls_hist

        # P-Values #
        ############
        if variances is not None:

            # Distribution of p_values
            p_values_hist = ValidationPlot('%s_p_values%s'
                    % (plot_name_prefix, self.plot_name_postfix))
            p_values_hist.hist(p_values, lower_bound=0, upper_bound=1)
            p_values_hist.xlabel = axis_label
            p_values_hist.title = 'Distribution of %s p-values%s' \
                % (quantity_name, self.plot_title_postfix)

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


