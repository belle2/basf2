import math
import numpy as np

from tracking.validation.utilities import getHelixFromMCParticle, getSeedTrackFitResult, is_primary

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2


class MCSideTrackingValidationModule(harvesting.HarvestingModule):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def __init__(
            self,
            name,
            contact,
            output_file_name=None,
            trackCandidatesColumnName="TrackCands",
            expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super(MCSideTrackingValidationModule, self).__init__(foreach="MCTrackCands",
                                                             name=name,
                                                             output_file_name=output_file_name,
                                                             contact=contact,
                                                             expert_level=expert_level)

        self.trackCandidatesColumnName = trackCandidatesColumnName
        self.mcTrackCandidatesColumnName = "MCTrackCands"

    def initialize(self):
        super(MCSideTrackingValidationModule, self).initialize()
        self.trackMatchLookUp = Belle2.TrackMatchLookUp(self.mcTrackCandidatesColumnName, self.trackCandidatesColumnName)

    def pick(self, mcTrackCand):
        return True

    def peel(self, mcTrackCand):
        """Looks at the individual Monte Carlo tracks and store information about them"""
        trackMatchLookUp = self.trackMatchLookUp
        mcParticles = Belle2.PyStoreArray('MCParticles')

        # Analyse from the Monte Carlo reference side
        mcTrackCands = Belle2.PyStoreArray(self.foreach)
        multiplicity = mcTrackCands.getEntries()

        mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
        mcHelix = getHelixFromMCParticle(mcParticle)
        momentum = mcParticle.getMomentum()

        # Collected variables
        return dict(
            finding_efficiency=trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand),
            hit_efficiency=trackMatchLookUp.getRelatedEfficiency(mcTrackCand),
            mc_pt=momentum.Perp(),
            mc_tan_lambda=np.divide(1.0, math.tan(momentum.Theta())),  # Avoid zero division exception
            mc_d0=mcHelix.getD0(),
            mc_multiplicity=multiplicity,
            mc_theta=momentum.Theta(),
            mc_phi=momentum.Phi(),
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(name="mc_tree", folder_name="mc_tree", above_expert_level=1)

    # Generate the average finding efficiencies and hit efficiencies
    save_overview_figures_of_merit = refiners.save_fom(
        name="{module.name}_efficiency_figures_of_merit",
        aggregation=np.nanmean,
        key="{part_name}",
        select=["finding_efficiency", "hit_efficiency"],
        description="""
finding_efficiency - the ratio of matched Monte Carlo tracks to all Monte Carlo tracks <br/>
hit_efficiency - the ratio of hits picked up by a matched Carlo track  to all Monte Carlo tracks <br/>
"""
    )

    # Save a histogram of the hit efficiency
    save_hit_efficiency_histogram = refiners.save_histograms(
        select="hit_efficiency",
        description="Not a serious plot yet.",
    )

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_finding_efficiency_profiles = {
        'finding_efficiency': 'finding efficiency',
        'mc_d0': 'd_{0}',
        'mc_pt': 'p_{t}',
        'mc_tan_lambda': 'tan #lambda',
        'mc_multiplicity': 'multiplicity',
        'mc_phi': '#phi',
        'mc_theta': '#theta',
    }

    save_finding_efficiency_profiles = refiners.save_profiles(
        select=renaming_select_for_finding_efficiency_profiles,
        y='finding efficiency',
        outlier_z_score=5.0,
        allow_discrete=True,
    )

    # Make profiles of the hit efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_hit_efficiency_profiles = {
        'hit_efficiency': 'hit efficiency',
        'mc_d0': 'd_{0}',
        'mc_pt': 'p_{t}',
        'mc_tan_lambda': 'tan #lambda',
        'mc_multiplicity': 'multiplicity',
        'mc_phi': '#phi',
        'mc_theta': '#theta'
    }

    save_hit_efficiency_profiles = refiners.save_profiles(
        select=renaming_select_for_hit_efficiency_profiles,
        y='hit efficiency',
        outlier_z_score=5.0,
        allow_discrete=True,
    )


