from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import basf2

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.validation.module import SeparatedTrackingValidationModule


from trackfindingcdc.cdcLegendreTrackingValidation import ReassignHits


class AddValidationMethod:

    def create_matcher_module(self, track_candidates_store_array_name):
        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': track_candidates_store_array_name,
        })

        return mc_track_matcher_module

    def create_validation(self, main_path, track_candidates_store_array_name, output_file_name):
        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name=output_file_name,
            trackCandidatesColumnName=track_candidates_store_array_name,
            expert_level=2)

        main_path.add_module(self.create_matcher_module(track_candidates_store_array_name))
        main_path.add_module(validation_module)


class MCTrackFinderRun(StandardEventGenerationRun):
    # output track cands
    mc_track_cands_store_array_name = "MCTrackCands"

    def create_path(self):
        main_path = super(MCTrackFinderRun, self).create_path()

        track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruth')
        track_finder_mc_truth_module.param({
            'UseCDCHits': True,
            'WhichParticles': [],
            'GFTrackCandidatesColName': self.mc_track_cands_store_array_name,
        })

        main_path.add_module(track_finder_mc_truth_module)

        return main_path


class LegendreTrackFinderRun(MCTrackFinderRun):
    # output (splitted) track cands
    legendre_track_cands_store_array_name = "LegendreTrackCands"
    # output not assigned cdc hits
    not_assigned_cdc_hits_store_array_name = "NotAssignedCDCHits"
    # input tmva cut
    tmva_cut = 0.1
    # input flag if to split
    splitting = True
    # input flag if to use stereo assignment
    stereo_assignment = True

    def create_argument_parser(self, **kwds):
        argument_parser = super(LegendreTrackFinderRun, self).create_argument_parser(**kwds)
        argument_parser.add_argument(
            '-t',
            '--tmva-cut',
            dest='tmva_cut',
            default=self.tmva_cut,
            type=float,
            help='Cut for the TMVA in the module.'
        )

        argument_parser.add_argument(
            '--splitting',
            dest='splitting',
            action="store_true",
            help='Split the tracks before searching for not assigned hits.'
        )

        argument_parser.add_argument(
            '--no-splitting',
            dest='splitting',
            action="store_false",
            help='Split the tracks before searching for not assigned hits.'
        )

        argument_parser.set_defaults(splitting=self.splitting)

        return argument_parser

    def create_path(self):
        main_path = super(LegendreTrackFinderRun, self).create_path()

        good_cdc_hits_store_array_name = "GoodCDCHits"
        temp_track_cands_store_array_name = "TempTrackCands"

        background_hit_finder_module = basf2.register_module("BackgroundHitFinder")
        background_hit_finder_module.param("TMVACut", float(self.tmva_cut))
        if self.tmva_cut > 0:
            background_hit_finder_module.param("GoodCDCHitsStoreObjName", good_cdc_hits_store_array_name)

        cdctracking = basf2.register_module('CDCLegendreTracking')
        if self.tmva_cut > 0:
            cdctracking.param('UseOnlyCDCHitsRelatedFrom', good_cdc_hits_store_array_name)
        cdctracking.param('GFTrackCandsStoreArrayName', temp_track_cands_store_array_name)
        if self.stereo_assignment:
            cdctracking.param('WriteGFTrackCands', False)
        cdctracking.set_log_level(basf2.LogLevel.WARNING)

        cdc_stereo_combiner = basf2.register_module('CDCLegendreHistogramming')
        if self.tmva_cut > 0:
            cdc_stereo_combiner.param('UseOnlyCDCHitsRelatedFrom', good_cdc_hits_store_array_name)
        cdc_stereo_combiner.param({'GFTrackCandsStoreArrayName': temp_track_cands_store_array_name,
                                   "TracksStoreObjNameIsInput": True})
        cdc_stereo_combiner.set_log_level(basf2.LogLevel.WARNING)

        not_assigned_hits_searcher_module = basf2.register_module("NotAssignedHitsSearcher")
        not_assigned_hits_searcher_module.param({"TracksFromFinder": temp_track_cands_store_array_name,
                                                 "SplittedTracks": self.legendre_track_cands_store_array_name,
                                                 "NotAssignedCDCHits": self.not_assigned_cdc_hits_store_array_name,
                                                 })

        if self.splitting:
            not_assigned_hits_searcher_module.param("MinimumDistanceToSplit", 0.2)
            not_assigned_hits_searcher_module.param("MinimalHits", 17)
        else:
            not_assigned_hits_searcher_module.param("MinimumDistanceToSplit", 1.1)

        if self.tmva_cut > 0:
            main_path.add_module(background_hit_finder_module)

        main_path.add_module(cdctracking)
        if self.stereo_assignment:
            main_path.add_module(cdc_stereo_combiner)
        main_path.add_module(not_assigned_hits_searcher_module)

        return main_path


class CombinerTrackFinderRun(LegendreTrackFinderRun):

    local_track_cands_store_array_name = "LocalTrackCands"
    use_segment_quality_check = True
    do_combining = True

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CombinerTrackFinderRun, self).create_path()

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
            "UseOnlyCDCHitsRelatedFrom": self.not_assigned_cdc_hits_store_array_name,
            "CreateGFTrackCands": True,
            "FitSegments": True,
        })

        segment_quality_check = basf2.register_module("SegmentQualityCheck")
        segment_quality_check.param("RecoSegments", 'CDCRecoSegment2DVector')

        not_assigned_hits_combiner = basf2.register_module("NotAssignedHitsCombiner")
        not_assigned_hits_combiner.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                          "ResultTrackCands": "ResultTrackCands",
                                          "BadTrackCands": "BadTrackCands",
                                          "RecoSegments": 'CDCRecoSegment2DVector',
                                          "MinimalChi2": 0.8,
                                          "MinimalThetaDifference": 0.3,
                                          "MinimalZDifference": 10,
                                          "MinimalChi2Stereo": 0.000001})

        track_finder_output_combiner_naive = basf2.register_module("NaiveCombiner")
        track_finder_output_combiner_naive.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                                  "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                                  "ResultTrackCands": "NaiveResultTrackCands",
                                                  "UseMCInformation": False
                                                  })

        mc_track_matcher_module_local = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_local.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.legendre_track_cands_store_array_name,
        })

        mc_track_matcher_module_legendre = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_legendre.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.local_track_cands_store_array_name,
        })

        track_finder_output_combiner_mc = basf2.register_module("NaiveCombiner")
        track_finder_output_combiner_mc.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                               "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                               "ResultTrackCands": "BestResultTrackCands",
                                               "UseMCInformation": True
                                               })

        main_path.add_module(local_track_finder)
        if self.use_segment_quality_check:
            main_path.add_module(segment_quality_check)

        if self.do_combining:
            main_path.add_module(not_assigned_hits_combiner)
            main_path.add_module(track_finder_output_combiner_naive)
            main_path.add_module(mc_track_matcher_module_legendre)
            main_path.add_module(mc_track_matcher_module_local)
            main_path.add_module(track_finder_output_combiner_mc)

        return main_path


class TrasanTrackFinderRun(CombinerTrackFinderRun):
    trasan_track_cands_store_array_name = "TrasanTrackCands"

    def create_path(self):
        main_path = super(TrasanTrackFinderRun, self).create_path()

        cdctracking = basf2.register_module('Trasan')
        cdctracking.param({
            'GFTrackCandidatesColName': self.trasan_track_cands_store_array_name,
        })

        main_path.add_module(cdctracking)

        return main_path
