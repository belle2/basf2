from tracking.harvest import peelers, refiners
from tracking.harvest.harvesting import HarvestingModule

from ROOT import Belle2


class VxdCdcMergerHarvester(HarvestingModule):
    def __init__(self, output_file_name):
        """
        Init harvester
        """
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name=output_file_name)

        #: matcher used for the MCTracks from the CDC
        self.mc_track_matcher_cdc = Belle2.TrackMatchLookUp("MCRecoTracks", "CDCRecoTracks")
        #: matcher used for the MCTracks from the VXD
        self.mc_track_matcher_vxd = Belle2.TrackMatchLookUp("MCRecoTracks", "SVDRecoTracks")

    def pick(self, mc_particle):
        # mc_track = mc_particle.getRelated("MCRecoTracks")
        # return mc_track is not None
        mc_track = mc_particle.getRelatedFrom("MCRecoTracks")
        if mc_track:
            return True
        else:
            return False

    def peel(self, mc_particle):
        mc_track = mc_particle.getRelatedFrom("MCRecoTracks")

        event_info = Belle2.PyStoreObj("EventMetaData")

        result = peelers.peel_mc_particle(mc_particle)
        result.update(peelers.peel_reco_track_hit_content(mc_track))
        result.update(peelers.peel_event_info(event_info))

        this_best_track_cdc = self.mc_track_matcher_cdc.getMatchedPRRecoTrack(mc_track)
        this_best_track_vxd = self.mc_track_matcher_vxd.getMatchedPRRecoTrack(mc_track)

        if this_best_track_cdc is not None:
            merged_vxd_track = this_best_track_cdc.getRelated("SVDRecoTracks")
        else:
            merged_vxd_track = None
        if this_best_track_vxd is not None:
            merged_cdc_track = this_best_track_vxd.getRelated("CDCRecoTracks")
        else:
            merged_cdc_track = None

        flag_incorrect_not_merged = False
        flag_correct_merged = False
        flag_not_found = False
        flag_correct_not_merged_cdcmissing = False
        flag_incorrect_merged_cdcwrong_although_not_there = False
        flag_correct_not_merged_vxdmissing = False
        flag_incorrect_merged_vxdwrong_although_not_there = False

        flag_incorrect_merged = False

        # Separate out *all* different cases!

        # No tracks exists at all --> Problem of Trackfinder
        if this_best_track_cdc is None and this_best_track_vxd is None:
            # Case 7
            flag_not_found = True

        # CDC track not existing
        elif this_best_track_cdc is None:
            # Cross-check: merged track == best track !!
            if merged_cdc_track is None:
                # Case 4
                flag_correct_not_merged_cdcmissing = True
            # If merged track is not empty, merger made a mistake
            else:
                # Case 2 !!
                flag_incorrect_merged_cdcwrong_although_not_there = True

        # VXD track not existing
        elif this_best_track_vxd is None:
            # Cross check> merged track == best track!!
            if merged_vxd_track is None:
                # Case 5
                flag_correct_not_merged_vxdmissing = True
            # If merged track i is not emtpy, merger made a mistake
            else:
                # Case 3 !!
                flag_incorrect_merged_vxdwrong_although_not_there = True

        # Both tracks exists but have not been merged, although they should have
        elif merged_vxd_track is None and merged_cdc_track is None:
            # Case 6
            flag_incorrect_not_merged = True

        # Tracks are merged correctly
        elif this_best_track_cdc == merged_cdc_track and this_best_track_vxd == merged_vxd_track:
            # Case 1
            flag_correct_merged = True

        # If all operations above have failed than the merger failed
        else:
            # Case 2 + 3 !!
            if merged_cdc_track != this_best_track_cdc:
                flag_incorrect_merged = True
                # The other combinations must then also be false

        # We miss the unmatched cdc/vxd reco tracks which are fakes!!!

        result.update({
            "flag_correct_not_merged_cdcmissing": flag_correct_not_merged_cdcmissing,
            "flag_correct_not_merged_vxdmissing": flag_correct_not_merged_vxdmissing,
            "flag_correct_merged": flag_correct_merged,
            "flag_incorrect_not_merged": flag_incorrect_not_merged,
            "flag_incorrect_merged": flag_incorrect_merged,
            "flag_incorrect_merged_cdcwrong_although_not_there": flag_incorrect_merged_cdcwrong_although_not_there,
            "flag_incorrect_merged_vxdwrong_although_not_there": flag_incorrect_merged_vxdwrong_although_not_there,
            "flag_not_found": flag_not_found,
        })

        return result

    save_tree = refiners.SaveTreeRefiner()