class ExpertMCSideTrackingValidationModule(MCSideTrackingValidationModule):

    """Module to collect more matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding. This module gives information on the number of hits etc. """

    def prepare(self):
        super(ExpertMCSideTrackingValidationModule, self).prepare()

        track_cands = Belle2.PyStoreArray(self.trackCandidatesColumnName)
        trackMatchLookUp = self.trackMatchLookUp

        pr_cdc_hit_ids = set()
        pr_good_cdc_hit_ids = set()
        pr_clone_cdc_hit_ids = set()
        pr_fake_cdc_hit_ids = set()

        for track_cand in track_cands:
            cdc_hit_ids_of_track_cand = set(track_cand.getHitIDs(Belle2.Const.CDC))

            pr_cdc_hit_ids |= cdc_hit_ids_of_track_cand

            if trackMatchLookUp.isMatchedPRTrackCand(track_cand):
                pr_good_cdc_hit_ids |= cdc_hit_ids_of_track_cand

            if trackMatchLookUp.isClonePRTrackCand(track_cand):
                pr_clone_cdc_hit_ids |= cdc_hit_ids_of_track_cand

            if (trackMatchLookUp.isGhostPRTrackCand(track_cand) or
                    trackMatchLookUp.isBackgroundPRTrackCand(track_cand)):
                pr_fake_cdc_hit_ids |= cdc_hit_ids_of_track_cand

        self.pr_cdc_hit_ids = pr_cdc_hit_ids
        self.pr_good_cdc_hit_ids = pr_good_cdc_hit_ids
        self.pr_clone_cdc_hit_ids = pr_clone_cdc_hit_ids
        self.pr_fake_cdc_hit_ids = pr_fake_cdc_hit_ids

    def peel(self, mcTrackCand):
        base_crops = super(ExpertMCSideTrackingValidationModule, self).peel(mcTrackCand)

        is_missing = self.trackMatchLookUp.isMissingMCTrackCand(mcTrackCand)

        mc_particle = self.trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
        mc_is_primary = is_primary(mc_particle) if mc_particle else False

        mc_track_cand_cdc_hit_ids = set(mcTrackCand.getHitIDs(Belle2.Const.CDC))

        ratio = np.divide(1.0 * len(mc_track_cand_cdc_hit_ids & self.pr_cdc_hit_ids), len(mc_track_cand_cdc_hit_ids))
        ratio_hits_in_mc_tracks_and_in_good_pr_tracks = np.divide(1.0 * len(mc_track_cand_cdc_hit_ids & self.pr_good_cdc_hit_ids), len(mc_track_cand_cdc_hit_ids))
        ratio_hits_in_mc_tracks_and_in_fake_pr_tracks = np.divide(1.0 * len(mc_track_cand_cdc_hit_ids & self.pr_fake_cdc_hit_ids), len(mc_track_cand_cdc_hit_ids))

        hit_crops = dict(
            mc_missing=is_missing,
            mc_is_primary=mc_is_primary,
            mc_number_of_hits=len(mc_track_cand_cdc_hit_ids),
            ratio_hits_in_mc_tracks_and_not_in_pr_tracks=1.0 - ratio,
            ratio_hits_in_mc_tracks_and_in_pr_tracks=ratio,
            ratio_hits_in_mc_tracks_and_in_good_pr_tracks=ratio_hits_in_mc_tracks_and_in_good_pr_tracks,
            ratio_hits_in_mc_tracks_and_in_fake_pr_tracks=ratio_hits_in_mc_tracks_and_in_fake_pr_tracks
        )

        base_crops.update(hit_crops)

        return base_crops

    # Refiners to be executed on terminate #
    # #################################### #
    save_ratio_hits_in_mc_tracks_and_in_pr_tracks_hist = refiners.save_histograms(
        select=dict(ratio_hits_in_mc_tracks_and_in_pr_tracks="ratio of hits in MCTracks found by the track finder")  # renaming quantity to name that is more suitable for display
    )

    save_ratio_hits_in_missing_mc_tracks_and_in_pr_tracks_hist = refiners.save_histograms(
        filter_on="mc_missing",  # filter on missing to mimic what ratio_hits_in_missing_mc_tracks_and_in_pr_tracks used to be
        select=dict(ratio_hits_in_mc_tracks_and_in_pr_tracks="ratio of hits in missing MCTracks found by the track finder")  # renaming quantity to name that is more suitable for display
    )


def main():
    mcSideTrackingValidationModule = ExpertMCSideTrackingValidationModule(name='mc_test', contact='dummy')
    mcSideTrackingValidationModule.run('tracked_generic100.root', pyprofile=True)


if __name__ == '__main__':
    import logging
    logging.basicConfig(level=logging.DEBUG)
    main()
