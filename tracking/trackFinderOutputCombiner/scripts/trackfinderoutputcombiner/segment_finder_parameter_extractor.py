import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.combinerValidation import MCTrackFinderRun
from trackfinderoutputcombiner.combination_checker import CombinationChecker
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners
import logging
import sys

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import root_pandas
import pandas
import matplotlib.pyplot as plt
import seaborn as sb
import numpy as np


class SegmentFinderParameterExtractorModule(HarvestingModule):

    def __init__(self, local_track_cands_store_array_name, mc_track_cands_store_array_name, output_file_name):
        super(
            SegmentFinderParameterExtractorModule,
            self).__init__(
            foreach=local_track_cands_store_array_name,
            output_file_name=output_file_name)

        self.mc_track_cands_store_array_name = mc_track_cands_store_array_name

        self.mc_track_matcher = Belle2.TrackMatchLookUp(self.mc_track_cands_store_array_name,
                                                        self.foreach)

    def peel(self, local_track_cand):
        mc_track_matcher = self.mc_track_matcher

        is_matched = mc_track_matcher.isMatchedPRTrackCand(local_track_cand)
        is_background = mc_track_matcher.isBackgroundPRTrackCand(local_track_cand)
        is_ghost = mc_track_matcher.isGhostPRTrackCand(local_track_cand)

        related_mc_track_cand = mc_track_matcher.getRelatedMCTrackCand(local_track_cand)

        track_momentum = np.NaN
        track_position = np.NaN
        track_pt = np.NaN
        track_phi = np.NaN

        segment_momentum = np.NaN
        segment_position = np.NaN
        segment_pt = np.NaN
        segment_phi = np.NaN

        if is_matched:
            track_momentum = related_mc_track_cand.getPosSeed()
            track_position = related_mc_track_cand.getPosSeed()
            track_pt = track_momentum.Pt()
            track_phi = track_momentum.Phi()
            trajectory_track = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(track_position),
                                                                      Belle2.TrackFindingCDC.Vector3D(track_momentum),
                                                                      related_mc_track_cand.getChargeSeed())

            segment_momentum = local_track_cand.getMomSeed()
            segment_position = local_track_cand.getPosSeed()
            segment_pt = segment_momentum.Pt()
            segment_phi = segment_momentum.Phi()
            trajectory_segment = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(segment_position),
                                                                        Belle2.TrackFindingCDC.Vector3D(segment_momentum),
                                                                        local_track_cand.getChargeSeed())

        return dict(is_matched=is_matched,
                    is_background=is_background,
                    is_ghost=is_ghost,
                    segment_pt=segment_pt,
                    track_pt=track_pt,
                    difference_pt=segment_pt - track_pt,
                    segment_phi=segment_phi,
                    track_phi=track_phi,
                    difference_phi=segment_phi - track_phi)

    save_tree = refiners.save_tree(folder_name="tree")


class SegmentTrackFinderRun(MCTrackFinderRun):
    # output (splitted) track cands
    local_track_cands_store_array_name = "LocalTrackCands"
    # input tmva cut
    tmva_cut = 0.1

    def create_argument_parser(self, **kwds):
        argument_parser = super(SegmentTrackFinderRun, self).create_argument_parser(**kwds)
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
        main_path = super(SegmentTrackFinderRun, self).create_path()

        good_cdc_hits_store_array_name = "GoodCDCHits"

        background_hit_finder_module = basf2.register_module("BackgroundHitFinder")
        background_hit_finder_module.param({"TMVACut": float(self.tmva_cut),
                                            "ClustersStoreObjName": "GoodCDCWireHitClusters"})

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
            "SegmentsStoreObjName": "CDCRecoSegment2DVector",
            "FitSegments": True,
            "CreateGFTrackCands": True
        })

        segment_quality_check_module = basf2.register_module("SegmentQualityCheck")
        segment_quality_check_module.param("RecoSegments", "CDCRecoSegment2DVector")

        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': self.mc_track_cands_store_array_name,
            'PRGFTrackCandsColName': self.local_track_cands_store_array_name
        })

        if self.tmva_cut > 0:
            background_hit_finder_module.param("GoodCDCHitsStoreObjName", good_cdc_hits_store_array_name)
            local_track_finder.param("UseOnlyCDCHitsRelatedFrom", good_cdc_hits_store_array_name)
            main_path.add_module(background_hit_finder_module)

        main_path.add_module(local_track_finder)
        main_path.add_module(segment_quality_check_module)
        main_path.add_module(mc_track_matcher_module)

        return main_path


class SegmentFinderParameterExtractorRun(SegmentTrackFinderRun):
    display_module = CDCSVGDisplayModule(output_folder="tmp", interactive=True)
    display_module.draw_hits = False
    display_module.draw_gftrackcand_trajectories = True
    display_module.draw_gftrackcands = True
    display_module.draw_clusters = False
    display_module.draw_segments_id = False
    display_module.cdc_wire_hit_cluster_store_obj_name = "GoodCDCWireHitClusters"
    display_module.track_cands_store_array_name = "LocalTrackCands"

    def create_path(self):
        main_path = super(SegmentFinderParameterExtractorRun, self).create_path()

        main_path.add_module(
            SegmentFinderParameterExtractorModule(
                local_track_cands_store_array_name=self.local_track_cands_store_array_name,
                mc_track_cands_store_array_name=self.mc_track_cands_store_array_name,
                output_file_name="segment_finder_parameter_extractor.root"))

        return main_path


def plot():
    data = root_pandas.read_root(
        "segment_finder_parameter_extractor.root",
        tree_key="tree/SegmentFinderParameterExtractorModule_tree")
    data.hist()
    plt.savefig("segment_finder_parameter_extractor.pdf")


def main():
    run = SegmentFinderParameterExtractorRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
    plot()
