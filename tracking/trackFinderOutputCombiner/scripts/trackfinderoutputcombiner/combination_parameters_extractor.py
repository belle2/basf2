import basf2
from ROOT import Belle2

from trackfinderoutputcombiner.combinerValidation import CombinerTrackFinderRun
from trackfinderoutputcombiner.combination_checker import CombinationChecker
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners


class CombinationParameterExtractorModule(HarvestingModule):

    def __init__(self, legendre_track_cands_store_array_name, local_track_cands_store_array_name, output_file_name):
        super(
            CombinationParameterExtractorModule,
            self).__init__(
            foreach=legendre_track_cands_store_array_name,
            output_file_name=output_file_name)
        self.foreach_other = local_track_cands_store_array_name

    # We need a doubleHarvester here
    def gather(self):
        registered_store_arrays = Belle2.PyStoreArray.list()
        registered_store_objs = Belle2.PyStoreObj.list()

        foreach = self.foreach
        foreach_other = self.foreach_other
        foreach_is_store_array = foreach in registered_store_arrays
        foreach_other_is_store_array = foreach_other in registered_store_arrays

        if foreach is not None and foreach_other is not None:
            if foreach_is_store_array and foreach_other_is_store_array:
                store_array = Belle2.PyStoreArray(self.foreach)
                store_array_other = Belle2.PyStoreArray(self.foreach_other)
                for crop in store_array:
                    for crop_other in store_array_other:
                        yield (crop, crop_other)
            else:
                raise KeyError(
                    "Name %s or %s does not refer to a valid store array on the data store" %
                    (self.foreach, self.foreach_other))
        else:
            yield None

    def prepare(self):
        self.combination_checker = CombinationChecker(track_cands_1_store_array_name=self.foreach,
                                                      track_cands_2_store_array_name=self.foreach_other,
                                                      mc_track_cands_store_array_name="MCTrackCands")
        return HarvestingModule.prepare(self)

    def pick(self, track_cands_tupel):
        (legendre_track_cand, local_track_cand) = track_cands_tupel
        return self.combination_checker.is_matchable(legendre_track_cand, local_track_cand)

    def peel(self, track_cands_tupel):
        (legendre_track_cand, local_track_cand) = track_cands_tupel
        belong_together = self.combination_checker.belong_to_same_mc_track(legendre_track_cand, local_track_cand)

        return dict(belong_together=belong_together)

    save_tree = refiners.save_tree(folder_name="tree")


class CombinationParameterExtractorRun(CombinerTrackFinderRun):

    def create_path(self):
        main_path = super(CombinationParameterExtractorRun, self).create_path()

        mc_track_matcher_module_legendre = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_legendre.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.legendre_track_cands_store_array_name
        })

        mc_track_matcher_module_local = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_local.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.local_track_cands_store_array_name
        })

        main_path.add_module(mc_track_matcher_module_legendre)
        main_path.add_module(mc_track_matcher_module_local)

        main_path.add_module(
            CombinationParameterExtractorModule(
                legendre_track_cands_store_array_name=self.legendre_track_cands_store_array_name,
                local_track_cands_store_array_name=self.local_track_cands_store_array_name,
                output_file_name="test.root"))

        return main_path

if __name__ == "__main__":
    run = CombinationParameterExtractorRun()
    run.configure_and_execute_from_commandline()
