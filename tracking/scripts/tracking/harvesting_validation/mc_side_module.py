import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import math
import warnings

import numpy as np

import tracking.validation.utilities as utilities

import tracking.harvest.refiners as refiners
import tracking.harvest.harvesting as harvesting
import tracking.harvest.peelers as peelers


class MCSideTrackingValidationModule(harvesting.HarvestingModule):

    """Module to collect matching information about the found particles and to generate
       validation plots and figures of merit on the performance of track finding."""

    def __init__(
            self,
            name,
            contact,
            output_file_name=None,
            reco_tracks_name='RecoTracks',
            mc_reco_tracks_name='MCRecoTracks',
            expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super().__init__(foreach=mc_reco_tracks_name,
                         name=name,
                         output_file_name=output_file_name,
                         contact=contact,
                         expert_level=expert_level)

        #: Name of the StoreArray of the tracks from pattern recognition
        self.reco_tracks_name = reco_tracks_name

        #: Name of the StoreArray of the ideal mc tracks
        self.mc_reco_tracks_name = mc_reco_tracks_name

        #: Reference to the track match lookup object reading the relation information constructed by the MCMatcherTracksModule
        self.track_match_look_up = None

        #: Set of all detector and hits ids contained in any pr track. Updated each event.
        self.found_det_hit_ids = set()

        #: Set of all detector and hits ids contained in matched pr tracks. Updated each event.
        self.matched_det_hit_ids = set()

        #: Set of all detector and hits ids contained in clone pr tracks. Updated each event.
        self.clone_det_hit_ids = set()

        #: Set of all detector and hits ids contained in background and ghost pr tracks. Updated each event.
        self.fake_det_hit_ids = set()

    def initialize(self):
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_reco_tracks_name, self.reco_tracks_name)

    def prepare(self):
        """Collect some statistics about the pattern recognition tracks used for comparision to the MC tracks

        Executed once at the start of each event.
        """
        super().prepare()

        reco_tracks = Belle2.PyStoreArray(self.reco_tracks_name)
        track_match_look_up = self.track_match_look_up

        found_det_hit_ids = set()
        matched_det_hit_ids = set()
        clone_det_hit_ids = set()
        fake_det_hit_ids = set()

        for reco_track in reco_tracks:
            det_hit_ids = utilities.get_det_hit_ids(reco_track)

            found_det_hit_ids |= det_hit_ids

            if track_match_look_up.isMatchedPRTrackCand(reco_track):
                matched_det_hit_ids |= det_hit_ids

            if track_match_look_up.isClonePRTrackCand(reco_track):
                clone_det_hit_ids |= det_hit_ids

            if (track_match_look_up.isGhostPRTrackCand(reco_track) or
                    track_match_look_up.isBackgroundPRTrackCand(reco_track)):
                fake_det_hit_ids |= det_hit_ids

        self.found_det_hit_ids = found_det_hit_ids
        self.matched_det_hit_ids = matched_det_hit_ids
        self.clone_det_hit_ids = clone_det_hit_ids
        self.fake_det_hit_ids = fake_det_hit_ids

    def pick(self, mc_reco_track):
        return True

    def peel(self, mc_reco_track):
        """Looks at the individual Monte Carlo tracks and store information about them"""
        track_match_look_up = self.track_match_look_up
        mc_particles = Belle2.PyStoreArray('MCParticles')

        # Analyse from the Monte Carlo reference side
        mc_reco_tracks = Belle2.PyStoreArray(self.foreach)
        multiplicity = mc_reco_tracks.getEntries()

        mc_particle = track_match_look_up.getRelatedMCParticle(mc_reco_track)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)
        hit_content_crops = peelers.peel_reco_track_hit_content(mc_reco_track)
        mc_to_pr_match_info_crops = self.peel_mc_to_pr_match_info(mc_reco_track)
        mc_hit_efficiencies_in_all_pr_tracks_crops = self.peel_hit_efficiencies_in_all_pr_tracks(mc_reco_track)

        crops = dict(multiplicity=multiplicity,
                     **mc_to_pr_match_info_crops,
                     **hit_content_crops,
                     **mc_particle_crops,
                     **mc_hit_efficiencies_in_all_pr_tracks_crops)

        return crops

    def peel_mc_to_pr_match_info(self, mc_reco_track):
        track_match_look_up = self.track_match_look_up
        return dict(
            is_matched=track_match_look_up.isMatchedMCTrackCand(mc_reco_track),
            is_merged=track_match_look_up.isMergedMCTrackCand(mc_reco_track),
            is_missing=track_match_look_up.isMissingMCTrackCand(mc_reco_track),
            hit_efficiency=track_match_look_up.getRelatedEfficiency(mc_reco_track),
            hit_purity=track_match_look_up.getRelatedPurity(mc_reco_track),
        )

    def peel_hit_efficiencies_in_all_pr_tracks(self, mc_reco_track):
        mc_det_hit_ids = utilities.get_det_hit_ids(mc_reco_track)

        hit_efficiency_in_all_found = utilities.calc_hit_efficiency(self.found_det_hit_ids,
                                                                    mc_det_hit_ids)

        unfound_hit_efficiency = 1.0 - hit_efficiency_in_all_found

        hit_efficiency_in_all_matched = utilities.calc_hit_efficiency(self.matched_det_hit_ids,
                                                                      mc_det_hit_ids)

        hit_efficiency_in_all_fake = utilities.calc_hit_efficiency(self.fake_det_hit_ids,
                                                                   mc_det_hit_ids)

        hit_efficiency_crops = dict(
            hit_efficiency_in_all_found=hit_efficiency_in_all_found,
            unfound_hit_efficiency=unfound_hit_efficiency,
            hit_efficiency_in_all_mached=hit_efficiency_in_all_matched,
            hit_efficiency_in_all_fake=hit_efficiency_in_all_fake,
        )
        return hit_efficiency_crops

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(name="mc_tree", folder_name="mc_tree", above_expert_level=1)

    # Generate the average finding efficiencies and hit efficiencies
    save_overview_figures_of_merit = refiners.save_fom(
        name="{module.id}_overview_figures_of_merit",
        title="Overview figures in {module.title}",
        aggregation=np.nanmean,
        key="{part_name}",
        select={"is_matched": "finding efficiency", "hit_efficiency": "hit efficiency", },
        description="""
finding efficiency - the ratio of matched Monte Carlo tracks to all Monte Carlo tracks
hit efficiency - the ratio of hits picked up by a matched Carlo track  to all Monte Carlo tracks
"""
    )

    # Save a histogram of the hit efficiency
    save_hit_efficiency_histogram = refiners.save_histograms(
        select={"hit_efficiency": "hit efficiency"},
        description="Not a serious plot yet.",
    )

    # Make profiles of the finding efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_finding_efficiency_profiles = {
        'is_matched': 'finding efficiency',
        'd0_truth': 'd_{0}',
        'pt_truth': 'p_{t}',
        'multiplicity': 'multiplicity',
        'phi0_truth': '#phi',
    }

    save_finding_efficiency_profiles = refiners.save_profiles(
        select=renaming_select_for_finding_efficiency_profiles,
        y='finding efficiency',
        y_binary=True,
        outlier_z_score=5.0,
        allow_discrete=True,
    )

    save_finding_efficiency_by_tan_lamba_profiles = refiners.save_profiles(
        select={
            'is_matched': 'finding efficiency',
            'tan_lambda_truth': 'tan #lambda'
        },
        y='finding efficiency',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
    )

    save_finding_efficiency_by_tan_lamba_in_pt_groups_profiles = refiners.save_profiles(
        select={
            'is_matched': 'finding efficiency',
            'tan_lambda_truth': 'tan #lambda'
        },
        y='finding efficiency',
        y_binary=True,
        groupby=[("pt_truth", [0.070, 0.250, 0.600])],
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
    )

    # Make profiles of the hit efficiencies versus various fit parameters
    # Rename the quatities to names that display nicely by root latex translation
    renaming_select_for_hit_efficiency_profiles = {
        'hit_efficiency': 'hit efficiency',
        'd0_truth': 'd_{0}',
        'pt_truth': 'p_{t}',
        'multiplicity': 'multiplicity',
        'phi0_truth': '#phi',
    }

    save_hit_efficiency_profiles = refiners.save_profiles(
        select=renaming_select_for_hit_efficiency_profiles,
        y='hit efficiency',
        y_binary=True,
        outlier_z_score=5.0,
        allow_discrete=True,
    )

    save_hit_efficiency_by_tan_lambda_profiles = refiners.save_profiles(
        select={
            'hit_efficiency': 'hit efficiency',
            'tan_lambda_truth': 'tan #lambda',
        },
        y='hit efficiency',
        y_binary=True,
        outlier_z_score=5.0,
        lower_bound=-1.73,
        upper_bound=3.27,
    )

    #: This creates a histogram for all MC track displaying the ratio of hits contained in any PR track
    #: Hence this is distinctly larger than the hit efficiency to the matched PR track
    #: Usefulness under discussion
    save_hit_efficiency_in_all_found_hist = refiners.save_histograms(
        # renaming quantity to name that is more suitable for display
        select=dict(hit_efficiency_in_all_found="total hit efficiency vs. all reconstructed tracks")
    )

    #: This creates a histogram for *each missing* MC track displaying the ratio of hits contained in any PR tracks
    #: High values in this hit efficiencies means that the MC track is consumed by other PR tracks but no proper
    #: match could be established.
    save_missing_mc_tracks_hit_efficiency_in_all_found_hist = refiners.save_histograms(
        filter_on="is_missing",  # show only the efficiencies of missing mc tracks
        # renaming quantity to name that is more suitable for display
        select=dict(hit_efficiency_in_all_found="total hit efficiency in all reconstructed tracks for missing mc tracks")
    )


class ExpertMCSideTrackingValidationModule(MCSideTrackingValidationModule):
    """Module to collect more matching information about the found particles and to generate
       validation plots and figures of merit on the performance of track finding. This module
       gives information on the number of hits etc. """

    def __init__(self, *args, **kwds):
        """Constructor issuing a deprecation warning"""
        warnings.warn("ExpertMCSideTrackingValidationModule is depricated for MCSideTrackingValidationModule",
                      DeprecationWarning)
        super().__init__(*args, **kwds)
