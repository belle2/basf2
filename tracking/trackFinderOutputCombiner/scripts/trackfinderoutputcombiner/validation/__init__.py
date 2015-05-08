from ROOT import gSystem
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
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
    # input tmva cut
    tmva_cut = 0.1
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

        return argument_parser

    def create_path(self):
        # BackgroundHitFinder, Legendre, LegendreStereo, NotAssignedHitsSearcher
        main_path = super(LegendreTrackFinderRun, self).create_path()

        background_hit_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
        background_hit_finder_module.param({
            "ClusterFilter": "tmva",
            "ClusterFilterParameters": {"cut": str(self.tmva_cut)},
            "SegmentsStoreObjName": "TempCDCRecoSegment2DVector",
            "FacetFilter": "none",
            "FacetNeighborChooser": "none",
        })

        cdctracking = basf2.register_module('CDCLegendreTracking')
        cdctracking.param({'WriteGFTrackCands': False,
                           'SkipHitsPreparation': True})

        cdc_stereo_combiner = basf2.register_module('CDCLegendreHistogramming')
        cdc_stereo_combiner.param({'WriteGFTrackCands': True,
                                   'SkipHitsPreparation': True,
                                   'TracksStoreObjNameIsInput': True})

        if self.tmva_cut > 0:
            main_path.add_module(background_hit_finder_module)

        main_path.add_module(cdctracking)
        if self.stereo_assignment:
            main_path.add_module(cdc_stereo_combiner)

        return main_path


class TrasanTrackFinderRun(LegendreTrackFinderRun):
    trasan_track_cands_store_array_name = "TrasanTrackCands"

    def create_path(self):
        main_path = super(TrasanTrackFinderRun, self).create_path()

        cdctracking = basf2.register_module('Trasan')
        cdctracking.param({
            'GFTrackCandidatesColName': self.trasan_track_cands_store_array_name,
        })

        main_path.add_module(cdctracking)

        return main_path
