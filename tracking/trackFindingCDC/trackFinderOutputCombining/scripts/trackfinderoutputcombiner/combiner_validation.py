#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2
import ROOT
from ROOT import Belle2

from trackfinderoutputcombiner.validation import AddValidationMethod, LegendreTrackFinderRun
from tracking.validation.extract_information_from_tracking_validation_output import (initialize_results,
                                                                                     extract_information_from_file)
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import logging

CONTACT = "ucddn@student.kit.edu"


class CombinerValidationRun(LegendreTrackFinderRun, AddValidationMethod):

    local_track_cands_store_array_name = "LocalTrackCands"
    create_mc_tracks = True
    segment_track_chooser_cut = 0.1
    stereo_assignment = True

    def add_mc_combination(self, main_path):
        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({'UseCDCHits': True,
                                       'UseSVDHits': False,
                                       'UsePXDHits': False,
                                       'RelateClonesToMCParticles': True,
                                       'MCGFTrackCandsColName': "MCTrackCands",
                                       'PRGFTrackCandsColName': "TrackCands"})
        main_path.add_module(mc_track_matcher_module)
        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({'UseCDCHits': True,
                                       'UseSVDHits': False,
                                       'UsePXDHits': False,
                                       'RelateClonesToMCParticles': True,
                                       'MCGFTrackCandsColName': "MCTrackCands",
                                       'PRGFTrackCandsColName': self.local_track_cands_store_array_name})
        main_path.add_module(mc_track_matcher_module)
        naive_combiner_module = basf2.register_module("NaiveCombiner")
        naive_combiner_module.param({"TracksFromLegendreFinder": "TrackCands",
                                     "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                     "UseMCInformation": False,
                                     "ResultTrackCands": "NaiveCombinerTrackCands"})
        main_path.add_module(naive_combiner_module)

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.

        main_path = super(CombinerValidationRun, self).create_path()

        display_module = CDCSVGDisplayModule()
        display_module.draw_tracks = True
        display_module.draw_track_trajectories = True
        # main_path.add_module(display_module)

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
            "TracksStoreObjName": "TempCDCTracks",
            "WriteGFTrackCands": True,
            "CreateGFTrackCands": True,
            "FitSegments": True,
            'SkipHitsPreparation': True,
        })
        main_path.add_module(local_track_finder)

        combiner_module = basf2.register_module("SegmentTrackCombinerDev")
        combiner_module.param({'SegmentTrackChooser': 'tmva',
                               'SegmentTrackChooserParameters': {"cut": str(self.segment_track_chooser_cut)},
                               'SegmentTrainFilter': 'simple',
                               'SegmentTrackFilter': 'mc',
                               'WriteGFTrackCands': False,
                               'SkipHitsPreparation': True,
                               'TracksStoreObjNameIsInput': True})
        if self.segment_track_chooser_cut >= 1.0:
            combiner_module.param({"SegmentTrackChooser": "mc", "SegmentTrackChooserParameters": {}})
        if self.segment_track_chooser_cut <= 0.0:
            combiner_module.param({"SegmentTrackChooser": "simple", "SegmentTrackChooserParameters": {}})

        # combiner_module.set_log_level(basf2.LogLevel.DEBUG)
        combiner_module.set_debug_level(200)
        main_path.add_module(combiner_module)

        self.add_mc_combination(main_path)

        rest_track_finder = basf2.register_module("TrackFinderCDCSegmentPairAutomatonDev")
        rest_track_finder.param({'SkipHitsPreparation': True,
                                 "WriteGFTrackCands": True,
                                 "SegmentPairFilter": "mc",
                                 "SegmentPairNeighborChooser": "mc",
                                 "TracksStoreObjNameIsInput": True,
                                 "GFTrackCandsStoreArrayName": "ResultTrackCands"})
        main_path.add_module(rest_track_finder)

        self.create_validation(
            main_path,
            track_candidates_store_array_name="ResultTrackCands",
            output_file_name="evaluation/Combiner%.1f.root" %
            self.segment_track_chooser_cut)
        self.create_validation(
            main_path,
            track_candidates_store_array_name="TrackCands",
            output_file_name="evaluation/Legendre.root")
        self.create_validation(
            main_path,
            track_candidates_store_array_name="NaiveCombinerTrackCands",
            output_file_name="evaluation/Naive.root")

        return main_path


def main():

    result = initialize_results()
    result.update({"tmva_cut": []})

    for tmva_cut in np.arange(0.0, 1.1, 0.1):
        run = CombinerValidationRun()
        run.segment_track_chooser_cut = tmva_cut
        run.configure_and_execute_from_commandline()

        result = extract_information_from_file("evaluation/Combiner%.1f.root" % tmva_cut, result)
        result["tmva_cut"].append(tmva_cut)

    for key in result:
        print key + "\t" + "\t".join([str(value) for value in result[key]])

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
