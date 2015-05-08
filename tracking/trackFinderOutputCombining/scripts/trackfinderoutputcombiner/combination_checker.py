import basf2
from ROOT import Belle2


class CombinationChecker:

    def __init__(
            self,
            track_cands_1_store_array_name=None,
            track_cands_2_store_array_name=None,
            mc_track_cands_store_array_name=None):
        self.track_cands_1_store_array_name = track_cands_1_store_array_name
        self.track_cands_2_store_array_name = track_cands_2_store_array_name
        self.mc_track_cands_store_array_name = mc_track_cands_store_array_name

        self.mc_track_matcher_1 = Belle2.TrackMatchLookUp(self.mc_track_cands_store_array_name,
                                                          self.track_cands_1_store_array_name)
        self.mc_track_matcher_2 = Belle2.TrackMatchLookUp(self.mc_track_cands_store_array_name,
                                                          self.track_cands_2_store_array_name)

        self.track_cands_1 = Belle2.PyStoreArray(self.track_cands_1_store_array_name)
        self.track_cands_2 = Belle2.PyStoreArray(self.track_cands_2_store_array_name)

    def belong_to_same_mc_track(self, track_cand_1, track_cand_2):
        if not self.is_matchable(track_cand_1, track_cand_2):
            return False

        mc_track_matcher_1 = self.mc_track_matcher_1
        mc_track_matcher_2 = self.mc_track_matcher_2

        # can not be null pointers because is_matchable
        mc_track_for_track_cand_1 = mc_track_matcher_1.getMatchedMCTrackCand(track_cand_1)
        mc_track_for_track_cand_2 = mc_track_matcher_2.getMatchedMCTrackCand(track_cand_2)

        return mc_track_for_track_cand_1 == mc_track_for_track_cand_2

    def is_matchable(self, track_cand_1, track_cand_2):
        if track_cand_1 not in self.track_cands_1 or track_cand_2 not in self.track_cands_2:
            raise IndexError

        mc_track_matcher_1 = self.mc_track_matcher_1
        mc_track_matcher_2 = self.mc_track_matcher_2

        if not mc_track_matcher_1.isMatchedPRTrackCand(track_cand_1) or not mc_track_matcher_2.isMatchedPRTrackCand(track_cand_2):
            return False

        return True
