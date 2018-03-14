#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>matching_validation.root</output>
  <input>EvtGenSim.root</input>
  <description>This module generates events for the validation using the full tracking with a tabular output.</description>
</header>
"""
import basf2
from ROOT import Belle2

import tracking
from tracking.harvest.peelers import peel_reco_track_hit_content, peel_mc_particle, peel_fit_status

from tracking.harvest.harvesting import HarvestingModule
from tracking.harvest import refiners


class VxdCdcPartFinderHarvester(HarvestingModule):
    """
    Harvester module to extract the information, if a MCTrack was found by the CDC
    and/or the VXD tracking reconstruction.
    """

    def __init__(self, output_file_name):
        """
        Init harvester
        """
        HarvestingModule.__init__(self, foreach="MCRecoTracks", output_file_name=output_file_name)

        #: matcher used for the MCTracks from the CDC
        self.mc_track_matcher_cdc = Belle2.TrackMatchLookUp("MCRecoTracks", "CDCRecoTracks")
        #: matcher used for the MCTracks from the VXD
        self.mc_track_matcher_vxd = Belle2.TrackMatchLookUp("MCRecoTracks", "SVDRecoTracks")
        #: matcher used for the MCTracks from both
        self.mc_track_matcher = Belle2.TrackMatchLookUp("MCRecoTracks", "RecoTracks")
        #: matcher used for the MCTracks from fitted tracks
        self.fitted_mc_track_matcher = Belle2.TrackMatchLookUp("MCRecoTracks", "FittedRecoTracks")

    def peel(self, mc_track):
        """
        Extract the information.
        """
        this_best_track_cdc = self.mc_track_matcher_cdc.getMatchedPRRecoTrack(mc_track)
        this_best_track_vxd = self.mc_track_matcher_vxd.getMatchedPRRecoTrack(mc_track)
        reco_track = self.mc_track_matcher.getMatchedPRRecoTrack(mc_track)

        mc_particle = mc_track.getRelated("MCParticles")

        return_dict = {
            "vxd_was_found": bool(this_best_track_vxd),
            "cdc_was_found": bool(this_best_track_cdc),
            "cdc_has_related": False,
            "vxd_has_related": False,
            "both_related": False,
        }

        if this_best_track_vxd and this_best_track_cdc:
            return_dict["both_related"] = this_best_track_cdc.getRelated("VXDRecoTracks") == this_best_track_vxd

        if this_best_track_vxd:
            return_dict["vxd_has_related"] = bool(this_best_track_vxd.getRelated("CDCRecoTracks"))
        if this_best_track_cdc:
            return_dict["cdc_has_related"] = bool(this_best_track_cdc.getRelated("VXDRecoTracks"))

        return_dict.update(peel_reco_track_hit_content(mc_track))
        return_dict.update(peel_mc_particle(mc_particle))

        return_dict.update(dict(
            is_matched=self.mc_track_matcher.isMatchedMCRecoTrack(mc_track),
            is_merged=self.mc_track_matcher.isMergedMCRecoTrack(mc_track),
            is_missing=self.mc_track_matcher.isMissingMCRecoTrack(mc_track),
            hit_efficiency=self.mc_track_matcher.getRelatedEfficiency(mc_track),
        ))

        return_dict.update(dict(
            fitted_is_matched=self.fitted_mc_track_matcher.isMatchedMCRecoTrack(mc_track),
            fitted_is_merged=self.fitted_mc_track_matcher.isMergedMCRecoTrack(mc_track),
            fitted_is_missing=self.fitted_mc_track_matcher.isMissingMCRecoTrack(mc_track),
            fitted_hit_efficiency=self.fitted_mc_track_matcher.getRelatedEfficiency(mc_track),
        ))

        return return_dict

    #: Store as a table in a ROOT file
    save_tree = refiners.SaveTreeRefiner()


def run():
    path = basf2.create_path()

    # Read in the pre generated events
    path.add_module('RootInput', inputFileName='../EvtGenSim.root')
    path.add_module("Gearbox")

    # Add the tracking reconstruction and store the fitted RecoTracks elsewhere
    tracking.add_tracking_reconstruction(path, prune_temporary_tracks=False, components=["SVD", "CDC"])
    path.add_module("FittedTracksStorer", inputRecoTracksStoreArrayName="RecoTracks",
                    outputRecoTracksStoreArrayName="FittedRecoTracks")

    # Also do a matching for all RecoTracks in sub detectors
    path.add_module(
        "MCRecoTracksMatcher",
        UsePXDHits=False,
        UseSVDHits=False,
        UseCDCHits=True,
        mcRecoTracksStoreArrayName="MCRecoTracks",
        prRecoTracksStoreArrayName="CDCRecoTracks")
    path.add_module(
        "MCRecoTracksMatcher",
        UsePXDHits=True,
        UseSVDHits=True,
        UseCDCHits=False,
        mcRecoTracksStoreArrayName="MCRecoTracks",
        prRecoTracksStoreArrayName="SVDRecoTracks")

    path.add_module(
        "MCRecoTracksMatcher",
        UsePXDHits=True,
        UseSVDHits=True,
        UseCDCHits=True,
        mcRecoTracksStoreArrayName="MCRecoTracks",
        prRecoTracksStoreArrayName="FittedRecoTracks")

    # Gather the results into ROOT files
    path.add_module(VxdCdcPartFinderHarvester("../matching_validation.root"))

    path.add_module("ProgressBar")
    basf2.process(path)
    print(basf2.statistics)


if __name__ == "__main__":
    run()
