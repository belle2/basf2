#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <output>cosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000

TRACKING_MAILING_LIST = 'tracking@belle2.kek.jp'

import basf2
basf2.set_random_seed(1337)

import argparse

import os
import sys
import logging
import collections

import numpy as np
import math

import simulation
from tracking.validation.plot import ValidationPlot, root_save_name, \
    compose_axis_label
from tracking.validation.pull import PullAnalysis

import ROOT
from ROOT import Belle2


def main():
    argument_parser = argparse.ArgumentParser()

    # Indication if tracks should be fitted.
    # Currently tracks are not fitted because of a segmentation fault related TGeo / an assertation error in Geant4 geometry.
    argument_parser.add_argument('-f', '--fit', action='store_true',
                                 help='Perform fitting of the generated tracks with Genfit. Default is not to perform a fit but use the seed values generated in track finding.'
                                 )

    # Geometry name to be used in the Genfit extrapolation.
    argument_parser.add_argument('-g', '--geometry', choices=['TGeo', 'Geant4'
                                 ], default='Geant4',
                                 help='Geometry to be used with Genfit.')

    argument_parser.add_argument('-s', '--show', action='store_true',
                                 help='Show generated plots in a TBrowser immediatly.'
                                 )

    arguments = argument_parser.parse_args()

    cosmics_validation_run = Cosmics()
    if arguments.fit:
        cosmics_validation_run.fit_geometry = arguments.geometry
    else:
        cosmics_validation_run.fit_geometry = None

    cosmics_validation_run.execute()

    # Show plots #
    ##############
    if arguments.show:
        output_file_name = cosmics_validation_run.output_file_name
        tFile = ROOT.TFile(output_file_name)
        tBrowser = ROOT.TBrowser()
        tBrowser.BrowseObject(tFile)
        tBrowser.Show()
        raw_input('Press enter to close.')
        tFile.Close()


class TrackingValidationRun(object):

    n_events = 10000
    generator_module = 'EvtGenInput'
    components = ['PXD', 'SVD', 'CDC', 'BeamPipe',
                  'MagneticFieldConstant4LimitedRCDC']
    finder_module = None  # To be specified
    tracking_coverage = {'UsePXDHits': True, 'UseSVDHits': True,
                         'UseCDCHits': True}
    fit_geometry = None
    pulls = True
    contact = TRACKING_MAILING_LIST
    output_file_name = 'StandardTrackingValidation.root'

    # Allow override from instances only in these field names to prevent some spelling errors
    __slots__ = [
        'n_events',
        'generator_module',
        'components',
        'finder_module',
        'tracking_coverage',
        'fit_geometry',
        'contract',
        'output_file_name',
        ]

    @property
    def name(self):
        return self.__class__.__name__

    def execute(self):
        # Compose basf2 module path #
        #############################
        main_path = basf2.create_path()

        # Master module
        eventInfoSetterModule = basf2.register_module('EventInfoSetter')
        eventInfoSetterModule.param({'evtNumList': [self.n_events],
                                    'runList': [1], 'expList': [1]})
        main_path.add_module(eventInfoSetterModule)

        # Progress module
        progressModule = basf2.register_module('Progress')
        main_path.add_module(progressModule)

        # Generator
        generatorModule = get_basf2_module(self.generator_module)
        main_path.add_module(generatorModule)

        # Simulation
        components = self.components
        simulation.add_simulation(main_path, components=components)

        # Setup track finder
        trackFinderModule = get_basf2_module(self.finder_module)
        main_path.add_module(trackFinderModule)

        if self.fit_geometry:
            # Prepare Genfit extrapolation
            setupGenfitExtrapolationModule = \
                basf2.register_module('SetupGenfitExtrapolation')
            setupGenfitExtrapolationModule.param({'whichGeometry': self.fit_geometry})
            main_path.add_module(setupGenfitExtrapolationModule)

            # Fit tracks
            genFitterModule = basf2.register_module('GenFitter')
            genFitterModule.param({'PDGCodes': [13]})
            main_path.add_module(genFitterModule)

        # Reference Monte Carlo tracks
        trackFinderMCTruthModule = basf2.register_module('TrackFinderMCTruth')
        trackFinderMCTruthModule.param({'WhichParticles': ['primary'],
                                       'EnergyCut': 0.1,
                                       'GFTrackCandidatesColName': 'MCTrackCands'
                                       })
        trackFinderMCTruthModule.param(self.tracking_coverage)
        main_path.add_module(trackFinderMCTruthModule)

        # Track matcher
        mcTrackMatcherModule = basf2.register_module('MCTrackMatcher')
        mcTrackMatcherModule.param({'MCGFTrackCandsColName': 'MCTrackCands',
                                   'MinimalPurity': 0.66,
                                   'RelateClonesToMCParticles': True})
        mcTrackMatcherModule.param(self.tracking_coverage)
        main_path.add_module(mcTrackMatcherModule)

        # Validation module generating plots
        fit = bool(self.fit_geometry)
        pulls = self.pulls
        contact = self.contact
        output_file_name = self.output_file_name
        trackingValidationModule = TrackingValidationModule(self.name,
                contact=contact, fit=fit, pulls=pulls,
                output_file_name=output_file_name)
        main_path.add_module(trackingValidationModule)

        # Run basf2 module path #
        #########################
        print 'Start processing'
        basf2.process(main_path)
        print basf2.statistics


