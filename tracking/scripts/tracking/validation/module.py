#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import collections
import numpy as np

from tracking.validation.plot import ValidationPlot, root_save_name, \
    compose_axis_label

from tracking.validation.pull import PullAnalysis
from tracking.validation.fom import ValidationFiguresOfMerit

from tracking.validation.mc_side_module import MCSideTrackingValidationModule
from tracking.validation.pr_side_module import PRSideTrackingValidationModule
from tracking.validation.eventwise_module import EventwiseTrackingValidationModule

import tracking.metamodules as metamodules

import basf2

import logging

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2


class SeparatedTrackingValidationModule(metamodules.PathModule):

    MCSideModule = MCSideTrackingValidationModule
    PRSideModule = PRSideTrackingValidationModule
    EventwiseModule = EventwiseTrackingValidationModule

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 track_cands_name="TrackCands",
                 mc_track_cands_name="MCTrackCands",
                 expert_level=None):

        # Output TFile to be opened in the initialize methode
        self.output_file_name = output_file_name

        # First forward the output_file_name to the separate modules
        # but we open the TFile in the top level and forward it to them on initialize.

        eventwise_module = self.EventwiseModule(name,
                                                contact,
                                                output_file_name=output_file_name,
                                                track_cands_name=track_cands_name,
                                                mc_track_cands_name=mc_track_cands_name,
                                                expert_level=expert_level)

        pr_side_module = self.PRSideModule(name,
                                           contact,
                                           output_file_name=output_file_name,
                                           track_cands_name=track_cands_name,
                                           mc_track_cands_name=mc_track_cands_name,
                                           expert_level=expert_level)

        mc_side_module = self.MCSideModule(name,
                                           contact,
                                           output_file_name=output_file_name,
                                           track_cands_name=track_cands_name,
                                           mc_track_cands_name=mc_track_cands_name,
                                           expert_level=expert_level)

        self.modules = [
            eventwise_module,
            pr_side_module,
            mc_side_module,
        ]

        super().__init__(modules=self.modules)

    def initialize(self):
        super().initialize()
        output_tfile = ROOT.TFile(self.output_file_name, "RECREATE")

        # Substitute the output file with the common output file opened in this module.
        for module in self.modules:
            module.output_file_name = output_tfile

        self.output_tfile = output_tfile

    def terminate(self):
        self.output_tfile.Flush()
        self.output_tfile.Close()
        super().terminate()


# contains all informations necessary for track filters to decide whether
# track will be included into the processed list of tracks
# This class is used for both providing information on pattern reco and
# MC tracks

class FilterProperties(object):

    def __init__(
        self,
        trackCand=None,
        mcParticle=None,
        mcParticles=None,
        wasFitted=False,
        fitResult=None,
        seedResult=None,
    ):

        self.trackCand = trackCand
        self.mcParticle = mcParticle
        self.mcParticles = mcParticles
        self.wasFitted = wasFitted
        self.fitResult = fitResult
        self.seedResult = seedResult


# This class will accept all pattern reco and mctracks
# It is used as the default option for the TrackingValidationModule
#
# It can be used as a starting point to implement your own particle filter class
# and supply it to the TrackingValidationModule via the init method's
# track_filter_object property.
#
#  doesPrPass is called for all pattern reconstructed tracks
#    Objects available infilterProperties:
#    trackCand is guaranteed to be != None
#    If wasFitted == True, fitResult will be set
#        otherwise seedResult will be set
#
#  doesMcPass is called for all MC tracks
#    Objects available infilterProperties:
#    mcParticle is guaranteed to be != None
#

class AlwaysPassFilter(object):

    def doesPrPass(self, filterProperties):
        return True

    def doesMcPass(self, filterProperties):
        return True


