#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import math
import collections
import numpy as np

from tracking.root_utils import root_save_name

from tracking.validation.plot import ValidationPlot, compose_axis_label

from tracking.validation.pull import PullAnalysis
from tracking.validation.resolution import ResolutionAnalysis
from tracking.validation.fom import ValidationFiguresOfMerit
from tracking.validation.utilities import (
    getHelixFromMCParticle,
    getSeedTrackFitResult,
    is_primary,
    get_det_hit_ids,
    calc_ndf_from_det_hit_ids
)

import basf2

import ROOT
from ROOT import Belle2
import os

ROOT.gSystem.Load("libtracking")


class FilterProperties:
    """
    contains all informations necessary for track filters to decide whether
    track will be included into the processed list of tracks
    This class is used for both providing information on pattern reco and
    MC tracks
    """

    def __init__(
        self,
        trackCand=None,
        mcParticle=None,
        mcParticles=None,
        wasFitted=False,
        fitResult=None,
        seedResult=None,
    ):
        """Constructor"""

        #: cached value of the track candidate
        self.trackCand = trackCand
        #: cached value of the MC particle
        self.mcParticle = mcParticle
        #: cached value of the MCParticles StoreArray
        self.mcParticles = mcParticles
        #: cached value of the fitted flag
        self.wasFitted = wasFitted
        #: cached value of the fit result
        self.fitResult = fitResult
        #: cached value of the seed result
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

