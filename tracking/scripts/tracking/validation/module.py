#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import collections
import numpy as np

from tracking.validation.plot import ValidationPlot, root_save_name, \
    compose_axis_label

from tracking.validation.pull import PullAnalysis
from tracking.validation.fom import ValidationFiguresOfMerit

import basf2

import ROOT
from ROOT import Belle2


class TrackingValidationModule(basf2.Module):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def __init__(
        self,
        name,
        contact,
        fit=False,
        pulls=False,
        output_file_name=None,
        ):

        super(TrackingValidationModule, self).__init__()
        self.name = name
        self.contact = contact
        self.fit = fit
        self.pulls = pulls
        self.output_file_name = output_file_name or self.name \
            + 'TrackingValidation.root'

    def initialize(self):
        self.trackMatchLookUp = Belle2.TrackMatchLookUp('MCTrackCands')

        # Use deques in favour of lists to prevent repeated memory allocation of cost O(n)
        self.pr_clones_and_matches = collections.deque()
        self.pr_matches = collections.deque()

        self.pr_omega_truths = collections.deque()
        self.pr_omega_estimates = collections.deque()
        self.pr_omega_variances = collections.deque()

        self.pr_tan_lambda_truths = collections.deque()
        self.pr_tan_lambda_estimates = collections.deque()
        self.pr_tan_lambda_variances = collections.deque()

        self.mc_matches = collections.deque()
        self.mc_d0s = collections.deque()
        self.mc_tan_lambdas = collections.deque()
        self.mc_pts = collections.deque()
        self.mc_hit_efficiencies = collections.deque()
        self.mc_multiplicities = collections.deque()

    def event(self):
        """Event method"""

        self.examine_pr_tracks()
        self.examine_mc_tracks()

    def examine_pr_tracks(self):
        """Looks at the individual pattern recognition tracks and store information about them"""

        # Analyse from the pattern recognition side
        trackMatchLookUp = self.trackMatchLookUp

        trackCands = Belle2.PyStoreArray('TrackCands')
        if not trackCands:
            return

        for trackCand in trackCands:
            is_matched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
            is_clone = trackMatchLookUp.isClonePRTrackCand(trackCand)

            if self.fit:
                prTrackFitResult = \
                    trackMatchLookUp.getRelatedTrackFitResult(trackCand)
            else:
                prTrackFitResult = getSeedTrackFitResult(trackCand)

            omega_estimate = float('nan')
            omega_variance = float('nan')
            tan_lambda_estimate = float('nan')
            tan_lambda_variance = float('nan')

            if prTrackFitResult != None:
                omega_estimate = prTrackFitResult.getOmega()
                omega_variance = prTrackFitResult.getCov()[9]

                tan_lambda_estimate = prTrackFitResult.getCotTheta()
                tan_lambda_variance = prTrackFitResult.getCov()[14]

            omega_truth = float('nan')
            tan_lambda_truth = float('nan')
            if is_matched or is_clone:
                # Only matched and clone tracks have a related MCParticle
                mcParticle = trackMatchLookUp.getRelatedMCParticle(trackCand)
                mcHelix = getHelixFromMCParticle(mcParticle)
                omega_truth = mcHelix.getOmega()
                tan_lambda_truth = mcHelix.getTanLambda()

            self.pr_clones_and_matches.append(is_matched or is_clone)
            self.pr_matches.append(is_matched)

            self.pr_omega_estimates.append(omega_estimate)
            self.pr_omega_variances.append(omega_variance)
            self.pr_omega_truths.append(omega_truth)

            self.pr_tan_lambda_estimates.append(tan_lambda_estimate)
            self.pr_tan_lambda_variances.append(tan_lambda_variance)
            self.pr_tan_lambda_truths.append(tan_lambda_truth)

    def examine_mc_tracks(self):
        """Looks at the individual Monte Carlo tracks and store information about them"""

        trackMatchLookUp = self.trackMatchLookUp

        # Analyse from the Monte Carlo reference side
        mcTrackCands = Belle2.PyStoreArray('MCTrackCands')
        if not mcTrackCands:
            return

        multiplicity = mcTrackCands.getEntries()

        for mcTrackCand in mcTrackCands:
            is_matched = trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand)

            hit_efficiency = trackMatchLookUp.getRelatedEfficiency(mcTrackCand)

            mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
            mcHelix = getHelixFromMCParticle(mcParticle)

            momentum = mcParticle.getMomentum()
            pt = momentum.Perp()
            tan_lambda = 1 / math.tan(momentum.Theta())
            d0 = mcHelix.getD0()

            self.mc_matches.append(is_matched)
            self.mc_hit_efficiencies.append(hit_efficiency)
            self.mc_pts.append(pt)
            self.mc_d0s.append(d0)
            self.mc_tan_lambdas.append(tan_lambda)
            self.mc_multiplicities.append(multiplicity)

    def terminate(self):
        name = self.name
        contact = self.contact

        # Overall figures of merit #
        ############################
        track_finding_efficiency = np.mean(self.mc_matches)
        fake_rate = 1.0 - np.mean(self.pr_clones_and_matches)
        clone_rate = 1.0 - np.average(self.pr_matches,
                                      weights=self.pr_clones_and_matches)
        hit_efficiency = np.nanmean(self.mc_hit_efficiencies)

        figures_of_merit = ValidationFiguresOfMerit('%s_figures_of_merit'
                % name)
        figures_of_merit['finding_efficiency'] = track_finding_efficiency
        figures_of_merit['fake_rate'] = fake_rate
        figures_of_merit['clone_rate'] = clone_rate
        figures_of_merit['hit_efficiency'] = hit_efficiency

        figures_of_merit.description = \
            """
finding_efficiency - the ratio of matched Monte Carlo tracks to all Monte Carlo tracks <br/>
fake_rate - ratio of pattern recognition tracks that are not related to a particle (background, ghost) to all pattern recognition tracks <br/>
clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches) <br/>
"""
        figures_of_merit.check = ''
        figures_of_merit.contact = contact
        print figures_of_merit

        # Validation plots #
        ####################
        validation_plots = []
        pull_analyses = []

        # Finding efficiency #
        ######################
        plots = self.profiles_by_mc_parameters(self.mc_matches,
                'finding efficiency', make_hist=False)
        validation_plots.extend(plots)

        # Hit efficiency #
        ##################
        plots = self.profiles_by_mc_parameters(self.mc_hit_efficiencies,
                'hit efficiency')
        validation_plots.extend(plots)

        # Fit quality #
        ###############
        if self.pulls:
            plot_name_prefix = name
            if not self.fit:
                plot_name_prefix += '_seed'

            # Omega / curvature pull
            pr_omega_truths = np.array(self.pr_omega_truths)
            pr_omega_estimates = np.array(self.pr_omega_estimates)
            pr_omega_variances = np.array(self.pr_omega_variances)

            curvature_pull_analysis = PullAnalysis('#omega', unit='1/cm',
                    plot_name_prefix=plot_name_prefix + '_omega')
            curvature_pull_analysis.analyse(pr_omega_truths,
                    pr_omega_estimates, pr_omega_variances)
            curvature_pull_analysis.contact = contact
            pull_analyses.append(curvature_pull_analysis)

            # Tan lambda pull
            pr_tan_lambda_truths = np.array(self.pr_tan_lambda_truths)
            pr_tan_lambda_estimates = np.array(self.pr_tan_lambda_estimates)
            pr_tan_lambda_variances = np.array(self.pr_tan_lambda_variances)

            curvature_pull_analysis = PullAnalysis('tan #lambda',
                    plot_name_prefix=plot_name_prefix + '_tan_lambda')
            curvature_pull_analysis.analyse(pr_tan_lambda_truths,
                    pr_tan_lambda_estimates, pr_tan_lambda_variances)
            curvature_pull_analysis.contact = contact
            pull_analyses.append(curvature_pull_analysis)

            # TODO
            # pulls for the vertex residuals
            # push pull analysis to seperate TDirectory and only forward highlights to the top level
            # Profile variances versus parameters

        # Saving #
        ##########

        # Save everything to a ROOT file
        output_tfile = ROOT.TFile(self.output_file_name, 'recreate')

        # Show all parameters and the fit result in the plots
        # if viewed in the browser or the validation
        opt_fit = 0112
        ROOT.gStyle.SetOptFit(opt_fit)

        figures_of_merit.write()

        for validation_plot in validation_plots:
            validation_plot.write()

        expert_tdirectory = output_tfile.mkdir('expert', 'Expert')
        expert_tdirectory.cd()
        ROOT.gStyle.SetOptFit(opt_fit)

        for pull_analysis in pull_analyses:
            pull_analysis.write()

        output_tfile.Close()

    def profiles_by_mc_parameters(
        self,
        xs,
        quantity_name,
        unit=None,
        parameter_names=['d_0', 'p_t', 'tan_lambda', 'multiplicity'],
        make_hist=True,
        ):

        contact = self.contact

        validation_plots = []
        plot_name_prefix = self.name + '_' + root_save_name(quantity_name)

        if make_hist:
            # Histogram of the quantity
            histogram = ValidationPlot(plot_name_prefix)
            histogram.hist(xs)

            histogram.xlabel = quantity_name
            histogram.description = 'Not a serious plot yet.'
            histogram.check = ''
            histogram.contact = contact

            validation_plots.append(histogram)

        # Profile versus the various parameters
        profile_parameters = {
            'd_{0}': self.mc_d0s,
            'p_{t}': self.mc_pts,
            'tan #lambda': self.mc_tan_lambdas,
            'multiplicity': self.mc_multiplicities,
            }

        for (parameter_name, parameter_values) in profile_parameters.items():
            if parameter_name in parameter_names \
                or root_save_name(parameter_name) in parameter_names:

                profile_plot_name = plot_name_prefix + '_by_' \
                    + root_save_name(parameter_name)
                profile_plot = ValidationPlot(profile_plot_name)
                profile_plot.profile(parameter_values, xs)

                profile_plot.xlabel = compose_axis_label(parameter_name)
                profile_plot.ylabel = compose_axis_label(quantity_name, unit)
                profile_plot.title = quantity_name + ' by ' + parameter_name \
                    + ' profile'

                profile_plot.description = \
                    'Dependence of %s of the track over the true %s' \
                    % (quantity_name, parameter_name)
                profile_plot.check = 'Variations should be low'
                profile_plot.contact = contact
                validation_plots.append(profile_plot)

        return validation_plots


def getHelixFromMCParticle(mcParticle):
    position = mcParticle.getVertex()
    momentum = mcParticle.getMomentum()
    charge_sign = (-1 if mcParticle.getCharge() < 0 else 1)
    b_field = 1.5

    seed_helix = Belle2.Helix(position, momentum, charge_sign, b_field)
    return seed_helix


def getSeedTrackFitResult(trackCand):
    position = trackCand.getPosSeed()
    momentum = trackCand.getMomSeed()
    cartesian_covariance = trackCand.getCovSeed()
    charge_sign = (-1 if trackCand.getChargeSeed() < 0 else 1)
    particle_type = Belle2.Const.ParticleType(trackCand.getPdgCode())
    p_value = float('nan')
    b_field = 1.5
    cdc_hit_pattern = 0
    svd_hit_pattern = 0

    track_fit_result = Belle2.TrackFitResult(
        position,
        momentum,
        cartesian_covariance,
        charge_sign,
        particle_type,
        p_value,
        b_field,
        cdc_hit_pattern,
        svd_hit_pattern,
        )

    return track_fit_result


