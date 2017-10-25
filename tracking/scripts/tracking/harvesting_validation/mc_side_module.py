import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import tracking.harvesting_validation.basic_mc_side_module as basic_mc_side_module

import tracking.validation.utilities as utilities
import tracking.harvest.refiners as refiners
import tracking.harvest.peelers as peelers


class MCSideTrackingValidationModule(basic_mc_side_module.BasicMCSideTrackingValidationModule):

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

        super().__init__(name=name,
                         contact=contact,
                         output_file_name=output_file_name,
                         reco_tracks_name=reco_tracks_name,
                         mc_reco_tracks_name=mc_reco_tracks_name,
                         expert_level=expert_level)

        #: Set of all detector and hits ids contained in any pr track. Updated each event.
        self.found_det_hit_ids = set()

        #: Set of all detector and hits ids contained in matched pr tracks. Updated each event.
        self.matched_det_hit_ids = set()

        #: Set of all detector and hits ids contained in clone pr tracks. Updated each event.
        self.clone_det_hit_ids = set()

        #: Set of all detector and hits ids contained in background and ghost pr tracks. Updated each event.
        self.fake_det_hit_ids = set()

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

            if track_match_look_up.isMatchedPRRecoTrack(reco_track):
                matched_det_hit_ids |= det_hit_ids

            if track_match_look_up.isClonePRRecoTrack(reco_track):
                clone_det_hit_ids |= det_hit_ids

            if (track_match_look_up.isGhostPRRecoTrack(reco_track) or
                    track_match_look_up.isBackgroundPRRecoTrack(reco_track)):
                fake_det_hit_ids |= det_hit_ids

        self.found_det_hit_ids = found_det_hit_ids
        self.matched_det_hit_ids = matched_det_hit_ids
        self.clone_det_hit_ids = clone_det_hit_ids
        self.fake_det_hit_ids = fake_det_hit_ids

    def peel(self, mc_reco_track):
        crops = super().peel(mc_reco_track=mc_reco_track)

        track_match_look_up = self.track_match_look_up
        mc_particle = track_match_look_up.getRelatedMCParticle(mc_reco_track)
        reco_track = track_match_look_up.getRelatedPRRecoTrack(mc_reco_track)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)
        hit_content_crops = peelers.peel_reco_track_hit_content(mc_reco_track)

        # Custom peel function to get single detector hit purities
        subdetector_hit_efficiency_crops = peelers.peel_subdetector_hit_efficiency(mc_reco_track, reco_track)

        mc_hit_efficiencies_in_all_pr_tracks_crops = self.peel_hit_efficiencies_in_all_pr_tracks(mc_reco_track)

        # Event Info
        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        event_crops = peelers.peel_event_info(event_meta_data)

        # Store Array for easier joining
        store_array_crops = peelers.peel_store_array_info(reco_track, key="pr_{part_name}")

        crops.update(dict(**hit_content_crops,
                          **mc_particle_crops,
                          **subdetector_hit_efficiency_crops,
                          **mc_hit_efficiencies_in_all_pr_tracks_crops,
                          **event_crops,
                          **store_array_crops
                          ))

        return crops

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

    # Save a histogram of the hit efficiency
    save_hit_efficiency_histogram = refiners.save_histograms(
        select={"hit_efficiency": "hit efficiency"},
        filter_on="is_primary",
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
        filter_on="is_primary",
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
        filter_on="is_primary",
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
        filter_on="is_primary",
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
        filter_on="is_primary",
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
        filter_on="is_primary",
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
