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
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule

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
        self.mc_track_matcher_vxd = Belle2.TrackMatchLookUp("MCRecoTracks", "VXDRecoTracks")

    def peel(self, mc_track):
        """
        Extract the information.
        """
        this_best_track_cdc = self.mc_track_matcher_cdc.getMatchedPRRecoTrack(mc_track)
        this_best_track_vxd = self.mc_track_matcher_vxd.getMatchedPRRecoTrack(mc_track)

        return {
            "vxd_was_found": this_best_track_vxd is not None,
            "cdc_was_found": this_best_track_cdc is not None
        }

    #: Store as a table in a ROOT file
    save_tree = refiners.SaveTreeRefiner()


def run():
    path = basf2.create_path()

    # Read in the pre generated events
    path.add_module('RootInput', inputFileName='../EvtGenSim.root')
    path.add_module("Gearbox")

    # Add the tracking reconstruction and store the fitted RecoTracks elsewhere
    tracking.add_tracking_reconstruction(path, keep_temporary_tracks=True)
    path.add_module("FittedTracksStorer", inputRecoTracksStoreArrayName="RecoTracks",
                    outputRecoTracksStoreArrayName="FittedRecoTracks")

    # Also do a matching for all RecoTracks in sub detectors
    path.add_module(
        "MCRecoTracksMatcher",
        UsePXDHits=False,
        UseSVDHits=False,
        mcRecoTracksStoreArrayName="MCRecoTracks",
        prRecoTracksStoreArrayName="CDCRecoTracks")
    path.add_module(
        "MCRecoTracksMatcher",
        UsePXDHits=True,
        UseCDCHits=False,
        mcRecoTracksStoreArrayName="MCRecoTracks",
        prRecoTracksStoreArrayName="VXDRecoTracks")
    path.add_module(
        "MCRecoTracksMatcher",
        UsePXDHits=True,
        UseSVDHits=True,
        mcRecoTracksStoreArrayName="MCRecoTracks",
        prRecoTracksStoreArrayName="FittedRecoTracks")

    # Gather the results into ROOT files
    path.add_module(VxdCdcPartFinderHarvester("../matching_validation.root"))
    path.add_module(
        CombinedTrackingValidationModule(
            name="",
            contact="",
            output_file_name="../tracking_validation.root",
            expert_level=200,
            reco_tracks_name="FittedRecoTracks"))
    path.add_module(
        CombinedTrackingValidationModule(
            name="",
            contact="",
            output_file_name="../tracking_validation_prefit.root",
            expert_level=200,
            reco_tracks_name="RecoTracks"))

    basf2.process(path)

if __name__ == "__main__":
    run()