class TrackingValidationModule(basf2.Module):

    """Module to collect matching information about the found particles and to
       generate validation plots and figures of merit on the performance of track finding."""

    def __init__(
        self,
        name,
        contact,
        fit=False,
        pulls=False,
        output_file_name=None,
        track_filter_object=AlwaysPassFilter(),
        plot_name_postfix='',
        plot_title_postfix='',
        exclude_profile_mc_parameter='',
        exclude_profile_pr_parameter='',
        use_expert_folder=True,
        trackCandidatesColumnName="RecoTracks",
        mcTrackCandidatesColumName="MCRecoTracks"
    ):

        super(TrackingValidationModule, self).__init__()
        self.validation_name = name
        self.contact = contact
        self.fit = fit
        self.pulls = pulls
        self.output_file_name = output_file_name or self.validation_name \
            + 'TrackingValidation.root'
        self.track_filter_object = track_filter_object
        self.plot_name_postfix = plot_name_postfix
        self.plot_title_postfix = plot_title_postfix
        self.exclude_profile_pr_parameter = exclude_profile_pr_parameter
        self.exclude_profile_mc_parameter = exclude_profile_mc_parameter
        self.use_expert_folder = use_expert_folder
        self.trackCandidatesColumnName = trackCandidatesColumnName
        self.mcTrackCandidatesColumnName = mcTrackCandidatesColumName

    def initialize(self):
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

        # Use deques in favour of lists to prevent repeated memory allocation of cost O(n)
        self.pr_clones_and_matches = collections.deque()
        self.pr_matches = collections.deque()
        self.pr_fakes = collections.deque()

        self.pr_seed_tan_lambdas = collections.deque()
        self.pr_seed_phi = collections.deque()
        self.pr_seed_theta = collections.deque()

        self.pr_omega_truths = collections.deque()
        self.pr_omega_estimates = collections.deque()
        self.pr_omega_variances = collections.deque()

        self.pr_tan_lambda_truths = collections.deque()
        self.pr_tan_lambda_estimates = collections.deque()
        self.pr_tan_lambda_variances = collections.deque()

        self.mc_matches = collections.deque()
        self.mc_d0s = collections.deque()
        self.mc_tan_lambdas = collections.deque()
        # direction of the track in theta
        self.mc_theta = collections.deque()
        # direction of the track in phi
        self.mc_phi = collections.deque()
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

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        mcParticles = Belle2.PyStoreArray("MCParticles")
        if not trackCands:
            return

        for trackCand in trackCands:
            is_matched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
            is_clone = trackMatchLookUp.isClonePRTrackCand(trackCand)

            omega_truth = float('nan')
            tan_lambda_truth = float('nan')
            mcParticle = None
            if is_matched or is_clone:
                # Only matched and clone tracks have a related MCParticle
                mcParticle = trackMatchLookUp.getRelatedMCParticle(trackCand)
                mcHelix = getHelixFromMCParticle(mcParticle)
                omega_truth = mcHelix.getOmega()
                tan_lambda_truth = mcHelix.getTanLambda()

            # fill the FilterProperties will all properties on this track
            # gathered so far
            filterProperties = FilterProperties(trackCand=trackCand,
                                                mcParticle=mcParticle, mcParticles=mcParticles)

            if self.fit:
                prTrackFitResult = \
                    trackMatchLookUp.getRelatedTrackFitResult(trackCand)
                filterProperties.wasFitted = True
                filterProperties.fitResult = prTrackFitResult
            else:
                prTrackFitResult = getSeedTrackFitResult(trackCand)
                filterProperties.seedResult = prTrackFitResult

            # skip this track due to the filtering rules ?
            if not self.track_filter_object.doesPrPass(filterProperties):
                continue

            omega_estimate = float('nan')
            omega_variance = float('nan')
            tan_lambda_estimate = float('nan')
            tan_lambda_variance = float('nan')

            momentum_pt = float('nan')
            momentum = float('nan')

            # store seed information, they are always available from the pattern reco
            # even if the fit was no successful
            # this information can we used when plotting fake tracks, for example
            seed_position = trackCand.getPositionSeed()
            seed_momentum = trackCand.getMomentumSeed()
            seed_tan_lambda = np.divide(1.0, math.tan(seed_momentum.Theta()))  # Avoid zero division exception
            seed_phi = seed_position.Phi()
            seed_theta = seed_position.Theta()

            if prTrackFitResult:
                omega_estimate = prTrackFitResult.getOmega()
                omega_variance = prTrackFitResult.getCov()[9]

                tan_lambda_estimate = prTrackFitResult.getCotTheta()
                tan_lambda_variance = prTrackFitResult.getCov()[14]

                momentum = prTrackFitResult.getMomentum()
                momentum_pt = momentum.Perp()

            # store properties of the seed
            self.pr_seed_tan_lambdas.append(seed_tan_lambda)
            self.pr_seed_phi.append(seed_phi)
            self.pr_seed_theta.append(seed_theta)

            # store properties resulting from this trackfit
            isMatchedOrIsClone = is_matched or is_clone
            self.pr_clones_and_matches.append(isMatchedOrIsClone)
            self.pr_matches.append(is_matched)
            self.pr_fakes.append(not isMatchedOrIsClone)

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
        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)
        mcParticles = Belle2.PyStoreArray('MCParticles')
        if not mcTrackCands:
            return

        multiplicity = mcTrackCands.getEntries()

        for mcTrackCand in mcTrackCands:
            is_matched = trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand)

            hit_efficiency = trackMatchLookUp.getRelatedEfficiency(mcTrackCand)

            mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
            mcHelix = getHelixFromMCParticle(mcParticle)

            # fill the FilterProperties will all properties on this track
            # gathered so far
            filterProperties = FilterProperties(mcParticle=mcParticle,
                                                mcParticles=mcParticles)

            if not self.track_filter_object.doesMcPass(filterProperties):
                continue

            momentum = mcParticle.getMomentum()
            pt = momentum.Perp()
            tan_lambda = np.divide(1.0, math.tan(momentum.Theta()))  # Avoid zero division exception
            d0 = mcHelix.getD0()

            self.mc_matches.append(is_matched)
            self.mc_hit_efficiencies.append(hit_efficiency)
            self.mc_pts.append(pt)
            self.mc_d0s.append(d0)
            self.mc_tan_lambdas.append(tan_lambda)
            self.mc_multiplicities.append(multiplicity)
            self.mc_theta.append(momentum.Theta())
            self.mc_phi.append(momentum.Phi())

    def terminate(self):
        name = self.validation_name
        contact = self.contact

        # Overall figures of merit #
        ############################
        track_finding_efficiency = np.mean(self.mc_matches)
        fake_rate = 1.0 - np.mean(self.pr_clones_and_matches)
        # can only be computed if there are entries
        if len(self.pr_clones_and_matches) > 0 and sum(self.pr_clones_and_matches) > 0:
            clone_rate = 1.0 - np.average(self.pr_matches,
                                          weights=self.pr_clones_and_matches)
        else:
            clone_rate = float('nan')

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
fake_rate - ratio of pattern recognition tracks that are not related to a particle
            (background, ghost) to all pattern recognition tracks <br/>
clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches) <br/>
"""
        figures_of_merit.check = 'Compare for degradations with respect to the reference line'
        figures_of_merit.contact = contact
        print(figures_of_merit)

        # Validation plots #
        ####################
        validation_plots = []
        pull_analyses = []

        # Finding efficiency #
        ######################
        plots = self.profiles_by_mc_parameters(self.mc_matches,
                                               'finding efficiency', make_hist=False)
        validation_plots.extend(plots)

        # Fake rate (all tracks not matched or clone            #
        # use TrackCand seeds for the fake track plotting       #
        # as the fit (if successful) is probably not meaningful #
        #########################################################
        print('fake list: ' + str(len(self.pr_fakes)))
        plots = self.profiles_by_pr_parameters(self.pr_fakes, 'fake rate',
                                               make_hist=False)
        validation_plots.extend(plots)

        # Hit efficiency #
        ##################
        plots = self.profiles_by_mc_parameters(self.mc_hit_efficiencies,
                                               'hit efficiency with matched tracks')
        validation_plots.extend(plots)

        # Fit quality #
        ###############
        if self.pulls:
            plot_name_prefix = name + self.plot_name_postfix
            if not self.fit:
                plot_name_prefix += '_seed'

            # Omega / curvature pull
            pr_omega_truths = np.array(self.pr_omega_truths)
            pr_omega_estimates = np.array(self.pr_omega_estimates)
            pr_omega_variances = np.array(self.pr_omega_variances)

            curvature_pull_analysis = PullAnalysis('#omega', unit='1/cm',
                                                   plot_name_prefix=plot_name_prefix + '_omega',
                                                   plot_title_postfix=self.plot_title_postfix)
            curvature_pull_analysis.analyse(pr_omega_truths,
                                            pr_omega_estimates, pr_omega_variances)
            curvature_pull_analysis.contact = contact
            pull_analyses.append(curvature_pull_analysis)

            # Tan lambda pull
            pr_tan_lambda_truths = np.array(self.pr_tan_lambda_truths)
            pr_tan_lambda_estimates = np.array(self.pr_tan_lambda_estimates)
            pr_tan_lambda_variances = np.array(self.pr_tan_lambda_variances)

            curvature_pull_analysis = PullAnalysis('tan #lambda',
                                                   plot_name_prefix=plot_name_prefix + '_tan_lambda',
                                                   plot_title_postfix=self.plot_title_postfix)
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
        opt_fit = 0o112
        ROOT.gStyle.SetOptFit(opt_fit)

        figures_of_merit.write()

        for validation_plot in validation_plots:
            validation_plot.write()

        if self.use_expert_folder:
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
        parameter_names=[
            'd_0',
            'p_t',
            'tan_lambda',
            'multiplicity',
            'phi',
            'theta',
        ],
        non_expert_parameters=['p_{t}'],
        make_hist=True,
    ):

        # apply exclusion list
        new_parameter_names = [item for item in parameter_names if item
                               not in self.exclude_profile_mc_parameter]

        # Profile versus the various parameters
        profile_parameters = {
            'd_{0}': self.mc_d0s,
            'p_{t}': self.mc_pts,
            'tan #lambda': self.mc_tan_lambdas,
            '#phi': self.mc_phi,
            '#theta': self.mc_theta,
            'multiplicity': self.mc_multiplicities,
        }

        return self.profiles_by_parameters_base(
            xs,
            quantity_name,
            new_parameter_names,
            profile_parameters,
            unit,
            make_hist,
            non_expert_parameters=non_expert_parameters
        )

    def profiles_by_pr_parameters(
        self,
        xs,
        quantity_name,
        unit=None,
        parameter_names=['Seed tan #lambda', 'Seed #phi', 'Seed #theta'],
        make_hist=True,
    ):

        # apply exclusion list
        new_parameter_names = [item for item in parameter_names if item
                               not in self.exclude_profile_pr_parameter]

        # Profile versus the various parameters
        profile_parameters = {'Seed tan #lambda': self.pr_seed_tan_lambdas,
                              'Seed #phi': self.pr_seed_phi,
                              'Seed #theta': self.pr_seed_theta}

        return self.profiles_by_parameters_base(
            xs,
            quantity_name,
            new_parameter_names,
            profile_parameters,
            unit,
            make_hist,
        )

    def profiles_by_parameters_base(
        self,
        xs,
        quantity_name,
        parameter_names,
        profile_parameters,
        unit,
        make_hist,
        non_expert_parameters=[]
    ):

        contact = self.contact

        validation_plots = []
        plot_name_prefix = self.validation_name + '_' + root_save_name(quantity_name) \
            + self.plot_name_postfix

        if make_hist:
            # Histogram of the quantity
            histogram = ValidationPlot(plot_name_prefix)
            histogram.hist(xs)

            histogram.xlabel = quantity_name
            histogram.description = 'Not a serious plot yet.'
            histogram.check = ''
            histogram.contact = contact
            histogram.title = quantity_name + self.plot_title_postfix

            validation_plots.append(histogram)

        for (parameter_name, parameter_values) in list(profile_parameters.items()):
            if parameter_name in parameter_names \
                    or root_save_name(parameter_name) in parameter_names:

                is_expert = not(parameter_name in non_expert_parameters)

                profile_plot_name = plot_name_prefix + '_by_' \
                    + root_save_name(parameter_name)
                profile_plot = ValidationPlot(profile_plot_name)
                profile_plot.profile(parameter_values, xs, outlier_z_score=10.0, is_expert=is_expert)

                profile_plot.xlabel = compose_axis_label(parameter_name)
                profile_plot.ylabel = compose_axis_label(quantity_name, unit)
                profile_plot.title = quantity_name + ' by ' + parameter_name \
                    + ' profile' + self.plot_title_postfix

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
    position = trackCand.getPositionSeed()
    momentum = trackCand.getMomentumSeed()
    cartesian_covariance = trackCand.getSeedCovariance()
    charge_sign = (-1 if trackCand.getChargeSeed() < 0 else 1)
    # It does not matter, which particle we put in here, so we just use a pion
    particle_type = Belle2.Const.pion
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


def main():
    from tracking.run.tracked_event_generation import StandardReconstructionEventsRun
    standard_reco_run = StandardReconstructionEventsRun()
    standard_reco_run.configure_from_commandline()

    validation_module = SeparatedTrackingValidationModule(name="test_run",
                                                          contact="dummy",
                                                          output_file_name="test_separated_module.root",
                                                          expert_level=0)

    standard_reco_run.add_module(validation_module)
    standard_reco_run.execute()


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    main()
