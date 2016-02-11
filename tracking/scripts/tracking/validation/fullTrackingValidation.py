#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>FullTrackingValidation.root</output>
  <description>This module validates that full track finding and fitting with particle gun muon tracks of varying pt.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'FullTrackingValidation.root'
N_EVENTS = 2000
ACTIVE = False

import basf2
# necessary for the logging to work properly
from basf2 import *
basf2.set_random_seed(1337)


from tracking.validation.run import TrackingValidationRun
from tracking.validation.module import TrackingValidationModule

from tracking.validation.mc_side_module import MCSideTrackingValidationModule
from tracking.validation.pr_side_module import PRSideTrackingValidationModule
from tracking.validation.module import SeparatedTrackingValidationModule
import tracking.validation.refiners as refiners

import ROOT
import logging
import numpy as np

# In preparation for the switch to the separated modules for Monte Carlo and pattern recognition side validation:

# the discrete pt values in GeV for which to generate particles using the
# particle gun
pt_values = [0.1, 0.25, 0.4, 0.6, 1., 1.5, 2., 3., 4.]


pt_bin_edges = [(low + high) / 2.0 for (low, high) in zip(pt_values[:-1], pt_values[1:])]


def getPtBinsWithWeight(pt_values):
    # each entry gets the same weight
    pt_weights = [1. / len(pt_values)] * len(pt_values)
    # weights are in the second half of the array going to ParticleGun
    return pt_values + pt_weights


class PtBinnedMCSideTrackingValidationModule(MCSideTrackingValidationModule):

    def peel(self, mc_track_cand):
        crops = super(PtBinnedMCSideTrackingValidationModule, self).peel(mc_track_cand)
        # To few values in this setup to be relevant.
        del crops["d0_truth"]
        del crops["multiplicity"]

        # Add pt value that was used by the generator to produce this track
        mc_pt = crops["pt_truth"]

        # Search the pt value which was used for the MCParticle
        # counters potential rounding errors
        pt_bin_id, = np.digitize((mc_pt,), pt_bin_edges)
        crops["pt_value"] = pt_values[pt_bin_id]

        return crops

    # TODO: Add some sort of groupby pt_value plots that we actually want
    # Refiners to be executed on terminate #
    # #################################### #

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_finding_efficiency_profiles = {
        'is_matched': 'finding efficiency',
        'tan_lambda_truth': 'tan #lambda',
        'phi0_truth': '#phi_0',
    }

    save_binned_finding_efficiency_profiles = refiners.save_profiles(
        select=renaming_select_for_finding_efficiency_profiles,
        groupby="pt_value",
        y='finding efficiency',
        outlier_z_score=5.0,
        allow_discrete=True,
        name="{module.id}_{y_part_name}_by_{x_part_name}_profile_PtBin_{groupby_value}",
        title="Profile of {y_part_name} by {x_part_name} in {module.title} Pt = {groupby_value} GeV",
        # folder_name=".", #Dummy everything in the top level folder
    )


class PtBinnedPRSideTrackingValidationModule(PRSideTrackingValidationModule):

    def peel(self, track_cand):
        crops = super(PtBinnedPRSideTrackingValidationModule, self).peel(track_cand)
        mc_pt = crops["pt_truth"]

        if np.isfinite(mc_pt):
            # Search the pt value which was used for the MCParticle
            # counters potential rounding errors
            pt_bin_id, = np.digitize((mc_pt,), pt_bin_edges)
            crops["pt_value"] = pt_values[pt_bin_id]

        else:
            crops["pt_value"] = float("nan")

        return crops

    # TODO: Add some sort of groupby pt_value plots that we actually want
    # Refiners to be executed on terminate #
    # #################################### #

    save_binned_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        quantity_name="tan #lambda",
        groupby="pt_value",
        part_name="tan_lambda",
        name="{module.id}_{quantity_name}_PtBin_{groupby_value}",
        title_postfix=" Pt = {groupby_value} GeV",
        # folder_name=".", #Dummy everything in the top level folder
    )


class PtBinnedTrackingValidationModule(SeparatedTrackingValidationModule):
    MCSideModule = PtBinnedMCSideTrackingValidationModule
    PRSideModule = PtBinnedPRSideTrackingValidationModule


class PtBinnedFullTracking(TrackingValidationRun):
    n_events = N_EVENTS
    generator_module = 'EvtGenInput'
    finder_module = 'StandardReco'
    fit_geometry = 'TGeo'
    generator_module = basf2.register_module('ParticleGun')
    generator_module.param({
        'pdgCodes': [13, -13],
        'nTracks': 10,
        'varyNTracks': 0,
        'momentumGeneration': 'discretePt',
        'momentumParams': getPtBinsWithWeight(pt_values),
        'vertexGeneration': 'fixed',
        'xVertexParams': [0.0],
        'yVertexParams': [0.0],
        'zVertexParams': [0.0],
        'thetaGeneration': 'uniformCos',
    })

    output_file_name = VALIDATION_OUTPUT_FILE

    def preparePathValidation(self, main_path):
        validation_module = PtBinnedTrackingValidationModule(
            name=self.name,
            contact=self.contact,
            output_file_name=self.output_file_name,
            expert_level=0  # No expert subfolder desired.
        )

        main_path.add_module(validation_module)


