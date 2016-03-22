import math
import numpy as np

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.validation.peelers as peelers

import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2


class PRSideTrackingValidationModule(harvesting.HarvestingModule):

    """Module to collect matching information about the found particles and to generate
       validation plots and figures of merit on the performance of track finding."""

    def __init__(
            self,
            name,
            contact,
            output_file_name=None,
            trackCandidatesColumnName="TrackCands",
            mcTrackCandidatesColumnName='MCTrackCands',
            expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super(PRSideTrackingValidationModule, self).__init__(foreach=trackCandidatesColumnName,
                                                             name=name,
                                                             contact=contact,
                                                             output_file_name=output_file_name,
                                                             expert_level=expert_level)

        self.trackCandidatesColumnName = trackCandidatesColumnName
        self.mcTrackCandidatesColumnName = mcTrackCandidatesColumnName

    def initialize(self):
        super(PRSideTrackingValidationModule, self).initialize()
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

    def pick(self, track_cand):
        return True

    def peel(self, track_cand):
        """Looks at the individual pattern recognition tracks and store information about them"""
        trackMatchLookUp = self.trackMatchLookUp

        crops = {}

        # Matching information
        mc_particle = trackMatchLookUp.getRelatedMCParticle(track_cand)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)
        crops.update(mc_particle_crops)

        hit_content_crops = peelers.peel_track_cand_hit_content(track_cand)
        crops.update(hit_content_crops)

        pr_to_mc_match_info_crops = self.peel_pr_to_mc_match_info(track_cand)
        crops.update(pr_to_mc_match_info_crops)

        # Get the fit result
        seed_fit_crops = peelers.peel_track_cand_seed(track_cand)
        crops.update(seed_fit_crops)

        fit_result = trackMatchLookUp.getRelatedTrackFitResult(track_cand)
        fit_crops = peelers.peel_fit_result(fit_result, key="{part_name}")
        crops.update(fit_crops)

        return crops

    def peel_pr_to_mc_match_info(self, track_cand):
        trackMatchLookUp = self.trackMatchLookUp
        is_matched = trackMatchLookUp.isMatchedPRTrackCand(track_cand)
        is_clone = trackMatchLookUp.isClonePRTrackCand(track_cand)
        is_background = trackMatchLookUp.isBackgroundPRTrackCand(track_cand)
        is_ghost = trackMatchLookUp.isGhostPRTrackCand(track_cand)
        return dict(
            is_matched=is_matched,
            is_clone=is_clone,
            is_background=is_background,
            is_ghost=is_ghost,
            is_clone_or_match=(is_matched or is_clone),
            is_fake=not (is_matched or is_clone),
            hit_efficiency=trackMatchLookUp.getRelatedEfficiency(track_cand),
            hit_purity=trackMatchLookUp.getRelatedPurity(track_cand),
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(folder_name="pr_tree", name="pr_tree", above_expert_level=1)

    save_clone_rate = refiners.save_fom(
        name="{module.id}_overview_figures_of_merit",
        # Same as in the mc side module to combine the overview figures of merit into the same TNTuple
        title="Overview figures in {module.title}",
        description="clone_rate - ratio of clones divided the number of tracks that are related to a particle (clones and matches)",
        key="clone rate",
        select=["is_clone"],
        aggregation=np.mean,
        filter_on="is_clone_or_match",
    )

    save_fake_rate = refiners.save_fom(
        name="{module.id}_overview_figures_of_merit",
        # Same as in the mc side module to combine the overview figures of merit into the same TNTuple
        title="Overview figures in {module.title}",
        description="fake_rate - ratio of pattern recognition tracks that are not related to a particle" +
                    "(background, ghost) to all pattern recognition tracks",
        key="fake rate",
        select="is_fake",
        aggregation=np.mean,
    )

    # Pulls of seed parameters
    save_seed_omega_pull_analysis = refiners.save_pull_analysis(
        part_name="seed_omega",
        quantity_name="seed #omega",
        folder_name="pull_seed_omega",
        truth_name="omega_truth",
        unit="1/cm",
    )

    save_seed_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        part_name="seed_tan_lambda",
        quantity_name="seed tan #lambda",
        folder_name="pull_seed_tan_lambda",
        truth_name="tan_lambda_truth",
    )

    # Pull of fitted parameters
    save_fitted_omega_pull_analysis = refiners.save_pull_analysis(
        part_name="omega",
        quantity_name="#omega",
        folder_name="pull_fitted_omega",
        unit="1/cm",
    )

    save_fitted_tan_lambda_pull_analysis = refiners.save_pull_analysis(
        part_name="tan_lambda",
        quantity_name="tan #lambda",
        folder_name="pull_fitted_tan_lambda",
    )

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    save_fake_rate_by_seed_phi0_profile = refiners.save_profiles(
        select={
            'is_fake': 'fake rate',
            'seed_phi0_estimate': 'seed #phi',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
    )

    save_fake_rate_by_seed_tan_lambda_profile = refiners.save_profiles(
        select={
            'is_fake': 'fake rate',
            'seed_tan_lambda_estimate': 'seed tan #lambda',
        },
        y='fake rate',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
    )


class ExpertPRSideTrackingValidationModule(PRSideTrackingValidationModule):

    """Module to collect more matching information about the found particles and to
       generate validation plots and figures of merit on the performance of track finding.
       This module gives information on the number of hits etc."""

    def prepare(self):
        super(ExpertPRSideTrackingValidationModule, self).prepare()

        mc_track_cands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)

        mc_track_cands_cdc_hit_ids = []
        for mc_track_cand in mc_track_cands:
            mc_track_cand_cdc_hit_ids = mc_track_cand.getHitIDs(Belle2.Const.CDC)  # Checked
            # Working around a bug in ROOT where you should not access empty std::vectors
            if len(mc_track_cand_cdc_hit_ids) == 0:
                mc_track_cand_cdc_hit_ids = frozenset()
            else:
                mc_track_cand_cdc_hit_ids = frozenset(mc_track_cand_cdc_hit_ids)
            mc_track_cands_cdc_hit_ids .append(mc_track_cand_cdc_hit_ids)

        self.mc_track_cands_cdc_hit_ids = mc_track_cands_cdc_hit_ids

    def peel(self, trackCand):
        base_crops = super(ExpertPRSideTrackingValidationModule, self).peel(trackCand)

        trackMatchLookUp = self.trackMatchLookUp

        trackCandHits = trackCand.getHitIDs(Belle2.Const.CDC)  # Checked
        # Working around a bug in ROOT where you should not access empty std::vectors
        if len(trackCandHits) == 0:
            trackCandHits = set()
        else:
            trackCandHits = set(trackCandHits)

        # Building the confusion matrix once more :-)
        list_of_connected_mc_tracks = []
        list_of_numbers_of_hits_for_connected_tracks = []
        number_of_connected_tracks = 0
        number_of_wrong_hits = 0

        mcTrackCands = Belle2.PyStoreArray(self.mcTrackCandidatesColumnName)

        for mc_track_id, mcTrackCandHits in enumerate(self.mc_track_cands_cdc_hit_ids):
            mcTrackCand = mcTrackCands[mc_track_id]
            length_of_intersection = len(mcTrackCandHits & trackCandHits)
            if length_of_intersection > 0:
                list_of_connected_mc_tracks.append(mcTrackCand)
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
