from tracking.harvest import peelers, refiners
from tracking.harvest.harvesting import HarvestingModule

from ROOT import Belle2

from tracking.harvest.peelers import format_crop_keys


class VxdCdcMergerHarvesterMCSide(HarvestingModule):
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
        # return mc_track != None
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

        result["mc_store_array_number"] = mc_track.getArrayIndex()

        this_best_track_cdc = self.mc_track_matcher_cdc.getMatchedPRRecoTrack(mc_track)
        this_best_track_vxd = self.mc_track_matcher_vxd.getMatchedPRRecoTrack(mc_track)

        result.update(peelers.peel_reco_track_hit_content(this_best_track_cdc, key="cdc_{part_name}"))
        result.update(peelers.peel_reco_track_seed(this_best_track_cdc, key="cdc_{part_name}"))
        if this_best_track_cdc:
            result["cdc_store_array_number"] = this_best_track_cdc.getArrayIndex()
            result["cdc_was_fitted"] = this_best_track_cdc.wasFitSuccessful()
        else:
            result["cdc_store_array_number"] = -1
            result["cdc_was_fitted"] = -1

        result.update(peelers.peel_reco_track_hit_content(this_best_track_vxd, key="vxd_{part_name}"))
        result.update(peelers.peel_reco_track_seed(this_best_track_vxd, key="vxd_{part_name}"))
        if this_best_track_vxd:
            result["vxd_store_array_number"] = this_best_track_vxd.getArrayIndex()
            result["vxd_was_fitted"] = this_best_track_vxd.wasFitSuccessful()
        else:
            result["vxd_store_array_number"] = -1
            result["vxd_was_fitted"] = -1

        if this_best_track_cdc:
            merged_vxd_track = this_best_track_cdc.getRelated("SVDRecoTracks")
        else:
            merged_vxd_track = None
        if this_best_track_vxd:
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
        if not this_best_track_cdc and not this_best_track_vxd:
            # Case 7
            flag_not_found = True

        # CDC track not existing
        elif not this_best_track_cdc:
            # Cross-check: merged track == best track !!
            if not merged_cdc_track:
                # Case 4
                flag_correct_not_merged_cdcmissing = True
            # If merged track is not empty, merger made a mistake
            else:
                # Case 2 !!
                flag_incorrect_merged_cdcwrong_although_not_there = True

        # VXD track not existing
        elif not this_best_track_vxd:
            # Cross check> merged track == best track!!
            if not merged_vxd_track:
                # Case 5
                flag_correct_not_merged_vxdmissing = True
            # If merged track i is not emtpy, merger made a mistake
            else:
                # Case 3 !!
                flag_incorrect_merged_vxdwrong_although_not_there = True

        # Both tracks exists but have not been merged, although they should have
        elif not merged_vxd_track and not merged_cdc_track:
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


@format_crop_keys
def peel_matching_information(pr_track_and_mc_track_matcher, key="{part_name}"):
    pr_track, mc_track_matcher = pr_track_and_mc_track_matcher
    if pr_track:
        is_matched = mc_track_matcher.isMatchedPRRecoTrack(pr_track)
        is_clone = mc_track_matcher.isClonePRRecoTrack(pr_track)
        is_background = mc_track_matcher.isBackgroundPRRecoTrack(pr_track)
        is_ghost = mc_track_matcher.isGhostPRRecoTrack(pr_track)

        return dict(
            is_matched=is_matched,
            is_clone=is_clone,
            is_background=is_background,
            is_ghost=is_ghost,
            is_clone_or_match=(is_matched or is_clone),
            is_fake=not (is_matched or is_clone),
            hit_purity=mc_track_matcher.getRelatedPurity(pr_track),
        )
    else:
        nan = float("nan")
        return dict(
            is_matched=nan,
            is_clone=nan,
            is_background=nan,
            is_ghost=nan,
            is_clone_or_match=nan,
            is_fake=nan,
            hit_purity=nan,
        )


class VxdCdcMergerHarvesterPRSide(HarvestingModule):
    def __init__(self, foreach, others, output_file_name):
        super().__init__(foreach, output_file_name)

        self.others = others

        self.mc_track_matcher = Belle2.TrackMatchLookUp("MCRecoTracks", foreach)
        self.mc_track_matcher_other = Belle2.TrackMatchLookUp("MCRecoTracks", others)

    def peel(self, pr_track):
        event_info = Belle2.PyStoreObj("EventMetaData")

        result = peelers.peel_reco_track_hit_content(pr_track)
        result.update(peelers.peel_reco_track_seed(pr_track))
        result.update(peelers.peel_event_info(event_info))

        result["store_array_number"] = pr_track.getArrayIndex()

        mc_track = self.mc_track_matcher.getMatchedMCRecoTrack(pr_track)

        if mc_track:
            result["mc_store_array_number"] = mc_track.getArrayIndex()
        else:
            result["mc_store_array_number"] = -1

        result.update(peel_matching_information((pr_track, self.mc_track_matcher)))

        matched_track = pr_track.getRelated(self.others)

        result.update(peel_matching_information((matched_track, self.mc_track_matcher_other), key="other_{part_name}"))

        if matched_track:
            result["matched_store_array_number"] = matched_track.getArrayIndex()
        else:
            result["matched_store_array_number"] = -1

        result["number_of_matches"] = pr_track.getRelationsWith(self.others).size()

        return result

    save_tree = refiners.SaveTreeRefiner()
