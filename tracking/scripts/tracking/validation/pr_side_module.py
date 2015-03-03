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
            output_file_name=None,
            trackCandidatesColumnName="TrackCands",
            expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super(PRSideTrackingValidationModule, self).__init__(foreach=trackCandidatesColumnName,
                                                             name=name,
                                                             contact=contact,
                                                             output_file_name=output_file_name,
                                                             expert_level=expert_level)

        self.trackCandidatesColumnName = trackCandidatesColumnName
        self.mcTrackCandidatesColumnName = "MCTrackCands"

    def initialize(self):
        super(PRSideTrackingValidationModule, self).initialize()
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

    def pick(self, trackCand):
        return True

    @staticmethod
    def peel_fit_result(fit_result, key="{part_name}"):
        nan = float("nan")
        if fit_result:
            fit_crops = dict(
                omega_estimate=fit_result.getOmega(),
                omega_variance=fit_result.getCov()[9],
                tan_lambda_estimate=fit_result.getCotTheta(),
                tan_lambda_variance=fit_result.getCov()[14],
                is_fitted=True,  # FIXME
            )
        else:
            fit_crops = dict(
                omega_estimate=nan,
                omega_variance=nan,
                tan_lambda_estimate=nan,
                tan_lambda_variance=nan,
                is_fitted=False,
            )

        if key:
            fit_crops_with_keys = dict()
            for part_name, value in fit_crops.items():
                fit_crops_with_keys[key.format(part_name=part_name)] = value
            return fit_crops_with_keys

        else:
            return fit_crops

    def peel(self, trackCand):
        """Looks at the individual pattern recognition tracks and store information about them"""
        trackMatchLookUp = self.trackMatchLookUp
        nan = float('nan')

        # Matching information
        is_matched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
        is_clone = trackMatchLookUp.isClonePRTrackCand(trackCand)
        is_matched_or_is_clone = is_matched or is_clone

        # Seed information
        # store seed information, they are always available from the pattern reco
        # even if the fit was no successful
        # this information can we used when plotting fake tracks, for example
        seed_position = trackCand.getPosSeed()
        seed_momentum = trackCand.getMomSeed()

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

        crops = dict(
            # store properties of the seed
            pr_seed_tan_lambda=np.divide(1.0, math.tan(seed_momentum.Theta())),
            pr_seed_phi=seed_position.Phi(),
            pr_seed_theta=seed_position.Theta(),

            # store properties resulting from this trackfit
            pr_clone_or_match=is_matched_or_is_clone,
            pr_match=is_matched,
            pr_clone=is_clone,
            pr_fake=not is_matched_or_is_clone,

            pr_omega_truth=omega_truth,
            pr_tan_lambda_truth=tan_lambda_truth,
        )

        # Get the fit result
        seed_fit_result = getSeedTrackFitResult(trackCand)
        seed_fit_crops = self.peel_fit_result(seed_fit_result, key="pr_seed_{part_name}")
        crops.update(seed_fit_crops)

        fit_result = trackMatchLookUp.getRelatedTrackFitResult(trackCand)
        fit_crops = self.peel_fit_result(seed_fit_result, key="pr_{part_name}")
        crops.update(fit_crops)

        return crops

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

    # Pulls of seed parameters
    save_seed_omega_pull_analysis = refiners.save_pull_analysis(
        part_name="pr_seed_omega",
        quantity_name="seed #omega",
        folder_name="pull_seed_omega",
        truth_name="pr_omega_truth",
        unit="1/cm",
    )

    save_seed_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        part_name="pr_seed_tan_lambda",
        quantity_name="seed tan #lambda",
        folder_name="pull_seed_tan_lambda",
        truth_name="pr_tan_lambda_truth",
    )

    # Pull of fitted parameters
    save_fitted_omega_pull_analysis = refiners.save_pull_analysis(
        part_name="pr_omega",
        quantity_name="#omega",
        folder_name="pull_fitted_omega",
        unit="1/cm",
        truth_name="pr_omega_truth",
    )

    save_fitted_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        part_name="pr_tan_lambda",
        quantity_name="tan #lambda",
        folder_name="pull_fitted_tan_lambda",
        truth_name="pr_tan_lambda_truth",
    )

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_fake_rate_profiles = {
        'pr_fake': 'fake rate',
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

    def prepare(self):
        super(ExpertPRSideTrackingValidationModule, self).prepare()

        mc_track_cands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)

        mc_track_cands_cdc_hit_ids = [frozenset(mc_track_cand.getHitIDs(Belle2.Const.CDC)) for mc_track_cand in mc_track_cands]
        self.mc_track_cands_cdc_hit_ids = mc_track_cands_cdc_hit_ids

    def peel(self, trackCand):
        base_crops = super(ExpertPRSideTrackingValidationModule, self).peel(trackCand)

        trackMatchLookUp = self.trackMatchLookUp

        trackCandHits = set(trackCand.getHitIDs(Belle2.Const.CDC))

        # Building the confusion matrix once more :-)
        list_of_connected_mc_tracks = set()
        list_of_numbers_of_hits_for_connected_tracks = []
        number_of_connected_tracks = 0
        number_of_wrong_hits = 0

        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)

        for mc_track_id, mcTrackCandHits in enumerate(self.mc_track_cands_cdc_hit_ids):
            mcTrackCand = mcTrackCands[mc_track_id]
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
