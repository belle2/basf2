#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2
import ROOT
from ROOT import Belle2

import glob

from trackfinderoutputcombiner.validation import AddValidationMethod, LegendreTrackFinderRun
from tracking.validation.extract_information_from_tracking_validation_output import (initialize_results,
                                                                                     extract_information_from_file)
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import logging

import pandas as pd


CONTACT = "ucddn@student.kit.edu"


class CombinerValidationRun(LegendreTrackFinderRun, AddValidationMethod):

    local_track_cands_store_array_name = "LocalTrackCands"
    segment_track_chooser_cut = 0.1
    segment_track_chooser_filter = "tmva"
    segment_train_filter = "simple"
    segment_track_filter = "simple"
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

        local_track_finder = self.get_basf2_module('SegmentFinderCDCFacetAutomaton',
                                                   GFTrackCandsStoreArrayName=self.local_track_cands_store_array_name,
                                                   TracksStoreObjName="TempCDCTracks",
                                                   WriteGFTrackCands=True,
                                                   CreateGFTrackCands=True,
                                                   FitSegments=True,
                                                   SkipHitsPreparation=True)
        main_path.add_module(local_track_finder)

        if not os.path.exists("evaluation/OldCombiner.root"):
            not_assigned_hits_combiner_module = basf2.register_module("NotAssignedHitsCombiner")
            not_assigned_hits_combiner_module.param({"TracksFromLegendreFinder": "TrackCands",
                                                     "ResultTrackCands": "OldCombinerTrackCands",
                                                     "RecoSegments": "CDCRecoSegment2DVector",
                                                     "BadTrackCands": "BadTrackCands"})
            main_path.add_module(not_assigned_hits_combiner_module)
            self.create_validation(
                main_path,
                track_candidates_store_array_name="OldCombinerTrackCands",
                output_file_name="evaluation/OldCombiner.root")

        combiner_module = basf2.register_module("SegmentTrackCombinerDev")
        combiner_module.param({'SegmentTrackChooser': self.segment_track_chooser_filter,
                               'SegmentTrainFilter': self.segment_train_filter,
                               'SegmentTrackFilter': self.segment_track_filter,
                               'WriteGFTrackCands': True,
                               'SkipHitsPreparation': True,
                               "GFTrackCandsStoreArrayName": "ResultTrackCands",
                               'TracksStoreObjNameIsInput': True})

        if self.segment_track_chooser_filter == "tmva":
            combiner_module.param('SegmentTrackChooserParameters', {"cut": str(self.segment_track_chooser_cut)})

        main_path.add_module(combiner_module)

        self.create_validation(
            main_path,
            track_candidates_store_array_name="ResultTrackCands",
            output_file_name="evaluation/Combiner%.1f_%s_%s_%s.root" % (self.segment_track_chooser_cut,
                                                                        self.segment_track_chooser_filter,
                                                                        self.segment_train_filter,
                                                                        self.segment_track_filter))

        if not os.path.exists("evaluation/Legendre.root"):
            self.create_validation(
                main_path,
                track_candidates_store_array_name="TrackCands",
                output_file_name="evaluation/Legendre.root")
        if not os.path.exists("evaluation/Naive.root"):
            self.add_mc_combination(main_path)
            self.create_validation(
                main_path,
                track_candidates_store_array_name="NaiveCombinerTrackCands",
                output_file_name="evaluation/Naive.root")
        if not os.path.exists("evaluation/Trasan.root"):
            trasan_track_finder = self.get_basf2_module("Trasan", GFTrackCandidatesColName="TrasanTrackCands")
            main_path.add_module(trasan_track_finder)
            self.create_validation(
                main_path,
                track_candidates_store_array_name="TrasanTrackCands",
                output_file_name="evaluation/Trasan.root")

        return main_path


def print_data():

    root_files = glob.glob("evaluation/*.root")

    result = initialize_results()
    result.update({"file_name": []})

    for file_name in root_files:
        result = extract_information_from_file(file_name, result)
        result["file_name"].append(file_name)

    print result


def main(pool_number):

    if pool_number == 1:
        for tmva_cut in np.arange(0.0, 1.0, 0.1):
            run = CombinerValidationRun()
            run.segment_track_chooser_cut = tmva_cut
            run.segment_track_chooser_filter = "tmva"
            run.segment_train_filter = "simple"
            run.segment_track_filter = "simple"
            run.configure_and_execute_from_commandline()

    elif pool_number == 2:
        for tmva_cut in np.arange(0.0, 1.0, 0.1):
            run = CombinerValidationRun()
            run.segment_track_chooser_cut = tmva_cut
            run.segment_track_chooser_filter = "tmva"
            run.segment_train_filter = "mc"
            run.segment_track_filter = "simple"
            run.configure_and_execute_from_commandline()

    elif pool_number == 3:
        for tmva_cut in np.arange(0.0, 1.0, 0.1):
            run = CombinerValidationRun()
            run.segment_track_chooser_cut = tmva_cut
            run.segment_track_chooser_filter = "tmva"
            run.segment_train_filter = "simple"
            run.segment_track_filter = "mc"
            run.configure_and_execute_from_commandline()

    elif pool_number == 4:
        for tmva_cut in np.arange(0.0, 1.0, 0.1):
            run = CombinerValidationRun()
            run.segment_track_chooser_cut = tmva_cut
            run.segment_track_chooser_filter = "tmva"
            run.segment_train_filter = "mc"
            run.segment_track_filter = "mc"
            run.configure_and_execute_from_commandline()

    elif pool_number == 5:
        run = CombinerValidationRun()
        run.segment_track_chooser_filter = "mc"
        run.segment_train_filter = "mc"
        run.segment_track_filter = "mc"
        run.configure_and_execute_from_commandline()

    elif pool_number == 6:
        run = CombinerValidationRun()
        run.segment_track_chooser_filter = "simple"
        run.segment_train_filter = "simple"
        run.segment_track_filter = "simple"
        run.configure_and_execute_from_commandline()

    elif pool_number == 7:
        run = CombinerValidationRun()
        run.segment_track_chooser_filter = "simple"
        run.segment_train_filter = "mc"
        run.segment_track_filter = "mc"
        run.configure_and_execute_from_commandline()

    elif pool_number == 8:
        run = CombinerValidationRun()
        run.segment_track_chooser_filter = "mc"
        run.segment_train_filter = "simple"
        run.segment_track_filter = "mc"
        run.configure_and_execute_from_commandline()

    elif pool_number == 9:
        run = CombinerValidationRun()
        run.segment_track_chooser_filter = "mc"
        run.segment_train_filter = "mc"
        run.segment_track_filter = "simple"
        run.configure_and_execute_from_commandline()

    elif pool_number == 10:
        run = CombinerValidationRun()
        run.segment_track_chooser_filter = "mc"
        run.segment_train_filter = "simple"
        run.segment_track_filter = "simple"
        run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    from multiprocessing import Pool
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    p = Pool(8)
    p.map(main, [1, 2, 3, 4])
    print_data()
