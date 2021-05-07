#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from tracking.validation.plot import ValidationPlot, compose_axis_label, get_unit

# get error function as a np.ufunc vectorised for numpy array
from tracking.validation.utilities import erf
from tracking.root_utils import root_save_name

import collections

import numpy as np

from tracking.validation.tolerate_missing_key_formatter import TolerateMissingKeyFormatter

formatter = TolerateMissingKeyFormatter()


class PullAnalysis(object):
    """Performs a comparision of an estimated quantity to their truths by generating standardized validation plots."""
    #: default outlier z score
    default_outlier_z_score = 5.0
    #: default plot name
    default_plot_name = "{plot_name_prefix}_{subplot_name}{plot_name_postfix}"
    #: default plot title
    default_plot_title = "{subplot_title} of {quantity_name}{plot_title_postfix}"
    #: default list of plots to be created in this analysis
    default_which_plots = [
        "truths",
        "estimates",
        "diag_profile",
        "diag_scatter",
        "residuals",
        "sigmas",
        "pulls",
        "p_values",
        "aux_residual_hist2d",
        "aux_residual_profile",
        "aux_pull_hist2d",
        "aux_pull_profile",
    ]

    #: if true the plots created here are declared as expert plots in the validation
    default_is_expert = True

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
        referenceFileName=None  # if set binnings of plots will be read from corresponding histograms
    ):
        """Performs a comparision of an estimated quantity to their truths by generating standardized validation plots."""

        #: name of the quantity the analysis is performed on
        self.quantity_name = quantity_name
        #: unit the quanitity is given in
        self.unit = unit or get_unit(quantity_name)

        #: the outlier score defines in terms of how many std deviations a data point is considered as an outlier
        if outlier_z_score is None:
            self.outlier_z_score = self.default_outlier_z_score
        else:
            self.outlier_z_score = outlier_z_score

        #: if true only the absolute value is compared
        self.absolute = absolute

        #: name of the plot
        self.plot_name = plot_name
        #: title of the plot
        self.plot_title = plot_title

        #: prefix to be prepended to the plot name
        self.plot_name_prefix = plot_name_prefix or root_save_name(quantity_name)
        #: post fix to be append after the plot name
        self.plot_name_postfix = plot_name_postfix
        #: postfix to be appended after the title
        self.plot_title_postfix = plot_title_postfix

        #: contact information
        self._contact = contact
        #: dictionary to store the plots
        self.plots = collections.OrderedDict()

        #: name of the reference file, if set the binnings of plots will be read from the corresponding object in that file
        self.referenceFileName = referenceFileName

    def analyse(
        self,
        truths,
        estimates,
        variances=None,
        auxiliaries={},
        which_plots=None,
        is_expert=None
    ):
        """Compares the concrete estimate to the truth and generates plots of the estimates, residuals, pulls and p-values.
        Close indicates if the figure shall be closed after they are saved.

        Parameters
        ----------
        truths : array_like(float)
            Sample of the true values
        estimates : array_like(float)
            Corresponding estimations
        variances : array_like(float), optional
            Corresponding variance estimations
        auxiliaries : Dict[name, array_like(float)], optional
            Auxiliary variable to show distribution of residuals and pull as function
        selected_plots : list(str), optional
            List of analysis plots to be generated. All if not given.
            Currently valid names are
            truths, estimates, diag_profile, diag_scatter, residuals,
            sigmas, pulls, p_values
        """

        if is_expert is None:
            is_expert = self.default_is_expert

        if which_plots is None:
            which_plots = self.default_which_plots

        quantity_name = self.quantity_name

        axis_label = compose_axis_label(quantity_name, self.unit)

        plot_name_prefix = self.plot_name_prefix
        outlier_z_score = self.outlier_z_score

        absolute = self.absolute
        # Compare only the absolute value by taking the absolute of the curvature truth
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

        # Truths #
        ##########

        if "truths" in which_plots:
            # Distribution of truths
            truths_hist_name = formatter.format(plot_name, subplot_name="truths")
            truths_hist = ValidationPlot(truths_hist_name, self.referenceFileName)
            truths_hist.hist(truths,
                             outlier_z_score=outlier_z_score,
                             is_expert=is_expert)
            truths_hist.xlabel = axis_label
            truths_hist.title = formatter.format(plot_title, subplot_title='True distribution')

            self.plots['truths'] = truths_hist

        # Estimates #
        #############

        if "estimates" in which_plots:
            # Distribution of estimates
            estimates_hist_name = formatter.format(plot_name, subplot_name="estimates")
            estimates_hist = ValidationPlot(estimates_hist_name, self.referenceFileName)
            estimates_hist.hist(estimates,
                                outlier_z_score=outlier_z_score,
                                is_expert=is_expert)
            estimates_hist.xlabel = axis_label
            estimates_hist.title = formatter.format(plot_title, subplot_title='Estimates distribution')

            self.plots['estimates'] = estimates_hist

        # Diagonal plots #
        ##################
        if "diag_scatter" in which_plots:
            # Estimates versus truths scatter plot
            estimates_by_truths_scatter_name = formatter.format(plot_name, subplot_name="diag_scatter")
            estimates_by_truths_scatter = ValidationPlot(estimates_by_truths_scatter_name, self.referenceFileName)
            estimates_by_truths_scatter.scatter(truths,
                                                estimates,
                                                outlier_z_score=outlier_z_score,
                                                is_expert=is_expert)
            estimates_by_truths_scatter.xlabel = 'True ' + axis_label
            estimates_by_truths_scatter.ylabel = 'Estimated ' + axis_label
            estimates_by_truths_scatter.title = formatter.format(plot_title, subplot_title='Diagonal scatter plot')

            self.plots['diag_scatter'] = estimates_by_truths_scatter

        if "diag_profile" in which_plots:
            # Estimates versus truths profile plot
            estimates_by_truths_profile_name = formatter.format(plot_name, subplot_name="diag_profile")
            estimates_by_truths_profile = ValidationPlot(estimates_by_truths_profile_name, self.referenceFileName)

            # Fill residuals and correct afterwards
            estimates_by_truths_profile.profile(truths,
                                                estimates - truths,
                                                outlier_z_score=outlier_z_score,
                                                gaus_z_score=4,
                                                is_expert=is_expert)

            # Correct with TF1 - only works because the gaus fit is active.
            hist = estimates_by_truths_profile.histograms[0]
            GetBinContent = hist.GetBinContent
            GetBinCenter = hist.GetBinCenter
            SetBinContent = hist.SetBinContent
            for i_bin in range(hist.GetNbinsX() + 2):
                residual = GetBinContent(i_bin)
                truth = GetBinCenter(i_bin)
                if residual != 0:
                    SetBinContent(i_bin, residual + truth)

            # Reset maximum and minimum
            estimates_by_truths_profile.histograms[0].SetMaximum()
            estimates_by_truths_profile.histograms[0].SetMinimum()

            estimates_by_truths_profile.xlabel = 'True ' + axis_label
            estimates_by_truths_profile.ylabel = 'Estimated ' + axis_label

            estimates_by_truths_profile.title = formatter.format(plot_title, subplot_title='Diagonal profile')
            estimates_by_truths_profile.fit_diag()

            self.plots['diag_profile'] = estimates_by_truths_profile

        # Residuals #
        #############
        if "residuals" in which_plots:
            # Distribution of the residuals
            residuals_hist_name = formatter.format(plot_name, subplot_name="residuals")
            residuals_hist = ValidationPlot(residuals_hist_name, self.referenceFileName)
            residuals_hist.hist(residuals,
                                outlier_z_score=outlier_z_score,
                                is_expert=is_expert)
            residuals_hist.xlabel = compose_axis_label("#Delta " + quantity_name + " (estimate - truth)", self.unit)
            residuals_hist.title = formatter.format(plot_title, subplot_title='Residual distribution')

            self.plots['residuals'] = residuals_hist

        # Variances #
        #############
        if variances is not None and "sigmas" in which_plots:

            # Distribution of sigmas
            sigmas_hist_name = formatter.format(plot_name, subplot_name="sigmas")
            sigmas_hist = ValidationPlot(sigmas_hist_name, self.referenceFileName)
            sigmas_hist.hist(sigmas,
                             lower_bound=0,
                             outlier_z_score=outlier_z_score,
                             is_expert=is_expert)
            sigmas_hist.xlabel = compose_axis_label("#sigma (" + quantity_name + ')', self.unit)
            sigmas_hist.title = formatter.format(plot_title, subplot_title='Estimated variance distribution')

            self.plots['sigmas'] = sigmas_hist

        # Pulls #
        #########
        if variances is not None and "pulls" in which_plots:

            # Distribution of pulls
            pulls_hist_name = formatter.format(plot_name, subplot_name="pulls")
            pulls_hist = ValidationPlot(pulls_hist_name, self.referenceFileName)
            pulls_hist.hist(pulls, outlier_z_score=outlier_z_score, is_expert=is_expert)
            pulls_hist.xlabel = "pull (" + quantity_name + ")"
            pulls_hist.title = formatter.format(plot_title, subplot_title='Pull distribution')
            pulls_hist.fit_gaus(z_score=1)

            self.plots['pulls'] = pulls_hist

        # P-Values #
        ############
        if variances is not None and "p_values" in which_plots:

            # Distribution of p_values
            p_values_hist_name = formatter.format(plot_name, subplot_name="p-values")
            p_values_hist = ValidationPlot(p_values_hist_name, self.referenceFileName)
            p_values_hist.hist(p_values, lower_bound=0, upper_bound=1, is_expert=is_expert)
            p_values_hist.xlabel = "p-value (" + quantity_name + ")"
            p_values_hist.title = formatter.format(plot_title, subplot_title='P-value distribution')
            p_values_hist.fit_const()

            self.plots['p_values'] = p_values_hist

        # Auxialliary variables #
        # ##################### #
        for aux_name, aux_values in auxiliaries.items():
            if "aux_residual_hist2d" in which_plots or "aux" in which_plots:
                # Distribution of the residuals over auxiliary variable
                aux_residuals_hist2d_name = formatter.format(plot_name,
                                                             subplot_name="residuals over {}".format(aux_name))
                aux_residuals_hist2d = ValidationPlot(aux_residuals_hist2d_name, self.referenceFileName)
                aux_residuals_hist2d.hist2d(aux_values,
                                            residuals,
                                            outlier_z_score=outlier_z_score,
                                            allow_discrete=True,
                                            is_expert=is_expert)
                aux_residuals_hist2d.xlabel = compose_axis_label(aux_name)
                aux_residuals_hist2d.ylabel = compose_axis_label("#Delta " + quantity_name + " (estimate - truth)", self.unit)
                aux_residuals_hist2d.title = formatter.format(plot_title,
                                                              subplot_title='Residual distribution over {}'.format(aux_name))

                self.plots['aux_residuals_hist2d_' + aux_name] = aux_residuals_hist2d

            if "aux_residual_profile" in which_plots or "aux" in which_plots:
                # Distribution of the residuals over auxiliary variable
                aux_residuals_profile_name = formatter.format(plot_name,
                                                              subplot_name="residuals profile over {}".format(aux_name))
                aux_residuals_profile = ValidationPlot(aux_residuals_profile_name, self.referenceFileName)
                aux_residuals_profile.profile(aux_values,
                                              residuals,
                                              outlier_z_score=outlier_z_score,
                                              gaus_z_score=1.5,
                                              allow_discrete=True,
                                              is_expert=is_expert,
                                              )
                aux_residuals_profile.xlabel = compose_axis_label(aux_name)
                aux_residuals_profile.ylabel = compose_axis_label("#Delta " + quantity_name + " (estimate - truth)", self.unit)
                aux_residuals_profile.title = formatter.format(plot_title,
                                                               subplot_title='Residual profile over {}'.format(aux_name))

                self.plots['aux_residuals_profile_' + aux_name] = aux_residuals_profile

            if variances is not None and ("aux_pull_hist2d" in which_plots or "aux" in which_plots):
                # Distribution of the pulls over auxiliary variable
                aux_pulls_hist2d_name = formatter.format(plot_name,
                                                         subplot_name="pulls over {}".format(aux_name))
                aux_pulls_hist2d = ValidationPlot(aux_pulls_hist2d_name, self.referenceFileName)
                aux_pulls_hist2d.hist2d(aux_values,
                                        pulls,
                                        outlier_z_score=outlier_z_score,
                                        allow_discrete=True,
                                        is_expert=is_expert)
                aux_pulls_hist2d.xlabel = compose_axis_label(aux_name)
                aux_pulls_hist2d.ylabel = "pull (" + quantity_name + ")"
                aux_pulls_hist2d.title = formatter.format(plot_title,
                                                          subplot_title='Pull scatter over {}'.format(aux_name))

                self.plots['aux_pulls_hist2d_' + aux_name] = aux_pulls_hist2d

            if variances is not None and ("aux_pull_profile" in which_plots or "aux" in which_plots):
                # Distribution of the pulls over auxiliary variable
                aux_pulls_profile_name = formatter.format(plot_name,
                                                          subplot_name="pull profile over {}".format(aux_name))
                aux_pulls_profile = ValidationPlot(aux_pulls_profile_name, self.referenceFileName)
                aux_pulls_profile.profile(aux_values,
                                          pulls,
                                          outlier_z_score=outlier_z_score,
                                          gaus_z_score=1.5,
                                          allow_discrete=True,
                                          is_expert=is_expert)
                aux_pulls_profile.xlabel = compose_axis_label(aux_name)
                aux_pulls_profile.ylabel = "pull (" + quantity_name + ")"
                aux_pulls_profile.title = formatter.format(plot_title,
                                                           subplot_title='Pull profile over {}'.format(aux_name))

                self.plots['aux_pulls_profile_' + aux_name] = aux_pulls_profile

        #: Forward the contract to all plots by reassigning the contact.
        self.contact = self.contact

    @property
    def contact(self):
        """ returns the contact """
        return self._contact

    @contact.setter
    def contact(self, contact):
        """
          sets the contact

          parameters:
          contact: new contact information
        """
        self._contact = contact
        for validation_plot in list(self.plots.values()):
            validation_plot.contact = contact

    def write(self, tDirectory=None):
        """ Write all validation plot to the given Root directory
            parameters:
            tDirectory - the root directory were to write to
        """
        for validation_plot in list(self.plots.values()):
            validation_plot.write(tDirectory)
