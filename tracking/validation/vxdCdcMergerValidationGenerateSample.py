#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <output>VxdCdcValidationHarvested.root</output>
  <input>EvtGenSimNoBkg.root</input>
  <description>This module generates events for the V0 validation.</description>
</header>
"""

import basf2
from ROOT import Belle2
from tracking import add_tracking_reconstruction
from tracking.harvest.harvesting import HarvestingModule
from tracking.harvest import refiners

ACTIVE = True


def run():
    """
    Generate and reconstruct Generic BBar
    and evaluate the merger performance
    """
    basf2.set_random_seed(1337)
    path = basf2.create_path()

    rootinput = basf2.register_module('RootInput')
    rootinput.param('inputFileName', "../EvtGenSimNoBkg.root")
    path.add_module(rootinput)
    path.add_module('Gearbox')

    add_tracking_reconstruction(path, components=["CDC", "SVD"], pruneTracks=False, prune_temporary_tracks=False)

    # add additional matching for vxd and cdc only tracks here

    # for VXD only track
    mctrackmatcher_vxd = basf2.register_module('MCRecoTracksMatcher')
    mctrackmatcher_vxd.param('mcRecoTracksStoreArrayName', 'MCRecoTracks')
    mctrackmatcher_vxd.param('prRecoTracksStoreArrayName', 'SVDRecoTracks')
    mctrackmatcher_vxd.param('UseCDCHits', False)
    path.add_module(mctrackmatcher_vxd)

    # for CDC only tracks
    mctrackmatcher_cdc = basf2.register_module('MCRecoTracksMatcher')
    mctrackmatcher_cdc.param('mcRecoTracksStoreArrayName', 'MCRecoTracks')
    mctrackmatcher_cdc.param('prRecoTracksStoreArrayName', 'CDCRecoTracks')
    mctrackmatcher_cdc.param('UsePXDHits', False)
    mctrackmatcher_cdc.param('UseSVDHits', False)
    path.add_module(mctrackmatcher_cdc)

    path.add_module(VxdCdcMergerHarvester())

    path.add_module('Progress')
    basf2.process(path)
    print(basf2.statistics)


class VxdCdcMergerHarvester(HarvestingModule):
    """
    Harvester module to extract useful information from VXD-CDC track merging.
    """

    def __init__(self):
        """
        Init harvester
        """
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name="../VxdCdcValidationHarvested.root")

        #: matcher used for the final MCTrack list
        self.mc_track_matcher = Belle2.TrackMatchLookUp("MCRecoTracks")
        #: matcher used for the MCTracks from the CDC
        self.mc_track_matcher_cdc = Belle2.TrackMatchLookUp("MCRecoTracks", "CDCRecoTracks")
        #: matcher used for the MCTracks from the VXD
        self.mc_track_matcher_vxd = Belle2.TrackMatchLookUp("MCRecoTracks", "SVDRecoTracks")

    def pick(self, mc_particle):
        """Select the MCParticle if it is related to an MCRecoTrack"""
        # mc_track = mc_particle.getRelated("MCRecoTracks")
        # return mc_track is not None
        mc_track = mc_particle.getRelatedFrom("MCRecoTracks")
        if mc_track:
            return True
        else:
            return False

    def peel(self, mc_particle):
        """
        Extract the information.
        """

        # mc is a genfit::TrackCand

        mc_track = mc_particle.getRelatedFrom("MCRecoTracks")

        mc_nhits = mc_track.getNumberOfTotalHits()
        mc_pt = mc_particle.getMomentum().Rho()
        mc_theta = mc_particle.getMomentum().Theta()

        # reco_tracks = Belle2.PyStoreArray('RecoTracks')

        this_best_track_cdc = self.mc_track_matcher_cdc.getRelatedPRRecoTrack(mc_track)
        this_best_track_vxd = self.mc_track_matcher_vxd.getRelatedPRRecoTrack(mc_track)

        # here we know, the above tracks should have been merged because they stem
        # from the same MC Track. Have they?

        this_best_track_merged = self.mc_track_matcher.getRelatedPRRecoTrack(mc_track)

        good_merge = False
        vxd_hits = 0
        cdc_hits = 0

        merged_hits = 0

        if this_best_track_merged:
            if this_best_track_cdc:
                cdc_hits = this_best_track_cdc.getNumberOfTotalHits()
            if this_best_track_vxd:
                vxd_hits = this_best_track_vxd.getNumberOfTotalHits()

            merged_hits = vxd_hits + cdc_hits
            good_merge = merged_hits == this_best_track_merged.getNumberOfTotalHits()

        return {
            "MC_PT": mc_pt,
            "MC_THETA": mc_theta,
            "MC_NHITS": mc_nhits,
            "PR_NHITS": merged_hits,
            "PR_CDC_NHITS": cdc_hits,
            "PR_VXD_NHITS": vxd_hits,
            "GOOD_MERGE": good_merge
        }

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Store as a table in a ROOT file
    save_tree = refiners.SaveTreeRefiner()


if __name__ == '__main__':
    if ACTIVE:
        run()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