class AlwaysPassFilter:
    """Filter that always passes"""

    def doesPrPass(self, filterProperties):
        """Pattern-reconstructed track always passes"""
        return True

    def doesMcPass(self, filterProperties):
        """MC track always passes"""
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
        resolution=False,
        output_file_name=None,
        track_filter_object=AlwaysPassFilter(),
        plot_name_postfix='',
        plot_title_postfix='',
        exclude_profile_mc_parameter='',
        exclude_profile_pr_parameter='',
        use_expert_folder=True,
        trackCandidatesColumnName="RecoTracks",
        mcTrackCandidatesColumName="MCRecoTracks",
        non_expert_parameters=['p_{t}']
    ):
        """Constructor"""

        super().__init__()

        #: cached value of the tracking-validation name
        self.validation_name = name
        #: cached value of the contact person name
        self.contact = contact
        #: cached value of the track fit
        self.fit = fit
        #: cached values of the track-fit pulls
        self.pulls = pulls
        #: cached value of the resolution
        self.resolution = resolution
        #: cached value of the output ROOT TFile
        self.output_file_name = output_file_name or self.validation_name \
            + 'TrackingValidation.root'
        #: cached value of the track-filter object
        self.track_filter_object = track_filter_object
        #: cached value of the suffix appended to the plot names
        self.plot_name_postfix = plot_name_postfix
        #: cached value of the suffix appended to the plot titles
        self.plot_title_postfix = plot_title_postfix
        #: cached list of perigee parameters excluded from PR side plots
        self.exclude_profile_pr_parameter = exclude_profile_pr_parameter
        #: cached list of perigee parameters excluded from MC side plots
        self.exclude_profile_mc_parameter = exclude_profile_mc_parameter
        #: cached flag to use the "expert" folder for the pull and residual plots
        self.use_expert_folder = use_expert_folder
        #: cached name of the RecoTracks StoreArray
        self.trackCandidatesColumnName = trackCandidatesColumnName
        #: cached name of the MCRecoTracks StoreArray
        self.mcTrackCandidatesColumnName = mcTrackCandidatesColumName
        #: list of parameters that determines which plots (all with corresponding x-axis) are marked as shifter plots
        self.non_expert_parameters = non_expert_parameters

        #: default binning used for resolution plots over pt
        self.resolution_pt_binning = [0.05, 0.1, 0.25, 0.4, 0.6, 1., 1.5, 2., 3., 4.]

        #: If this variable is set the code will open the file with same name as the file created here
        #: and will read the binning from the TH1/TProfile with same name as the one created here. If you
        #: do not want this feature either remove the corresponding root files from the validation
        #: directory (this will trigger the default behaviour) or set the environmental variable DO_NOT_READ_BINNING
        self.referenceFileName = None
        if "DO_NOT_READ_BINNING" not in os.environ:
            # the validity of the file will be checked later
            self.referenceFileName = Belle2.FileSystem.findFile("tracking/validation/" + self.output_file_name, True)
            basf2.B2INFO("Will read binning from: " + self.referenceFileName)
            basf2.B2INFO("If this is not wanted set the environment variable DO_NOT_READ_BINNING or remove reference files.")
        else:
            basf2.B2INFO("Will not read binning from reference files.")

    def initialize(self):
        """Receive signal at the start of event processing"""

        #: Track-match object that examines relation information from MCMatcherTracksModule
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

        #: Use deques in favour of lists to prevent repeated memory allocation of cost O(n)

        #: list of PR-track clones and matches
        self.pr_clones_and_matches = collections.deque()
        #: list of PR-track matches
        self.pr_matches = collections.deque()
        #: list of PR-track fakes
        self.pr_fakes = collections.deque()

        #: list of PR-track seed pt values
        self.pr_seed_pt = collections.deque()
        #: list of PR-track seed tan(lambda) values
        self.pr_seed_tan_lambdas = collections.deque()
        #: list of PR-track seed phi values
        self.pr_seed_phi = collections.deque()

        #: list of PR-track seed omega-truth values
        self.pr_omega_truths = collections.deque()
        #: list of PR-track seed omega-estimate values
        self.pr_omega_estimates = collections.deque()
        #: list of PR-track seed omega-variance values
        self.pr_omega_variances = collections.deque()

        #: list of PR-track seed tan(lambda)-truth values
        self.pr_tan_lambda_truths = collections.deque()
        #: list of PR-track seed tan(lambda)-estimate values
        self.pr_tan_lambda_estimates = collections.deque()
        #: list of PR-track seed tan(lambda)-variance values
        self.pr_tan_lambda_variances = collections.deque()

        #: list of PR-track seed d0-truth values
        self.pr_d0_truths = collections.deque()
        #: list of PR-track seed d0-estimate values
        self.pr_d0_estimates = collections.deque()
        #: list of PR-track seed d0-variance values
        self.pr_d0_variances = collections.deque()

        #: list of PR-track seed z0-truth values
        self.pr_z0_truths = collections.deque()
        #: list of PR-track seed z0-estimate values
        self.pr_z0_estimates = collections.deque()

        #: list of PR-track seed pt-truth values
        self.pr_pt_truths = collections.deque()
        #: list of PR-track seed pt-estimate values
        self.pr_pt_estimates = collections.deque()

        #: list of PR-track binning values
        self.pr_bining_pt = collections.deque()

        #: list of MC-track matches
        self.mc_matches = collections.deque()
        #: list of MC-track matches, including matched charge
        self.mc_charge_matches = collections.deque()
        #: list of MC-track matches charge asymmetry
        self.mc_charge_asymmetry = collections.deque()
        #: list of MC-track matches charge asymmetry weights
        self.mc_charge_asymmetry_weights = collections.deque()
        #: list of MC-track primaries
        self.mc_primaries = collections.deque()
        #: list of MC-track d0 values
        self.mc_d0s = collections.deque()
        #: list of MC-track tan(lambda) values
        self.mc_tan_lambdas = collections.deque()
        #: direction of the track in phi
        self.mc_phi = collections.deque()
        #: list of MC-track pt values
        self.mc_pts = collections.deque()
        #: list of MC-track hit efficiencies
        self.mc_hit_efficiencies = collections.deque()
        #: list of MC-track multiplicities
        self.mc_multiplicities = collections.deque()
        #: list of MC-track number of degrees of freedom
        self.mc_ndf = collections.deque()

    def event(self):
        """Event method"""

        self.examine_pr_tracks()
        self.examine_mc_tracks()

    def examine_pr_tracks(self):
        """Looks at the individual pattern reconstructed tracks and store information about them"""

        # Analyse from the pattern recognition side
        trackMatchLookUp = self.trackMatchLookUp

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        mcParticles = Belle2.PyStoreArray("MCParticles")
        if not trackCands:
            return

        for trackCand in trackCands:
            is_matched = trackMatchLookUp.isAnyChargeMatchedPRRecoTrack(trackCand)
            is_clone = trackMatchLookUp.isAnyChargeClonePRRecoTrack(trackCand)

            pt_truth = float('nan')
            omega_truth = float('nan')
            tan_lambda_truth = float('nan')
            d0_truth = float('nan')
            z0_truth = float('nan')

            mcParticle = None
            if is_matched or is_clone:
                # Only matched and clone tracks have a related MCParticle
                mcParticle = trackMatchLookUp.getRelatedMCParticle(trackCand)
                mcHelix = getHelixFromMCParticle(mcParticle)
                omega_truth = mcHelix.getOmega()
                tan_lambda_truth = mcHelix.getTanLambda()
                pt_truth = mcParticle.getMomentum().Rho()
                d0_truth = mcHelix.getD0()
                z0_truth = mcHelix.getZ0()

            # fill the FilterProperties will all properties on this track
            # gathered so far
            filterProperties = FilterProperties(trackCand=trackCand,
                                                mcParticle=mcParticle, mcParticles=mcParticles)

            if self.fit:
                prTrackFitResult = \
                    trackMatchLookUp.getRelatedTrackFitResult(trackCand)
                filterProperties.wasFitted = prTrackFitResult is not None
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
            d0_estimate = float('nan')
            d0_variance = float('nan')
            z0_estimate = float('nan')
            pt_estimate = float('nan')

            momentum = float('nan')

            # store seed information, they are always available from the pattern reco
            # even if the fit was no successful
            # this information can we used when plotting fake tracks, for example
            seed_momentum = trackCand.getMomentumSeed()
            # Avoid zero division exception
            seed_tan_lambda = np.divide(1.0, math.tan(seed_momentum.Theta()))
            seed_phi = seed_momentum.Phi()
            seed_pt = seed_momentum.Rho()

            if prTrackFitResult:
                omega_estimate = prTrackFitResult.getOmega()
                omega_variance = prTrackFitResult.getCov()[9]

                tan_lambda_estimate = prTrackFitResult.getCotTheta()
                tan_lambda_variance = prTrackFitResult.getCov()[14]

                d0_estimate = prTrackFitResult.getD0()
                d0_variance = prTrackFitResult.getCov()[0]

                z0_estimate = prTrackFitResult.getZ0()

                momentum = prTrackFitResult.getMomentum()
                pt_estimate = momentum.Rho()

            # store properties of the seed
            self.pr_seed_pt.append(seed_pt)
            self.pr_seed_tan_lambdas.append(seed_tan_lambda)
            self.pr_seed_phi.append(seed_phi)

            self.pr_bining_pt.append(pt_truth)

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

            self.pr_d0_estimates.append(d0_estimate)
            self.pr_d0_variances.append(d0_variance)
            self.pr_d0_truths.append(d0_truth)

            self.pr_z0_estimates.append(z0_estimate)
            self.pr_z0_truths.append(z0_truth)

            self.pr_pt_estimates.append(pt_estimate)
            self.pr_pt_truths.append(pt_truth)

    def examine_mc_tracks(self):
        """Looks at the individual Monte Carlo tracks and store information about them"""

        trackMatchLookUp = self.trackMatchLookUp

        # Analyse from the Monte Carlo reference side
        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)
        mcParticles = Belle2.PyStoreArray('MCParticles')
        if not mcTrackCands:
            return

        multiplicity = mcTrackCands.getEntries()
        multiplicity_primaries = multiplicity

        # measure the charge asymmetry
        n_matched_plus = 0
        n_matched_minus = 0

        for mcTrackCand in mcTrackCands:
            is_matched = trackMatchLookUp.isAnyChargeMatchedMCRecoTrack(mcTrackCand)

            relatedPRtrackCand = trackMatchLookUp.getRelatedPRRecoTrack(mcTrackCand)
            if relatedPRtrackCand:
                is_chargeMatched = trackMatchLookUp.isChargeMatched(relatedPRtrackCand)
            else:
                is_chargeMatched = False

            if is_chargeMatched:
                if mcTrackCand.getChargeSeed() > 0:
                    n_matched_plus += 1
                else:
                    n_matched_minus += 1

            hit_efficiency = trackMatchLookUp.getRelatedEfficiency(mcTrackCand)
            if math.isnan(hit_efficiency):
                hit_efficiency = 0

            mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
            mcHelix = getHelixFromMCParticle(mcParticle)

            # fill the FilterProperties will all properties on this track
            # gathered so far
            filterProperties = FilterProperties(mcParticle=mcParticle,
                                                mcParticles=mcParticles)

            if not self.track_filter_object.doesMcPass(filterProperties):
                continue

            momentum = mcParticle.getMomentum()
            pt = momentum.Rho()
            tan_lambda = np.divide(1.0, math.tan(momentum.Theta()))  # Avoid zero division exception
            d0 = mcHelix.getD0()
            det_hit_ids = get_det_hit_ids(mcTrackCand)
            ndf = calc_ndf_from_det_hit_ids(det_hit_ids)

            self.mc_matches.append(is_matched)
            self.mc_charge_matches.append(is_chargeMatched and is_matched)
            self.mc_primaries.append(is_primary(mcParticle))
            self.mc_hit_efficiencies.append(hit_efficiency)
            self.mc_pts.append(pt)
            self.mc_d0s.append(d0)
            self.mc_tan_lambdas.append(tan_lambda)
            self.mc_multiplicities.append(multiplicity)
            self.mc_phi.append(momentum.Phi())
            self.mc_ndf.append(ndf)
            if not is_primary(mcParticle):
                multiplicity_primaries -= 1

        charge_asymmetry = (n_matched_plus - n_matched_minus)/(n_matched_plus +
                                                               n_matched_minus) if (n_matched_plus + n_matched_minus) != 0 else 0
        for mcTrackCand in mcTrackCands:
            if is_primary(mcParticle):
                self.mc_charge_asymmetry.append(charge_asymmetry)
                self.mc_charge_asymmetry_weights.append(1./multiplicity_primaries)
            else:
                self.mc_charge_asymmetry.append(0)
                self.mc_charge_asymmetry_weights.append(0)

    def terminate(self):
        """Receive signal at the end of event processing"""
        name = self.validation_name
        contact = self.contact

        # Overall figures of merit #
        ############################

        mc_matched_primaries = np.logical_and(self.mc_primaries, self.mc_matches)

        charge_asymmetry = np.average(self.mc_charge_asymmetry, weights=self.mc_charge_asymmetry_weights)
        if len(mc_matched_primaries) > 0 and sum(mc_matched_primaries) > 0:
            charge_efficiency = np.average(self.mc_charge_matches, weights=mc_matched_primaries)
            hit_efficiency = np.average(self.mc_hit_efficiencies, weights=mc_matched_primaries)
        else:
            charge_efficiency = float('nan')
            hit_efficiency = float('nan')
        finding_charge_efficiency = np.average(self.mc_charge_matches, weights=self.mc_primaries)
        finding_efficiency = np.average(self.mc_matches, weights=self.mc_primaries)
        fake_rate = 1.0 - np.mean(self.pr_clones_and_matches)
        # can only be computed if there are entries
        if len(self.pr_clones_and_matches) > 0 and sum(self.pr_clones_and_matches) > 0:
            clone_rate = 1.0 - np.average(self.pr_matches,
                                          weights=self.pr_clones_and_matches)
        else:
            clone_rate = float('nan')

        figures_of_merit = ValidationFiguresOfMerit(f'{name}_figures_of_merit')
        figures_of_merit['finding_charge_efficiency'] = finding_charge_efficiency
        figures_of_merit['finding_efficiency'] = finding_efficiency
        figures_of_merit['charge_efficiency'] = charge_efficiency
        figures_of_merit['charge_asymmetry'] = charge_asymmetry
        figures_of_merit['fake_rate'] = fake_rate
        figures_of_merit['clone_rate'] = clone_rate
        figures_of_merit['hit_efficiency'] = hit_efficiency

        figures_of_merit.description = \
            """
finding_efficiency - the ratio of matched Monte Carlo tracks to all primary Monte Carlo tracks <br/>
charge_efficiency - the ratio of matched Monte Carlo tracks with correct charge to matched primary Monte Carlo tracks <br/>
finding_charge_efficiency - the ratio of matched Monte Carlo tracks with correct charge to all primary Monte Carlo tracks <br/>
fake_rate - ratio of pattern recognition tracks that are not related to a particle
            (background, ghost) to all pattern recognition tracks <br/>
clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches) <br/>

"""
        figures_of_merit.check = 'Compare for degradations with respect to the reference'
        figures_of_merit.contact = contact
        print(figures_of_merit)

        # Validation plots #
        ####################
        validation_plots = []
        pull_analyses = []

        # Finding efficiency #
        ######################
        plots = self.profiles_by_mc_parameters(self.mc_matches,
                                               'finding efficiency',
                                               make_hist=False,
                                               weights=self.mc_primaries)

        validation_plots.extend(plots)

        # Fake rate (all tracks not matched or clone            #
        # use TrackCand seeds for the fake track plotting       #
        # as the fit (if successful) is probably not meaningful #
        #########################################################
        print('fake list: ' + str(self.pr_fakes.count(1)))
        plots = self.profiles_by_pr_parameters(self.pr_fakes, 'fake rate',
                                               make_hist=False)

        validation_plots.extend(plots)

        # Charge efficiency of matched primary tracks #
        #######################################
        plots = self.profiles_by_mc_parameters(self.mc_charge_matches,
                                               'charge efficiency for matched primary tracks',
                                               weights=mc_matched_primaries)

        validation_plots.extend(plots)

        # Finding & Charge efficiency of primary tracks #
        #######################################
        plots = self.profiles_by_mc_parameters(self.mc_charge_matches,
                                               'finding and charge efficiency for primary tracks',
                                               weights=self.mc_primaries)

        validation_plots.extend(plots)

        # Charge asymmetry of primary tracks #
        #######################################
        plots = self.profiles_by_mc_parameters(self.mc_charge_asymmetry,
                                               'charge asymmetry for primary tracks',
                                               weights=self.mc_charge_asymmetry_weights,
                                               is_asymmetry=True)

        validation_plots.extend(plots)

        # Hit efficiency #
        ##################
        plots = self.profiles_by_mc_parameters(self.mc_hit_efficiencies,
                                               'hit efficiency with matched tracks',
                                               weights=mc_matched_primaries)

        validation_plots.extend(plots)

        # Fit quality #
        ###############
        if self.pulls:
            all_but_diagonal_plots = list(PullAnalysis.default_which_plots)
            all_but_diagonal_plots.remove("diag_profile")
            all_but_diagonal_plots.remove("diag_scatter")

            plot_name_prefix = name + self.plot_name_postfix
            if not self.fit:
                plot_name_prefix += '_seed'

            # Omega / curvature pull
            pr_omega_truths = np.array(self.pr_omega_truths)
            pr_omega_estimates = np.array(self.pr_omega_estimates)
            pr_omega_variances = np.array(self.pr_omega_variances)

            curvature_pull_analysis = PullAnalysis('#omega', unit='1/cm',
                                                   plot_name_prefix=plot_name_prefix + '_omega',
                                                   plot_title_postfix=self.plot_title_postfix,
                                                   referenceFileName=self.referenceFileName)

            curvature_pull_analysis.analyse(pr_omega_truths,
                                            pr_omega_estimates,
                                            pr_omega_variances,
                                            which_plots=all_but_diagonal_plots)

            curvature_pull_analysis.contact = contact
            pull_analyses.append(curvature_pull_analysis)

            # Tan lambda pull
            pr_tan_lambda_truths = np.array(self.pr_tan_lambda_truths)
            pr_tan_lambda_estimates = np.array(self.pr_tan_lambda_estimates)
            pr_tan_lambda_variances = np.array(self.pr_tan_lambda_variances)

            curvature_pull_analysis = PullAnalysis('tan #lambda',
                                                   plot_name_prefix=plot_name_prefix + '_tan_lambda',
                                                   plot_title_postfix=self.plot_title_postfix,
                                                   referenceFileName=self.referenceFileName)

            curvature_pull_analysis.analyse(pr_tan_lambda_truths,
                                            pr_tan_lambda_estimates,
                                            pr_tan_lambda_variances,
                                            which_plots=all_but_diagonal_plots)

            curvature_pull_analysis.contact = contact
            pull_analyses.append(curvature_pull_analysis)

            # d0 pull
            curvature_pull_analysis = PullAnalysis('d0',
                                                   plot_name_prefix=plot_name_prefix + '_d0',
                                                   plot_title_postfix=self.plot_title_postfix,
                                                   referenceFileName=self.referenceFileName)

            curvature_pull_analysis.analyse(np.array(self.pr_d0_truths),
                                            np.array(self.pr_d0_estimates),
                                            np.array(self.pr_d0_variances),
                                            which_plots=all_but_diagonal_plots)

            curvature_pull_analysis.contact = contact
            pull_analyses.append(curvature_pull_analysis)

        # Resolution plots
        ##############################
        if self.resolution:
            # d0 impact parameter resolution plot
            d0_resolution_analysis = ResolutionAnalysis('d0_res',
                                                        self.resolution_pt_binning,
                                                        'Pt',
                                                        plot_name_prefix=plot_name_prefix + '_d0_res',
                                                        plot_title_postfix=self.plot_title_postfix,
                                                        referenceFileName=self.referenceFileName)
            d0_resolution_analysis.analyse(np.array(self.pr_bining_pt),
                                           np.array(self.pr_d0_truths),
                                           np.array(self.pr_d0_estimates))
            d0_resolution_analysis.contact = contact
            pull_analyses.append(d0_resolution_analysis)

            # z0 impact parameter resolution plot
            z0_resolution_analysis = ResolutionAnalysis('z0_res',
                                                        self.resolution_pt_binning,
                                                        "Pt",
                                                        plot_name_prefix=plot_name_prefix + '_z0_res',
                                                        plot_title_postfix=self.plot_title_postfix,
                                                        referenceFileName=self.referenceFileName)
            z0_resolution_analysis.analyse(np.array(self.pr_bining_pt),
                                           np.array(self.pr_z0_truths),
                                           np.array(self.pr_z0_estimates))
            z0_resolution_analysis.contact = contact
            pull_analyses.append(z0_resolution_analysis)

            # omega curvature parameter resolution plot
            omega_resolution_analysis = ResolutionAnalysis('omega_res',
                                                           self.resolution_pt_binning,
                                                           "Pt",
                                                           plot_name_prefix=plot_name_prefix + '_omega_res',
                                                           plot_title_postfix=self.plot_title_postfix,
                                                           referenceFileName=self.referenceFileName)
            omega_resolution_analysis.analyse(np.array(self.pr_bining_pt),
                                              np.array(self.pr_omega_truths),
                                              np.array(self.pr_omega_estimates))
            omega_resolution_analysis.contact = contact
            pull_analyses.append(omega_resolution_analysis)

            # transverse momentum resolution plot
            pt_resolution_analysis = ResolutionAnalysis('pt_res',
                                                        self.resolution_pt_binning,
                                                        "Pt",
                                                        plot_name_prefix=plot_name_prefix + '_pt_res',
                                                        plot_title_postfix=self.plot_title_postfix,
                                                        referenceFileName=self.referenceFileName)
            pt_resolution_analysis.analyse(np.array(self.pr_bining_pt),
                                           np.array(self.pr_pt_truths),
                                           np.array(self.pr_pt_estimates))
            pt_resolution_analysis.contact = contact
            pull_analyses.append(pt_resolution_analysis)

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
            'ndf',
        ],
        make_hist=True,
        weights=None,
        is_asymmetry=False,
    ):
        """Create profile histograms by MC-track parameters"""

        # apply exclusion list
        new_parameter_names = [item for item in parameter_names if item
                               not in self.exclude_profile_mc_parameter]

        # Profile versus the various parameters
        profile_parameters = {
            'd_{0}': self.mc_d0s,
            'p_{t}': self.mc_pts,
            'tan #lambda': self.mc_tan_lambdas,
            '#phi': self.mc_phi,
            'multiplicity': self.mc_multiplicities,
            'ndf': self.mc_ndf,
        }

        return self.profiles_by_parameters_base(
            xs,
            quantity_name,
            new_parameter_names,
            profile_parameters,
            unit,
            make_hist,
            weights=weights,
            is_asymmetry=is_asymmetry
        )

    def profiles_by_pr_parameters(
        self,
        xs,
        quantity_name,
        unit=None,
        parameter_names=['Seed_p_t', 'Seed tan #lambda', 'Seed #phi'],
        make_hist=True,
    ):
        """Create profile histograms by PR-track parameters"""

        # apply exclusion list
        new_parameter_names = [item for item in parameter_names if item
                               not in self.exclude_profile_pr_parameter]

        # Profile versus the various parameters
        profile_parameters = {'Seed p_{t}': self.pr_seed_pt,
                              'Seed tan #lambda': self.pr_seed_tan_lambdas,
                              'Seed #phi': self.pr_seed_phi}

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
        weights=None,
        is_asymmetry=False,
    ):
        """Create profile histograms for generic parameters"""

        contact = self.contact

        validation_plots = []
        plot_name_prefix = self.validation_name + '_' + root_save_name(quantity_name) \
            + self.plot_name_postfix

        if make_hist:
            # Histogram of the quantity
            histogram = ValidationPlot(plot_name_prefix, self.referenceFileName)
            histogram.hist(xs, weights=weights)

            histogram.xlabel = quantity_name
            histogram.description = 'Not a serious plot yet.'
            histogram.check = ''
            histogram.contact = contact
            histogram.title = quantity_name + self.plot_title_postfix

            validation_plots.append(histogram)

        for (parameter_name, parameter_values) in list(profile_parameters.items()):
            if parameter_name in parameter_names \
                    or root_save_name(parameter_name) in parameter_names:

                is_expert = not(parameter_name in self.non_expert_parameters)

                parameter_root_name = root_save_name(parameter_name)

                # Apply some boundaries for the maximal tracking acceptance
                # such that the plots look more instructive
                if 'tan_lambda' in parameter_root_name:
                    lower_bound = -2.0
                    upper_bound = 5.0
                    # need different bounds for cosmics
                    if 'cosmics' in self.validation_name.lower() or \
                       'cosmics' in self.output_file_name.lower():
                        lower_bound = -1.5
                        upper_bound = 1.5
                elif 'ndf' in parameter_root_name:
                    lower_bound = 0
                    upper_bound = min(200, np.max(parameter_values))
                elif 'p_t' in parameter_root_name:
                    lower_bound = 0
                    upper_bound = 2.5
                    # need different upper_bound for cosmics
                    if 'cosmics' in self.validation_name.lower() or \
                       'cosmics' in self.output_file_name.lower():
                        upper_bound = 30
                elif 'd_0' in parameter_root_name:
                    lower_bound = -0.06
                    upper_bound = 0.06
                    # need different bounds for cosmics
                    if 'cosmics' in self.validation_name.lower() or \
                       'cosmics' in self.output_file_name.lower():
                        lower_bound = -20
                        upper_bound = 20
                else:
                    lower_bound = None
                    upper_bound = None

                profile_plot_name = plot_name_prefix + '_by_' \
                    + root_save_name(parameter_name)
                profile_plot = ValidationPlot(profile_plot_name, self.referenceFileName)
                profile_plot.profile(parameter_values,
                                     xs,
                                     weights=weights,
                                     outlier_z_score=10.0,
                                     lower_bound=lower_bound,
                                     upper_bound=upper_bound,
                                     y_binary=True,
                                     is_expert=is_expert,
                                     is_asymmetry=is_asymmetry)

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
