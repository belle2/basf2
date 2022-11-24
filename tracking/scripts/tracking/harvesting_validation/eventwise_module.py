##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import ROOT
from ROOT import Belle2

import numpy as np

import tracking.validation.utilities as utilities

import tracking.harvest.refiners as refiners
import tracking.harvest.harvesting as harvesting
ROOT.gSystem.Load("libtracking")


class EventwiseTrackingValidationModule(harvesting.HarvestingModule):
    """Module to perform event-by-event tracking validation."""

    """ Expert level behavior:
        expert_level <= default_expert_level: all figures and plots from this module except tree entries
        expert_level > default_expert_level: everything including tree entries
    """

    #: the threshold value for the expert level
    default_expert_level = 10

    def __init__(self,
                 name,
                 contact,
                 output_file_name=None,
                 reco_tracks_name='RecoTracks',
                 mc_reco_tracks_name='MCRecoTracks',
                 expert_level=None):
        """Constructor"""

        output_file_name = output_file_name or name + 'TrackingValidation.root'

        super().__init__(foreach="EventMetaData",  # Dummy for on element per event
                         name=name,
                         output_file_name=output_file_name,
                         contact=contact,
                         expert_level=expert_level)

        #: cached value of the RecoTracks collection name
        self.reco_tracks_name = reco_tracks_name
        #: cached value of the MCRecoTracks collection name
        self.mc_reco_tracks_name = mc_reco_tracks_name
        #: cached value of the CDCHits collection name
        self.cdc_hits_name = "CDCHits"

    def initialize(self):
        """Initialization signal at the start of the event processing"""
        super().initialize()
        #: Reference to the track-match object that examines relation information from MCMatcherTracksModule
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_reco_tracks_name,
                                                           self.reco_tracks_name)

    def pick(self, event_meta_data=None):
        """Always pick"""
        return True

    def peel(self, event_meta_data=None):
        """Peel information from the event"""
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
        n_matched_correct_charge_mc_reco_tracks = 0
        n_matched_wrong_charge_mc_reco_tracks = 0
        n_merged_mc_reco_tracks = 0
        n_merged_correct_charge_mc_reco_tracks = 0
        n_merged_wrong_charge_mc_reco_tracks = 0
        n_missing_mc_reco_tracks = 0
        for mc_reco_track in mc_reco_tracks:
            mc_reco_track_det_hit_ids = utilities.get_det_hit_ids(mc_reco_track)
            all_mc_tracks_det_hit_ids.update(mc_reco_track_det_hit_ids)

            is_matched = track_match_look_up.isAnyChargeMatchedMCRecoTrack(mc_reco_track)
            is_matched_correct_charge = track_match_look_up.isCorrectChargeMatchedMCRecoTrack(mc_reco_track)
            is_matched_wrong_charge = track_match_look_up.isWrongChargeMatchedMCRecoTrack(mc_reco_track)
            is_merged = track_match_look_up.isAnyChargeMergedMCRecoTrack(mc_reco_track)
            is_merged_correct_charge = track_match_look_up.isCorrectChargeMergedMCRecoTrack(mc_reco_track)
            is_merged_wrong_charge = track_match_look_up.isWrongChargeMergedMCRecoTrack(mc_reco_track)
            is_missing = track_match_look_up.isMissingMCRecoTrack(mc_reco_track)

            if is_matched:
                n_matched_mc_reco_tracks += 1
                if is_matched_correct_charge:
                    n_matched_correct_charge_mc_reco_tracks += 1
                elif is_matched_wrong_charge:
                    n_matched_wrong_charge_mc_reco_tracks += 1
            elif is_merged:
                n_merged_mc_reco_tracks += 1
                if is_merged_correct_charge:
                    n_merged_correct_charge_mc_reco_tracks += 1
                elif is_merged_wrong_charge:
                    n_merged_wrong_charge_mc_reco_tracks += 1
            elif is_missing:
                n_missing_mc_reco_tracks += 1

        # Aggregate information about pattern recognition tracks
        n_matched_reco_tracks = 0
        n_matched_correct_charge_reco_tracks = 0
        n_matched_wrong_charge_reco_tracks = 0
        n_clone_reco_tracks = 0
        n_clone_correct_charge_reco_tracks = 0
        n_clone_wrong_charge_reco_tracks = 0
        n_background_reco_tracks = 0
        n_ghost_reco_tracks = 0

        all_tracks_det_hit_ids = set()
        n_matched_hits = 0
        for reco_track in reco_tracks:
            is_matched = track_match_look_up.isAnyChargeMatchedPRRecoTrack(reco_track)
            is_matched_correct_charge = track_match_look_up.isCorrectChargeMatchedPRRecoTrack(reco_track)
            is_matched_wrong_charge = track_match_look_up.isWrongChargeMatchedPRRecoTrack(reco_track)
            is_clone = track_match_look_up.isAnyChargeClonePRRecoTrack(reco_track)
            is_clone_correct_charge = track_match_look_up.isCorrectChargeClonePRRecoTrack(reco_track)
            is_clone_wrong_charge = track_match_look_up.isWrongChargeClonePRRecoTrack(reco_track)
            is_background = track_match_look_up.isBackgroundPRRecoTrack(reco_track)
            is_ghost = track_match_look_up.isGhostPRRecoTrack(reco_track)

            if is_matched:
                n_matched_reco_tracks += 1
                if is_matched_correct_charge:
                    n_matched_correct_charge_reco_tracks += 1
                elif is_matched_wrong_charge:
                    n_matched_wrong_charge_reco_tracks += 1
            elif is_clone:
                n_clone_reco_tracks += 1
                if is_clone_correct_charge:
                    n_clone_correct_charge_reco_tracks += 1
                elif is_clone_wrong_charge:
                    n_clone_wrong_charge_reco_tracks += 1
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
            n_matched_correct_charge_mc_reco_tracks=n_matched_correct_charge_mc_reco_tracks,
            n_matched_wrong_charge_mc_reco_tracks=n_matched_wrong_charge_mc_reco_tracks,

            n_merged_mc_reco_tracks=n_merged_mc_reco_tracks,
            n_merged_correct_charge_mc_reco_tracks=n_merged_correct_charge_mc_reco_tracks,
            n_merged_wrong_charge_mc_reco_tracks=n_merged_wrong_charge_mc_reco_tracks,

            n_missing_mc_reco_tracks=n_missing_mc_reco_tracks,


            n_matched_reco_tracks=n_matched_reco_tracks,
            n_matched_correct_charge_reco_tracks=n_matched_correct_charge_reco_tracks,
            n_matched_wrong_charge_reco_tracks=n_matched_wrong_charge_reco_tracks,
            n_clone_reco_tracks=n_clone_reco_tracks,
            n_clone_correct_charge_reco_tracks=n_clone_correct_charge_reco_tracks,
            n_clone_wrong_charge_reco_tracks=n_clone_wrong_charge_reco_tracks,

            n_background_reco_tracks=n_background_reco_tracks,
            n_ghost_reco_tracks=n_ghost_reco_tracks,

            n_cdc_hits=cdc_hits.getEntries(),
            n_all_mc_track_hits=len(all_mc_tracks_det_hit_ids),
            n_all_track_hits=len(all_tracks_det_hit_ids),
            n_found_hits=len(all_mc_tracks_det_hit_ids & all_tracks_det_hit_ids),
            n_matched_hits=n_matched_hits
        )

    # Refiners to be executed on terminate #
    # ==================================== #

    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(
        # using cond to suppress false doxygen warnings
        #: \cond
        folder_name="event_tree",
        name="event_tree",
        above_expert_level=default_expert_level
        #: \endcond
    )

    #: Save selected hit counters
    save_clone_rate = refiners.save_fom(
        # using cond to suppress false doxygen warnings
        #: \cond
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
        #: \endcond
    )
