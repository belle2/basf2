import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import math
import numpy as np

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.validation.utilities as utilities


class EventwiseTrackingValidationModule(harvesting.HarvestingModule):

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 track_cands_name='TrackCands',
                 mc_track_cands_name='MCTrackCands',
                 expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super().__init__(foreach="EventMetaData",  # Dummy for on element per event
                         name=name,
                         output_file_name=output_file_name,
                         contact=contact,
                         expert_level=expert_level)

        self.track_cands_name = track_cands_name
        self.mc_track_cands_name = mc_track_cands_name
        self.cdc_hits_name = "CDCHits"

    def initialize(self):
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_track_cands_name,
                                                           self.track_cands_name)

    def pick(self, event_meta_data=None):
        return True

    def peel(self, event_meta_data=None):
        # Note event_meta_data is just used as a dummy.

        track_match_look_up = self.track_match_look_up
        track_cands = Belle2.PyStoreArray(self.track_cands_name)
        mc_track_cands = Belle2.PyStoreArray(self.mc_track_cands_name)
        cdc_hits = Belle2.PyStoreArray(self.cdc_hits_name)
        mc_particles = Belle2.PyStoreArray("MCParticles")

        # General object count in the event.
        if mc_particles:
            n_mc_particles = mc_particles.getEntries()
        else:
            n_mc_particles = -1

        if mc_track_cands:
            n_mc_track_cands = mc_track_cands.getEntries()
        else:
            n_mc_track_cands = -1

        n_track_cands = track_cands.getEntries()

        # Aggregate information about Monte Carlo tracks
        all_mc_tracks_det_hit_ids = set()
        n_matched_mc_track_cands = 0
        n_merged_mc_track_cands = 0
        n_missing_mc_track_cands = 0
        for mc_track_cand in mc_track_cands:
            mc_track_cand_det_hit_ids = utilities.get_det_hit_ids(mc_track_cand)
            all_mc_tracks_det_hit_ids.update(mc_track_cand_det_hit_ids)

            is_matched = track_match_look_up.isMatchedMCTrackCand(mc_track_cand)
            is_merged = track_match_look_up.isMergedMCTrackCand(mc_track_cand)
            is_missing = track_match_look_up.isMissingMCTrackCand(mc_track_cand)

            if is_matched:
                n_matched_mc_track_cands += 1
            elif is_merged:
                n_merged_mc_track_cands += 1
            elif is_missing:
                n_missing_mc_track_cands += 1

        # Aggregate information about pattern recognition tracks
        n_matched_track_cands = 0
        n_clone_track_cands = 0
        n_background_track_cands = 0
        n_ghost_track_cands = 0

        all_tracks_det_hit_ids = set()
        n_matched_hits = 0
        for track_cand in track_cands:
            is_matched = track_match_look_up.isMatchedPRTrackCand(track_cand)
            is_clone = track_match_look_up.isClonePRTrackCand(track_cand)
            is_background = track_match_look_up.isBackgroundPRTrackCand(track_cand)
            is_ghost = track_match_look_up.isGhostPRTrackCand(track_cand)

            if is_matched:
                n_matched_track_cands += 1
            elif is_clone:
                n_clone_track_cands += 1
            elif is_background:
                n_background_track_cands += 1
            elif is_ghost:
                n_ghost_track_cands += 1

            track_cand_det_hit_ids = utilities.get_det_hit_ids(track_cand)

            all_tracks_det_hit_ids.update(track_cand_det_hit_ids)
            if is_matched or is_clone:
                mc_track_cand = self.track_match_look_up.getRelatedMCTrackCand(track_cand)

                mc_track_cand_det_hit_ids = utilities.get_det_hit_ids(mc_track_cand)
                n_matched_hits += len(track_cand_det_hit_ids & mc_track_cand_det_hit_ids)

        return dict(
            n_mc_particles=n_mc_particles,
            n_mc_track_cands=n_mc_track_cands,
            n_track_cands=n_track_cands,

            n_matched_mc_track_cands=n_matched_mc_track_cands,
            n_merged_mc_track_cands=n_merged_mc_track_cands,
            n_missing_mc_track_cands=n_missing_mc_track_cands,

            n_matched_track_cands=n_matched_track_cands,
            n_clone_track_cands=n_clone_track_cands,
            n_background_track_cands=n_background_track_cands,
            n_ghost_track_cands=n_ghost_track_cands,

            n_cdc_hits=cdc_hits.getEntries(),
            n_all_mc_track_hits=len(all_mc_tracks_det_hit_ids),
            n_all_track_hits=len(all_tracks_det_hit_ids),
            n_found_hits=len(all_mc_tracks_det_hit_ids & all_tracks_det_hit_ids),
            n_matched_hits=n_matched_hits
        )

    # Refiners to be executed on terminate #
    # #################################### #

    # Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(folder_name="event_tree",
                                   name="event_tree",
                                   above_expert_level=1)

    save_clone_rate = refiners.save_fom(
        name="{module.id}_hit_figures_of_merit",
        title="Hit sums in {module.title}",
        description="",  # to be given
        select=[
            "n_cdc_hits",
            "n_all_mc_track_hits",
            "n_all_track_hits",
            "n_found_hits",
            "n_matched_hits",
        ],

        aggregation=np.sum,
        key="{part_name}",
    )
