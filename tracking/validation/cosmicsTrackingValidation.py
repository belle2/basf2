#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <output>cosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'cosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000

import basf2
basf2.set_random_seed(1337)

import argparse

import os
import sys
import logging
import collections

import numpy as np

import simulation
from cdclocaltracking.validation.plot import ValidationPlot

import ROOT
from ROOT import Belle2


def main():
    """Function to be executed during the validation run"""

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

    argument_parser.add_argument('-p', '--pulls', action='store_true',
                                 help='Plot pulls of the curvature.')

    arguments = argument_parser.parse_args()

    # Compose basf2 module path #
    #############################
    main_path = basf2.create_path()

    # Master module
    eventInfoSetterModule = basf2.register_module('EventInfoSetter')
    eventInfoSetterModule.param({'evtNumList': [N_EVENTS], 'runList': [1],
                                'expList': [1]})
    main_path.add_module(eventInfoSetterModule)

    # Progress module
    progressModule = basf2.register_module('Progress')
    main_path.add_module(progressModule)

    # Cosmics generator
    cosmicsModule = basf2.register_module('Cosmics')
    main_path.add_module(cosmicsModule)

    # Simulation
    components = ['MagneticFieldConstant4LimitedRCDC', 'CDC']
    simulation.add_simulation(main_path, components=components)

    # Setup track finder
    trackFinderModule = basf2.register_module('CDCLocalTracking')
    main_path.add_module(trackFinderModule)

    if arguments.fit:
        # Prepare Genfit extrapolation
        setupGenfitExtrapolationModule = \
            basf2.register_module('SetupGenfitExtrapolation')
        setupGenfitExtrapolationModule.param({'whichGeometry': arguments.geometry})
        main_path.add_module(setupGenfitExtrapolationModule)

        # Fit tracks
        genFitterModule = basf2.register_module('GenFitter')
        genFitterModule.param({'PDGCodes': [13]})
        main_path.add_module(genFitterModule)

    # Reference Monte Carlo tracks
    trackFinderMCTruthModule = basf2.register_module('TrackFinderMCTruth')
    trackFinderMCTruthModule.param({
        'UseCDCHits': True,
        'UseSVDHits': False,
        'UsePXDHits': False,
        'WhichParticles': ['primary'],
        'EnergyCut': 0.1,
        'GFTrackCandidatesColName': 'MCTrackCands',
        })
    main_path.add_module(trackFinderMCTruthModule)

    # Track matcher
    mcTrackMatcherModule = basf2.register_module('MCTrackMatcher')
    mcTrackMatcherModule.param({
        'UseCDCHits': True,
        'UseSVDHits': False,
        'UsePXDHits': False,
        'MCGFTrackCandsColName': 'MCTrackCands',
        'MinimalPurity': 0.66,
        'RelateClonesToMCParticles': True,
        })

    main_path.add_module(mcTrackMatcherModule)

    # Validation module generating plots
    trackingValidationModule = TrackingValidationModule('Cosmics',
            fit=arguments.fit, pulls=arguments.pulls)
    main_path.add_module(trackingValidationModule)

    # Run basf2 module path #
    #########################
    print 'Start processing'
    basf2.process(main_path)
    print basf2.statistics


# Analysis module #
###################

