import math
import numpy as np

from tracking.validation.utilities import getHelixFromMCParticle, getSeedTrackFitResult

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2


class PRSideTrackingValidationModule(harvesting.HarvestingModule):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def __init__(
            self,
            name,
            contact,
            fit=False,
            output_file_name=None,
            trackCandidatesColumnName="TrackCands",
            expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super(PRSideTrackingValidationModule, self).__init__(foreach=trackCandidatesColumnName,
                                                             name=name,
                                                             contact=contact,
                                                             output_file_name=output_file_name,
                                                             expert_level=expert_level)

        self.fit = fit
        self.trackCandidatesColumnName = trackCandidatesColumnName
        self.mcTrackCandidatesColumnName = "MCTrackCands"

    def initialize(self):
        super(PRSideTrackingValidationModule, self).initialize()
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

    def pick(self, trackCand):
        return True
        trackMatchLookUp = self.trackMatchLookUp

        trackCands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        mcParticles = Belle2.PyStoreArray("MCParticles")

        # fill the FilterProperties will all properties on this track
        # gathered so far
        filterProperties = FilterProperties(trackCand=trackCand,
                                            mcParticle=mcParticle,
                                            mcParticles=mcParticles)

        if self.fit:
            prTrackFitResult = trackMatchLookUp.getRelatedTrackFitResult(trackCand)
            filterProperties.wasFitted = True
            filterProperties.fitResult = prTrackFitResult
        else:
            prTrackFitResult = getSeedTrackFitResult(trackCand)
            filterProperties.seedResult = prTrackFitResult

        # skip this track due to the filtering rules ?
        return self.track_filter_object.doesPrPass(filterProperties)

    def peel(self, trackCand):
        """Looks at the individual pattern recognition tracks and store information about them"""
        trackMatchLookUp = self.trackMatchLookUp
        nan = float('nan')

        # Matching information
        is_matched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
        is_clone = trackMatchLookUp.isClonePRTrackCand(trackCand)
        is_matched_or_is_clone = is_matched or is_clone

        # True fit parametes
        if is_matched or is_clone:
            # Only matched and clone tracks have a related MCParticle
            mcParticle = trackMatchLookUp.getRelatedMCParticle(trackCand)
            mcHelix = getHelixFromMCParticle(mcParticle)

            omega_truth = mcHelix.getOmega()
            tan_lambda_truth = mcHelix.getTanLambda()
        else:
            omega_truth = nan
            tan_lambda_truth = nan

        # Get the fit result
        if self.fit:
            fit_result = trackMatchLookUp.getRelatedTrackFitResult(trackCand)
        else:
            fit_result = getSeedTrackFitResult(trackCand)

        if fit_result:
            omega_estimate = fit_result.getOmega()
            omega_variance = fit_result.getCov()[9]

            tan_lambda_estimate = fit_result.getCotTheta()
            tan_lambda_variance = fit_result.getCov()[14]
            momentum = fit_result.getMomentum()
            momentum_pt = momentum.Perp()

            is_fitted = True

        else:
            omega_estimate = nan
            omega_variance = nan
            tan_lambda_estimate = nan
            tan_lambda_variance = nan
            momentum_pt = nan

            is_fitted = False

        # Seed information
        # store seed information, they are always available from the pattern reco
        # even if the fit was no successful
        # this information can we used when plotting fake tracks, for example
        seed_position = trackCand.getPosSeed()
        seed_momentum = trackCand.getMomSeed()

        return dict(
            # store properties of the seed
            pr_seed_tan_lambda=np.divide(1.0, math.tan(seed_momentum.Theta())),
            pr_seed_phi=seed_position.Phi(),
            pr_seed_theta=seed_position.Theta(),

            # store properties resulting from this trackfit
            pr_clone_or_match=is_matched_or_is_clone,
            pr_match=is_matched,
            pr_clone=is_clone,
            pr_fake=not is_matched_or_is_clone,

            pr_omega_estimate=omega_estimate,
            pr_omega_variance=omega_variance,
            pr_omega_truth=omega_truth,

            pr_tan_lambda_estimate=tan_lambda_estimate,
            pr_tan_lambda_variance=tan_lambda_variance,
            pr_tan_lambda_truth=tan_lambda_truth,

            is_fitted=is_fitted,
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(folder_name="pr_tree", name="pr_tree", above_expert_level=1)

    save_clone_rate = refiners.save_fom(
        name="{module.name}_clone_rate_figures_of_merit",
        title="Clone rate in {module.title}",
        description="clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches)",
        key="clone rate",
        select="pr_clone",
        aggregation=np.mean,
        filter_on="pr_clone_or_match",
    )

    save_fake_rate = refiners.save_fom(
        name="{module.name}_fake_rate_figures_of_merit",
        title="Fake rate in {module.title}",
        description="fake_rate - ratio of pattern recognition tracks that are not related to a particle (background, ghost) to all pattern recognition tracks",
        key="fake rate",
        select="pr_fake",
        aggregation=np.mean,
    )

    @refiners.Refiner
    def save_omega_pull_analysis(self, crops, **kwds):
        if self.fit:
            quantity_name = '#omega'
        else:
            quantity_name = 'seed #omega'

        if self.expert_level < 1:
            folder_name = None
        else:
            folder_name = 'expert'

        save_pull_analysis = refiners.save_pull_analysis(
            part_name="pr_omega",
            quantity_name=quantity_name,
            folder_name=folder_name,
            unit="1/cm",
        )

        save_pull_analysis(self, crops, **kwds)

    @refiners.Refiner
    def save_tan_lambda_pull_analysis(self, crops, **kwds):
        if self.fit:
            quantity_name = 'tan #lambda'
        else:
            quantity_name = 'seed tan #lambda'

        # Do not use the expert folder in case of expert level = 0
        if self.expert_level < 1:
            folder_name = None
        else:
            folder_name = 'expert'

        save_pull_analysis = refiners.save_pull_analysis(
            quantity_name=quantity_name,
            part_name="pr_tan_lambda",
            folder_name=folder_name
        )

        save_pull_analysis(self, crops, **kwds)

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_fake_rate_profiles = {
        'pr_fakes': 'fake rate',
        'pr_seed_tan_lambda': 'seed tan #lambda',
        'pr_seed_phi': 'seed #phi',
        'pr_seed_theta': 'seed #theta',
    }

    save_fake_rate_profile = refiners.save_profiles(
        select=renaming_select_for_fake_rate_profiles,
        y='fake rate',
        outlier_z_score=5.0,
        allow_discrete=True,
    )


class ExpertPRSideTrackingValidationModule(PRSideTrackingValidationModule):

    """Module to collect more matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding. This module gives information on the number of hits etc."""

    def peel(self, trackCand):
        base_crops = super(ExpertPRSideTrackingValidationModule, self).peel(trackCand)

        trackCandHits = set(trackCand.getHitIDs(Belle2.Const.CDC))

        # Building the confusion matrix once more :-)
        list_of_connected_mc_tracks = set()
        list_of_numbers_of_hits_for_connected_tracks = []
        number_of_connected_tracks = 0
        number_of_wrong_hits = 0

        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)

        for mcTrackCand in mcTrackCands:
            mcTrackCandHits = set(mcTrackCand.getHitIDs(Belle2.Const.CDC))
            length_of_intersection = len(mcTrackCandHits & trackCandHits)
            if length_of_intersection > 0:
                list_of_connected_mc_tracks.add(mcTrackCand)
                list_of_numbers_of_hits_for_connected_tracks.append(length_of_intersection)

        if len(list_of_numbers_of_hits_for_connected_tracks) == 0:
            number_of_wrong_hits = 0
            pr_number_of_matched_hits = 0
        else:
            maximum_intersection = max(list_of_numbers_of_hits_for_connected_tracks)
            pr_number_of_matched_hits = sum(list_of_numbers_of_hits_for_connected_tracks)
            number_of_wrong_hits = sum(list_of_numbers_of_hits_for_connected_tracks) - maximum_intersection

        hit_crops = dict(
            pr_number_of_matched_hits=pr_number_of_matched_hits,
            number_of_wrong_hits=number_of_wrong_hits,
            number_of_connected_tracks=len(list_of_connected_mc_tracks),
            pr_number_of_hits=len(trackCandHits),
        )

        base_crops.update(hit_crops)

        return base_crops


def main():
    prSideTrackingValidationModule = ExpertPRSideTrackingValidationModule(name='pr_test', contact='dummy')
    prSideTrackingValidationModule.expert_level = 0
    prSideTrackingValidationModule.run('tracked_gun100.root')


if __name__ == '__main__':
    import logging
    logging.basicConfig(level=logging.DEBUG)
    main()
