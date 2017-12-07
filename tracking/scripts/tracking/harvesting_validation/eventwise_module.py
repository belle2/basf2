import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import math
import numpy as np

import tracking.validation.utilities as utilities

import tracking.harvest.refiners as refiners
import tracking.harvest.harvesting as harvesting


class EventwiseTrackingValidationModule(harvesting.HarvestingModule):

    """ Expert level behavior:
        expert_level <= default_expert_level: all figures and plots from this module except tree entries
        expert_level > default_expert_level: everything including tree entries
    """
    default_expert_level = 10

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 reco_tracks_name='RecoTracks',
                 mc_reco_tracks_name='MCRecoTracks',
                 expert_level=None):

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super().__init__(foreach="EventMetaData",  # Dummy for on element per event
                         name=name,
                         output_file_name=output_file_name,
                         contact=contact,
                         expert_level=expert_level)

        self.reco_tracks_name = reco_tracks_name
        self.mc_reco_tracks_name = mc_reco_tracks_name
        self.cdc_hits_name = "CDCHits"

    def initialize(self):
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_reco_tracks_name,
                                                           self.reco_tracks_name)

    def pick(self, event_meta_data=None):
        return True

    def peel(self, event_meta_data=None):
        # Note event_meta_data is just used as a dummy.

        track_match_look_up = self.track_match_look_up
        reco_tracks = Belle2.PyStoreArray(self.reco_tracks_name)
        mc_reco_tracks = Belle2.PyStoreArray(self.mc_reco_tracks_name)
        cdc_hits = Belle2.PyStoreArray(self.cdc_hits_name)
        mc_particles = Belle2.PyStoreArray("MCParticles")

        # General object count in the event.
        if mc_particles:
            n_mc_particles = mc_particles.getEntries()
        else:
            n_mc_particles = -1

        if mc_reco_tracks:
            n_mc_reco_tracks = mc_reco_tracks.getEntries()
        else:
            n_mc_reco_tracks = -1

        n_reco_tracks = reco_tracks.getEntries()

        # Aggregate information about Monte Carlo tracks
        all_mc_tracks_det_hit_ids = set()
        n_matched_mc_reco_tracks = 0
        n_merged_mc_reco_tracks = 0
        n_missing_mc_reco_tracks = 0
        for mc_reco_track in mc_reco_tracks:
            mc_reco_track_det_hit_ids = utilities.get_det_hit_ids(mc_reco_track)
            all_mc_tracks_det_hit_ids.update(mc_reco_track_det_hit_ids)

            is_matched = track_match_look_up.isMatchedMCRecoTrack(mc_reco_track)
            is_merged = track_match_look_up.isMergedMCRecoTrack(mc_reco_track)
            is_missing = track_match_look_up.isMissingMCRecoTrack(mc_reco_track)

            if is_matched:
                n_matched_mc_reco_tracks += 1
            elif is_merged:
                n_merged_mc_reco_tracks += 1
            elif is_missing:
                n_missing_mc_reco_tracks += 1

        # Aggregate information about pattern recognition tracks
        n_matched_reco_tracks = 0
        n_clone_reco_tracks = 0
        n_background_reco_tracks = 0
        n_ghost_reco_tracks = 0

        all_tracks_det_hit_ids = set()
        n_matched_hits = 0
        for reco_track in reco_tracks:
            is_matched = track_match_look_up.isMatchedPRRecoTrack(reco_track)
            is_clone = track_match_look_up.isClonePRRecoTrack(reco_track)
            is_background = track_match_look_up.isBackgroundPRRecoTrack(reco_track)
            is_ghost = track_match_look_up.isGhostPRRecoTrack(reco_track)

            if is_matched:
                n_matched_reco_tracks += 1
            elif is_clone:
                n_clone_reco_tracks += 1
            elif is_background:
                n_background_reco_tracks += 1
            elif is_ghost:
                n_ghost_reco_tracks += 1

            reco_track_det_hit_ids = utilities.get_det_hit_ids(reco_track)

            all_tracks_det_hit_ids.update(reco_track_det_hit_ids)
            if is_matched or is_clone:
                mc_reco_track = self.track_match_look_up.getRelatedMCRecoTrack(reco_track)

                mc_reco_track_det_hit_ids = utilities.get_det_hit_ids(mc_reco_track)
                n_matched_hits += len(reco_track_det_hit_ids & mc_reco_track_det_hit_ids)

        return dict(
            n_mc_particles=n_mc_particles,
            n_mc_reco_tracks=n_mc_reco_tracks,
            n_reco_tracks=n_reco_tracks,

            n_matched_mc_reco_tracks=n_matched_mc_reco_tracks,
            n_merged_mc_reco_tracks=n_merged_mc_reco_tracks,
            n_missing_mc_reco_tracks=n_missing_mc_reco_tracks,

            n_matched_reco_tracks=n_matched_reco_tracks,
            n_clone_reco_tracks=n_clone_reco_tracks,
            n_background_reco_tracks=n_background_reco_tracks,
            n_ghost_reco_tracks=n_ghost_reco_tracks,

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
                                   above_expert_level=default_expert_level)

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