class TrackingValidationModule(basf2.Module):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def __init__(
        self,
        name,
        fit=False,
        pulls=False,
        ):

        super(TrackingValidationModule, self).__init__()
        self.name = name
        self.fit = fit
        self.pulls = pulls

    @property
    def output_file_name(self):
        return self.name + 'TrackingValidation.root'

    def initialize(self):
        self.trackMatchLookUp = Belle2.TrackMatchLookUp('MCTrackCands')

        # Use deques in favour of lists to prevent repeated memory allocation of cost O(n)
        self.pr_clones_and_matches = collections.deque()
        self.pr_matches = collections.deque()
        self.pr_seen_in_vxds = collections.deque()
        self.pr_fitted_omegas = collections.deque()
        self.pr_true_omegas = collections.deque()

        self.mc_transverse_momenta = collections.deque()
        self.mc_matches = collections.deque()
        self.mc_d0s = collections.deque()
        self.mc_cos_thetas = collections.deque()
        self.mc_pt = collections.deque()
        self.mc_hit_efficiencies = collections.deque()

    def event(self):
        """Event method"""

        trackMatchLookUp = self.trackMatchLookUp

        # Analyse from the pattern recognition side
        trackCands = Belle2.PyStoreArray('TrackCands')
        if trackCands:
            # print 'Number of pattern recognition tracks', \
            #    trackCands.getEntries()

            for trackCand in trackCands:
                is_matched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
                is_clone = trackMatchLookUp.isClonePRTrackCand(trackCand)

                if self.fit:
                    prTrackFitResult = \
                        trackMatchLookUp.getRelatedTrackFitResult(trackCand)
                else:
                    prTrackFitResult = getSeedTrackFitResult(trackCand)

                fitted_omega = float('nan')
                if prTrackFitResult != None:
                    fitted_omega = prTrackFitResult.getOmega()

                seen_in_vxd = False
                true_omega = float('nan')
                if is_matched or is_clone:
                    # Only matched and clone tracks have a related MCParticle
                    mcParticle = \
                        trackMatchLookUp.getRelatedMCParticle(trackCand)
                    mcTrackFitResult = \
                        getTrackFitResultFromMCParticle(mcParticle)
                    true_omega = mcTrackFitResult.getOmega()

                    if mcParticle:
                        seenInDetector = mcParticle.getSeenInDetector()

                        pxd_detector_set = \
                            Belle2.Const.DetectorSet(Belle2.Const.PXD)
                        svd_detector_set = \
                            Belle2.Const.DetectorSet(Belle2.Const.SVD)

                        seen_in_pxd = seenInDetector.contains(pxd_detector_set)
                        seen_in_svd = seenInDetector.contains(svd_detector_set)
                        seen_in_vxd = seen_in_pxd | seen_in_vxd

                self.pr_clones_and_matches.append(is_matched or is_clone)
                self.pr_matches.append(is_matched)
                self.pr_seen_in_vxds.append(seen_in_vxd)

                self.pr_fitted_omegas.append(fitted_omega)
                self.pr_true_omegas.append(true_omega)

        # Analyse from the Monte carlo reference side
        mcTrackCands = Belle2.PyStoreArray('MCTrackCands')
        if trackCands:
            # print 'Number of Monte Carlo tracks', mcTrackCands.getEntries()

            for mcTrackCand in mcTrackCands:
                is_matched = trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand)

                hit_efficiency = \
                    trackMatchLookUp.getRelatedEfficiency(mcTrackCand)
                if np.isnan(hit_efficiency):
                    hit_efficiency = 0

                mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
                mcTrackFitResult = getTrackFitResultFromMCParticle(mcParticle)

                momentum = mcParticle.getMomentum()
                transverse_momentum = momentum.Perp()
                cos_theta = momentum.CosTheta()
                d0 = mcTrackFitResult.getD0()

                self.mc_matches.append(is_matched)
                self.mc_hit_efficiencies.append(hit_efficiency)
                self.mc_transverse_momenta.append(transverse_momentum)
                self.mc_d0s.append(d0)
                self.mc_cos_thetas.append(cos_theta)

    def terminate(self):
        # Overall figures of merit #
        ############################
        track_finding_efficiency = np.mean(self.mc_matches)
        fake_rate = 1.0 - np.mean(self.pr_clones_and_matches)
        clone_rate = 1.0 - np.average(self.pr_matches,
                                      weights=self.pr_clones_and_matches)
        hit_efficiency = np.mean(self.mc_hit_efficiencies)

        cosmics_figures_of_merit = \
            ValidationFiguresOfMerit('%s_figures_of_merit' % self.name)
        cosmics_figures_of_merit['finding_efficiency'] = \
            track_finding_efficiency
        cosmics_figures_of_merit['fake_rate'] = fake_rate
        cosmics_figures_of_merit['clone_rate'] = clone_rate
        cosmics_figures_of_merit['hit_efficiency'] = hit_efficiency

        cosmics_figures_of_merit.description = \
            """
finding_efficiency - the ratio of matched Monte Carlo tracks to all Monte Carlo tracks <br/>
fake_rate - ratio of pattern recognition tracks that are not related to a particle (background, ghost) to all pattern recognition tracks <br/>
clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches) <br/>
"""
        cosmics_figures_of_merit.check = ''
        cosmics_figures_of_merit.contact = CONTACT

        print cosmics_figures_of_merit

        # Validation plots #
        ####################
        validation_plots = []

        # Track finding efficiency over the true transverse momentum profile
        finding_efficiency_by_pt = ValidationPlot('%s_finding_efficiency_by_pt'
                 % self.name)

        finding_efficiency_by_pt.profile(self.mc_transverse_momenta,
                self.mc_matches, bins=50)

        finding_efficiency_by_pt.xlabel = 'Momentum (GeV)'
        finding_efficiency_by_pt.ylabel = 'Efficiency'
        finding_efficiency_by_pt.description = 'Not a serious plot yet.'
        finding_efficiency_by_pt.check = ''
        finding_efficiency_by_pt.contact = CONTACT

        validation_plots.append(finding_efficiency_by_pt)

        # A tryout d0 parameter plot, yet using the seen in vxd as an d0 parameter.
        clone_rate_by_d0 = ValidationPlot('%s_clone_rate_by_seen_in_vxd'
                % self.name)

        pr_matches = np.array(self.pr_matches)
        clone_rate_by_d0.fill(self.pr_seen_in_vxds, ~pr_matches,
                              weights=self.pr_clones_and_matches)

        clone_rate_by_d0.xlabel = 'Seen in vxd (bool)'
        clone_rate_by_d0.ylabel = 'Clone rate'

        clone_rate_by_d0.description = 'Not a serious plot yet.'
        clone_rate_by_d0.check = ''
        clone_rate_by_d0.contact = CONTACT
        # validation_plots.append(clone_rate_by_d0)

        # Finding efficiency by d0 parameter
        finding_efficiency_by_d0 = ValidationPlot('%s_finding_efficiency_by_d0'
                 % self.name)
        finding_efficiency_by_d0.fill(self.mc_d0s, self.mc_matches, bins=50)

        finding_efficiency_by_d0.xlabel = 'D0 (cm)'
        finding_efficiency_by_d0.ylabel = 'Finding efficiency'

        finding_efficiency_by_d0.description = 'Not a serious plot yet.'
        finding_efficiency_by_d0.check = ''
        finding_efficiency_by_d0.contact = CONTACT

        validation_plots.append(finding_efficiency_by_d0)

        # Finding efficiency by cos theta
        finding_efficiency_by_cos_theta = \
            ValidationPlot('%s_finding_efficiency_by_cos_theta' % self.name)

        finding_efficiency_by_cos_theta.fill(self.mc_cos_thetas,
                self.mc_matches, bins=50)

        finding_efficiency_by_cos_theta.xlabel = 'cos #theta'
        finding_efficiency_by_cos_theta.ylabel = 'Finding efficiency'

        finding_efficiency_by_cos_theta.description = 'Not a serious plot yet.'
        finding_efficiency_by_cos_theta.check = ''
        finding_efficiency_by_cos_theta.contact = CONTACT

        validation_plots.append(finding_efficiency_by_cos_theta)

        # Histogram of the hit efficiency
        hit_efficiency_histogram = ValidationPlot('%s_hit_efficiency'
                % self.name)
        hit_efficiency_histogram.fill(self.mc_hit_efficiencies, bins=50)

        hit_efficiency_histogram.xlabel = 'Hit efficiency'
        hit_efficiency_histogram.description = 'Not a serious plot yet.'
        hit_efficiency_histogram.check = ''
        hit_efficiency_histogram.contact = CONTACT

        validation_plots.append(hit_efficiency_histogram)

        # Hit efficiency by d0 parameter
        hit_efficiency_by_d0 = ValidationPlot('%s_hit_efficiency_by_d0'
                % self.name)

        hit_efficiency_by_d0.fill(self.mc_d0s, self.mc_hit_efficiencies,
                                  bins=50)

        hit_efficiency_by_d0.xlabel = 'D0 (cm)'
        hit_efficiency_by_d0.ylabel = 'Hit efficiency'

        hit_efficiency_by_d0.description = 'Not a serious plot yet.'
        hit_efficiency_by_d0.check = ''
        hit_efficiency_by_d0.contact = CONTACT

        validation_plots.append(hit_efficiency_by_d0)

        # Hit efficiency by cos_theta parameter
        hit_efficiency_by_cos_theta = \
            ValidationPlot('%s_hit_efficiency_by_cos_theta' % self.name)

        hit_efficiency_by_cos_theta.fill(self.mc_cos_thetas,
                self.mc_hit_efficiencies, bins=50)

        hit_efficiency_by_cos_theta.xlabel = 'cos #theta'
        hit_efficiency_by_cos_theta.ylabel = 'Hit efficiency'

        hit_efficiency_by_cos_theta.description = 'Not a serious plot yet.'
        hit_efficiency_by_cos_theta.check = ''
        hit_efficiency_by_cos_theta.contact = CONTACT

        validation_plots.append(hit_efficiency_by_cos_theta)

        # Omega / curvature residuals
        if self.pulls:
            pr_fitted_omegas = np.array(self.pr_fitted_omegas)
            pr_failed_fits = np.isnan(pr_fitted_omegas)
            pr_true_omegas = np.array(self.pr_true_omegas)
            pr_no_true_omega = np.isnan(pr_true_omegas)
            pr_omega_residuals = pr_fitted_omegas - pr_true_omegas

            # print 'Number of failed fits', np.sum(pr_failed_fits)
            # import matplotlib.pyplot as plt
            # plt.figure()
            # plt.hist(pr_fitted_omegas[~pr_failed_fits])
            # plt.show()

            # print 'Number of no true omegas', np.sum(pr_no_true_omega)
            # plt.figure()
            # plt.hist(pr_true_omegas[~pr_no_true_omega])
            # plt.show()

            # Truths
            true_omegas_histogram = ValidationPlot('%s_true_omegas'
                    % self.name)
            true_omegas_histogram.fill(pr_true_omegas[~pr_no_true_omega],
                                       bins=50)

            true_omegas_histogram.xlabel = 'True omega (1/cm)'

            true_omegas_histogram.description = \
                'True omega value of the track. Not a serious plot yet.'
            true_omegas_histogram.check = ''
            true_omegas_histogram.contact = CONTACT

            validation_plots.append(true_omegas_histogram)

            # Fitted
            fitted_omegas_histogram = ValidationPlot('%s_fitted_omegas'
                    % self.name)
            fitted_omegas_histogram.fill(pr_fitted_omegas[~pr_failed_fits],
                    bins=50)

            fitted_omegas_histogram.xlabel = 'Fitted omega (1/cm)'

            fitted_omegas_histogram.description = \
                'Fitted omega value, for now it is the seed value of the TrackCand, which has not enough sensitivity as is shown below. Not a serious plot yet.'
            fitted_omegas_histogram.check = ''
            fitted_omegas_histogram.contact = CONTACT

            validation_plots.append(fitted_omegas_histogram)

            # Residuals
            omega_residuals_histogram = ValidationPlot('%s_omega_residuals'
                    % self.name)
            omega_residuals_histogram.fill(pr_omega_residuals[~pr_failed_fits
                    & ~pr_no_true_omega], bins=50)

            omega_residuals_histogram.xlabel = 'Omega residuals (1/cm)'

            omega_residuals_histogram.description = 'Not a serious plot yet.'
            omega_residuals_histogram.check = ''
            omega_residuals_histogram.contact = CONTACT

            validation_plots.append(omega_residuals_histogram)

            # Diagonal plot
            fitted_omegas_by_true_omegas = \
                ValidationPlot('%s_fitted_omegas_by_true_omegas' % self.name)
            fitted_omegas_by_true_omegas.fill(pr_true_omegas[~pr_failed_fits
                    & ~pr_no_true_omega], pr_fitted_omegas[~pr_failed_fits
                    & ~pr_no_true_omega], bins=50)

            fitted_omegas_by_true_omegas.xlabel = 'True omega (1/cm)'
            fitted_omegas_by_true_omegas.ylabel = 'Fitted omega (1/cm)'

            fitted_omegas_by_true_omegas.description = \
                'Displays the seed values of the TrackCand. Sensitivity for Not a serious plot yet.'
            fitted_omegas_by_true_omegas.check = 'Should be a diagonal plot'
            fitted_omegas_by_true_omegas.contact = CONTACT

            validation_plots.append(fitted_omegas_by_true_omegas)

        # Save everything to a ROOT file
        output_file = ROOT.TFile(self.output_file_name, 'recreate')

        cosmics_figures_of_merit.write()

        for validation_plot in validation_plots:
            validation_plot.write()

        output_file.Close()


def getTrackFitResultFromMCParticle(mcParticle):
    position = mcParticle.getVertex()
    momentum = mcParticle.getMomentum()
    cartesian_covariance = ROOT.TMatrixDSym(6)
    cartesian_covariance.UnitMatrix()
    charge_sign = (-1 if mcParticle.getCharge() < 0 else 1)
    particle_type = Belle2.Const.ParticleType(mcParticle.getPDG())
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


# Standardized generation of validation histograms making the actual validation code more readable

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