def get_basf2_module(module_or_module_name):
    if isinstance(module_or_module_name, str):
        module_name = module_or_module_name
        module = basf2.register_module(module_name)
        return module
    elif isinstance(module_or_module_name, basf2.Module):
        module = module_or_module_name
        return module
    else:
        message_template = \
            '%s of type %s is neither a module nor the name of module. Expected str or basf2.Module instance.'
        raise ValueError(message_template % (module_or_module_name,
                         type(module_or_module_name)))


class Cosmics(TrackingValidationRun):

    n_events = N_EVENTS
    generator_module = 'Cosmics'
    components = ['CDC', 'MagneticFieldConstant4LimitedRCDC']
    finder_module = 'CDCLocalTracking'
    tracking_coverage = {'UsePXDHits': False, 'UseSVDHits': False,
                         'UseCDCHits': True}
    fit_geometry = None
    pulls = True
    contact = CONTACT
    output_file_name = 'CosmicsTrackingValidation.root'


# Analysis module #
###################

class TrackingValidationModule(basf2.Module):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def __init__(
        self,
        name,
        contact=TRACKING_MAILING_LIST,
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

        # Overall figures of merit #
        ############################
        track_finding_efficiency = np.mean(self.mc_matches)
        fake_rate = 1.0 - np.mean(self.pr_clones_and_matches)
        clone_rate = 1.0 - np.average(self.pr_matches,
                                      weights=self.pr_clones_and_matches)
        hit_efficiency = np.mean(self.mc_hit_efficiencies)

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
        figures_of_merit.contact = CONTACT
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
            curvature_pull_analysis.contact = CONTACT
            pull_analyses.append(curvature_pull_analysis)

            # Tan lambda pull
            pr_tan_lambda_truths = np.array(self.pr_tan_lambda_truths)
            pr_tan_lambda_estimates = np.array(self.pr_tan_lambda_estimates)
            pr_tan_lambda_variances = np.array(self.pr_tan_lambda_variances)

            curvature_pull_analysis = PullAnalysis('tan #lambda',
                    plot_name_prefix=plot_name_prefix + '_tan_lambda')
            curvature_pull_analysis.analyse(pr_tan_lambda_truths,
                    pr_tan_lambda_estimates, pr_tan_lambda_variances)
            curvature_pull_analysis.contact = CONTACT
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

        validation_plots = []
        plot_name_prefix = self.name + '_' + root_save_name(quantity_name)

        if make_hist:
            # Histogram of the quantity
            histogram = ValidationPlot(plot_name_prefix)
            histogram.hist(xs)

            histogram.xlabel = quantity_name
            histogram.description = 'Not a serious plot yet.'
            histogram.check = ''
            histogram.contact = CONTACT

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
                profile_plot.contact = CONTACT
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


class ValidationFiguresOfMerit(dict):

    def __init__(
        self,
        name,
        description='',
        check='',
        contact='',
        ):

        self.name = name
        self.description = description
        self.check = check
        self.contact = contact

        self.figures_by_name = {}

    def __str__(self):
        """Informal sting output listing the assigned figures of merit."""

        return '\n'.join('%s : %s' % (key, value) for (key, value) in
                         self.figures_by_name.items())

    def write(self):
        """Writes the figures of merit as a TNtuple to the currently open TFile in the format complient with the validation frame work."""

        name = self.name
        figure_names = list(self.figures_by_name.keys())
        values = list(self.figures_by_name.values())

        leaf_specification = ':'.join(figure_names)
        title = ''
        ntuple = ROOT.TNtuple(name, title, leaf_specification)
        ntuple.Fill(*values)

        ntuple.SetAlias('Description', self.description)
        ntuple.SetAlias('Check', self.check)
        ntuple.SetAlias('Contact', self.contact)

        ntuple.Write()

    def __setitem__(self, figure_name, value):
        """Braketed item assignement for figures of merit"""

        self.figures_by_name[figure_name] = value

    def __getitem__(self, figure_name):
        """Braketed item lookup for figures of merit"""

        return self.figures_by_name[figure_name]

    def __delitem__(self, figure_name):
        """Braketed item deletion for figures of merit"""

        del self.figures_by_name[figure_name]

    def __iter__(self):
        """Implements the iter() hook as if it was a dictionary."""

        return iter(self.figures_by_name)

    def __len__(self):
        """Returns the number of figures of merit assigned. Implements the len() hook."""

        return len(self.figures_by_name)


if __name__ == '__main__':
    main()

