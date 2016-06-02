#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>VxdCdcValidationHarvested.root</output>
  <input>EvtGenSimNoBkg.root</input>
  <description>This module generates events for the V0 validation.</description>
</header>
"""

import basf2
from ROOT import Belle2
from simulation import add_simulation
from reconstruction import add_mdst_output, add_reconstruction
from modularAnalysis import generateY4S
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners, tracking_efficiency_helpers
import numpy


def run():
    """
    Generate and reconstruct Generic BBar
    and evaluate the merger performance
    """
    components = tracking_efficiency_helpers.get_simulation_components()

    basf2.set_random_seed(1337)
    path = basf2.create_path()

    rootinput = basf2.register_module('RootInput')
    rootinput.param('inputFileName', "../EvtGenSimNoBkg.root")
    path.add_module(rootinput)
    path.add_module('Gearbox')

    add_reconstruction(path, components=components, pruneTracks=False)

    # convert to TrackCands so tracks can be matched
    path.add_module("GenfitTrackCandidatesCreator", recoTracksStoreArrayName='VXDRecoTracks',
                    genfitTrackCandsStoreArrayName='VXDGFTrackCands')
    path.add_module("GenfitTrackCandidatesCreator", recoTracksStoreArrayName='CDCRecoTracks',
                    genfitTrackCandsStoreArrayName='CDCGFTrackCands')

    # add additional matching for vxd and cdc only tracks here

    # for VXD only track
    mctrackmatcher_vxd = basf2.register_module('MCTrackMatcher')
    mctrackmatcher_vxd.param('MCGFTrackCandsColName', 'MCTrackCands')
    mctrackmatcher_vxd.param('PRGFTrackCandsColName', 'VXDGFTrackCands')
    mctrackmatcher_vxd.param('UseCDCHits', False)
    path.add_module(mctrackmatcher_vxd)

    # for CDC only tracks
    mctrackmatcher_cdc = basf2.register_module('MCTrackMatcher')
    mctrackmatcher_cdc.param('MCGFTrackCandsColName', 'MCTrackCands')
    mctrackmatcher_cdc.param('PRGFTrackCandsColName', 'CDCGFTrackCands')
    mctrackmatcher_cdc.param('UsePXDHits', False)
    mctrackmatcher_cdc.param('UseSVDHits', False)
    path.add_module(mctrackmatcher_cdc)

    path.add_module(VxdCdcMergerHarvester())

    basf2.process(path)
    print(basf2.statistics)


class VxdCdcMergerHarvester(HarvestingModule):

    def __init__(self):
        """
        Init harvester
        """
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name="../VxdCdcValidationHarvested.root")

        #: matcher used for the final MCTrack list
        self.mc_track_matcher = Belle2.TrackMatchLookUp("MCTrackCands")
        #: matcher used for the MCTracks from the CDC
        self.mc_track_matcher_cdc = Belle2.TrackMatchLookUp("MCTrackCands", "CDCGFTrackCands")
        #: matcher used for the MCTracks from the VXD
        self.mc_track_matcher_vxd = Belle2.TrackMatchLookUp("MCTrackCands", "VXDGFTrackCands")

    def pick(self, mc_particle):
        # mc_track = mc_particle.getRelated("MCTrackCands")
        # return mc_track is not None
        mc_track = mc_particle.getRelatedFrom("MCTrackCands")
        if mc_track:
            return True
        else:
            return False

    def peel(self, mc_particle):
        # mc is a genfit::TrackCand

        mc_track = mc_particle.getRelatedFrom("MCTrackCands")

        mc_nhits = mc_track.getNHits()
        mc_pt = mc_particle.getMomentum().Pt()
        mc_theta = mc_particle.getMomentum().Theta()

        reco_pxd_hits = Belle2.PyStoreArray('PXDRecoHits')
        reco_svd_hits = Belle2.PyStoreArray('SVDRecoHits')
        reco_cdc_hits = Belle2.PyStoreArray('CDCHits')

        reco_tracks = Belle2.PyStoreArray('TrackCands')

        this_best_track_cdc = self.mc_track_matcher_cdc.getRelatedPRTrackCand(mc_track)
        this_best_track_vxd = self.mc_track_matcher_vxd.getRelatedPRTrackCand(mc_track)

        # here we know, the above tracks should have been merged because they stem
        # from the same MC Track. Have they?

        this_best_track_merged = self.mc_track_matcher.getRelatedPRTrackCand(mc_track)

        # todo: once moved to RecoTrack: use more elaborate comparison method to indentify vxd/cdc tracks
        # in merged tracks
        # cdc_merge_fraction = sameHitFractionCdc( this_best_track_merged, this_best_track_cdc, reco_cdc_hits )
        # vxd_merge_fraction = sameHitFractionVxd( this_best_track_merged, this_best_track_vxd, reco_pxd_hits, reco_svd_hits)

        good_merge = False
        vxd_hits = 0
        cdc_hits = 0

        merged_hits = 0

        if this_best_track_merged:
            if this_best_track_cdc:
                cdc_hits = this_best_track_cdc.getNHits()
            if this_best_track_vxd:
                vxd_hits = this_best_track_vxd.getNHits()

            merged_hits = vxd_hits + cdc_hits
            good_merge = merged_hits == this_best_track_merged.getNHits()

        return {
            "MC_PT": mc_pt,
            "MC_THETA": mc_theta,
            "MC_NHITS": mc_nhits,
            "PR_NHITS": merged_hits,
            "PR_CDC_NHITS": cdc_hits,
            "PR_VXD_NHITS": vxd_hits,
            "GOOD_MERGE": good_merge
        }

    save_tree = refiners.SaveTreeRefiner()


if __name__ == '__main__':
    run()