def new_main():
    validation_run = PtBinnedFullTracking()
    validation_run.configure_and_execute_from_commandline()

# End of preparation

# computes bins to have one bin containing
# exactly one of the discrete values passed


def computeBins(discreteValues):
    if len(discreteValues) < 2:
        B2FATAL('discreteVaules list must at least contain two items')
    discreteValues.sort()
    # add leading item which has the same distance as entry 0 to 1
    discreteValues = [2 * discreteValues[0] - discreteValues[1]] \
        + discreteValues
    # add trailing item which has the same distance as entry n-1 to n
    discreteValues = discreteValues + [2 * discreteValues[-1] - discreteValues[-2]]

    bin_borders = []
    # loop over the discrete entries and compute the bin borders
    # exactly between each entry
    for i in range(0, len(discreteValues) - 2):
        avg_low = (discreteValues[i] + discreteValues[i + 1]) / 2.
        avg_high = (discreteValues[i + 1] + discreteValues[i + 2]) / 2.
        bin_borders = bin_borders + [(avg_low, avg_high, discreteValues[i + 1])]

    return bin_borders


class FilterMcParticleInPtBin(object):

    def __init__(self, ptBin):
        self.ptBin = ptBin

    def doesPrPass(self, filterProperties):
        return self.doesPass(filterProperties)

    def doesMcPass(self, filterProperties):
        return self.doesPass(filterProperties)

    # due to the specific ParticleGun setup of this validation run,
    # all tracks generated in one event have the exact same Pt
    # therefore, a cut on the first entry can be applied
    def doesPass(self, filterProperties):
        # todo: check the length of the PyStoreArray to be >0
        pt = filterProperties.mcParticles[0].getMomentum().Pt()
        doesPass = pt > self.ptBin[0] and pt < self.ptBin[1]
        return doesPass


# Specialized version of the tracking validation
# which creates seperate root files with dedicated plots
# for pt bins

class FullTracking(TrackingValidationRun):

    n_events = N_EVENTS
    generator_module = 'EvtGenInput'
    finder_module = 'StandardReco'
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE

    def __init__(self, ptBinBorders):
        super(FullTracking, self).__init__()
        self.ptBinBorders = ptBinBorders

    def preparePathValidation(self, main_path):

        output_file_name_noExt, ext = os.path.splitext(self.output_file_name)

        # common exclude list for this test scenario
        exclude_mc = ['d_0', 'multiplicity']

        # add plotting over the whole pt spectrum
        fit = bool(self.fit_geometry)
        trackingValidationModule = TrackingValidationModule(
            self.name,
            contact=self.contact,
            fit=fit,
            pulls=self.pulls,
            exclude_profile_mc_parameter=exclude_mc,
            output_file_name=self.output_file_name,
        )
        main_path.add_module(trackingValidationModule)

        for ptBin in self.ptBinBorders:
            # create a special file name for this ptbin, using the discrete pt value configured
            bin_file_name = output_file_name_noExt + str(ptBin[2]) + ext
            postfix = 'PtBin_' + str(ptBin[2])
            title_postfix = ' Pt = ' + str(ptBin[2]) + ' GeV'
            passObj = FilterMcParticleInPtBin(ptBin)
            trackingValidationModule = TrackingValidationModule(
                self.name,
                contact=self.contact,
                fit=fit,
                pulls=self.pulls,
                output_file_name=bin_file_name,
                track_filter_object=passObj,
                plot_name_postfix=postfix,
                plot_title_postfix=title_postfix,
                exclude_profile_mc_parameter=exclude_mc + ['p_t'],
                use_expert_folder=False,
            )
            main_path.add_module(trackingValidationModule)


def getPtBinsWithWeight(pt_values):
    # each entry gets the same weight
    pt_weights = [1. / len(pt_values)] * len(pt_values)
    # weights are in the second half of the array going to ParticleGun
    return pt_values + pt_weights


def main():
    # Configure particle gun to produce with a discrete list of pt values #
    #######################################################################

    # the discrete pt values in GeV for which to generate particles using the
    # particle gun
    ptDiscrete = [0.1, 0.25, 0.4, 0.6, 1., 1.5, 2., 3., 4.]
    ptBinBorders = computeBins(ptDiscrete)
    validation_run = FullTracking(ptBinBorders=ptBinBorders)

    pt_bins_weight = getPtBinsWithWeight(ptDiscrete)
    validation_run.generator_module = basf2.register_module('ParticleGun')
    validation_run.generator_module.param({
        'pdgCodes': [13, -13],
        'nTracks': 10,
        'varyNTracks': 0,
        'momentumGeneration': 'discretePt',
        'momentumParams': pt_bins_weight,
        'vertexGeneration': 'fixed',
        'xVertexParams': [0.0],
        'yVertexParams': [0.0],
        'zVertexParams': [0.0],
        'thetaGeneration': 'uniformCos',
    })

    # Execute the validation run #
    ##############################
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        new_main()
