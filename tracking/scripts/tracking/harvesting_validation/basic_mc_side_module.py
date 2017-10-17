import basf2

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2

import warnings

import numpy as np

import tracking.validation.utilities as utilities

import tracking.harvest.refiners as refiners
import tracking.harvest.harvesting as harvesting
import tracking.harvest.peelers as peelers


class BasicMCSideTrackingValidationModule(harvesting.HarvestingModule):

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

    def initialize(self):
        super().initialize()
        self.track_match_look_up = Belle2.TrackMatchLookUp(self.mc_reco_tracks_name, self.reco_tracks_name)

    def prepare(self):
        """Collect some statistics about the pattern recognition tracks used for comparision to the MC tracks

        Executed once at the start of each event.
        """
        super().prepare()

    def pick(self, mc_reco_track):
        return True

    def peel(self, mc_reco_track):
        """Looks at the individual Monte Carlo tracks and store information about them"""
        track_match_look_up = self.track_match_look_up

        # Analyse from the Monte Carlo reference side
        mc_reco_tracks = Belle2.PyStoreArray(self.foreach)
        multiplicity = mc_reco_tracks.getEntries()

        mc_to_pr_match_info_crops = self.peel_mc_to_pr_match_info(mc_reco_track)
        mc_store_array_crops = peelers.peel_store_array_info(mc_reco_track, key="mc_{part_name}")

        crops = dict(multiplicity=multiplicity,
                     **mc_to_pr_match_info_crops,
                     **mc_store_array_crops
                     )

        return crops

    def peel_mc_to_pr_match_info(self, mc_reco_track):
        track_match_look_up = self.track_match_look_up
        return dict(
            is_matched=track_match_look_up.isMatchedMCRecoTrack(mc_reco_track),
            is_merged=track_match_look_up.isMergedMCRecoTrack(mc_reco_track),
            is_missing=track_match_look_up.isMissingMCRecoTrack(mc_reco_track),
            hit_efficiency=track_match_look_up.getRelatedEfficiency(mc_reco_track),
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
        filter_on="is_primary",
        description="""
finding efficiency - the ratio of matched primary Monte Carlo tracks to all Monte Carlo tracks
hit efficiency - the ratio of hits picked up by the matched pattern recognition track of primary Monte Carlo tracks
"""
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
