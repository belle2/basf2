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

from trackfinderoutputcombiner.validation import add_local_track_finder, add_validation, add_new_combiner,\
    add_legendre_track_finder, add_old_combiner, add_mc_track_finder
from tracking.validation.extract_information_from_tracking_validation_output import (initialize_results,
                                                                                     extract_information_from_file)
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from tracking.run.event_generation import StandardEventGenerationRun

import logging

import pandas as pd


CONTACT = "ucddn@student.kit.edu"


class CombinerValidationRun(StandardEventGenerationRun):

    local_track_cands_store_array_name = "LocalTrackCands"
    legendre_track_cands_store_array_name = "LegendreTrackCands"

    segment_track_chooser_cut = 0.1
    segment_track_chooser_filter = "tmva"
    segment_train_filter = "simple"
    segment_track_filter = "simple"

    def add_mc_combination(self, main_path):
        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({'UseCDCHits': True,
                                       'UseSVDHits': False,
                                       'UsePXDHits': False,
                                       'RelateClonesToMCParticles': True,
                                       'MCGFTrackCandsColName': "MCTrackCands",
                                       'PRGFTrackCandsColName': self.legendre_track_cands_store_array_name})
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
        naive_combiner_module.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                     "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                     "UseMCInformation": False,
                                     "ResultTrackCands": "NaiveCombinerTrackCands"})
        main_path.add_module(naive_combiner_module)

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.

        main_path = super(CombinerValidationRun, self).create_path()

        add_mc_track_finder(main_path)
        add_legendre_track_finder(main_path, output_track_cands_store_array_name=self.legendre_track_cands_store_array_name)

        drawing_module = CDCSVGDisplayModule()
        drawing_module.draw_tracks = True
        drawing_module.draw_track_trajectories = True
        # main_path.add_module(drawing_module)

        add_local_track_finder(main_path, output_track_cands_store_array_name="LocalTrackCands")

        drawing_module = CDCSVGDisplayModule()
        drawing_module.draw_segments_id = True
        # main_path.add_module(drawing_module)

        add_old_combiner(main_path, output_track_cands_store_array_name="OldCombinerTrackCands")

        drawing_module = CDCSVGDisplayModule()
        drawing_module.draw_tracks = True
        drawing_module.draw_track_trajectories = True
        # main_path.add_module(drawing_module)

        add_new_combiner(main_path, output_track_cands_store_array_name="ResultTrackCands",
                         segment_track_chooser_filter=self.segment_track_chooser_filter,
                         segment_track_chooser_cut=self.segment_track_chooser_cut,
                         segment_train_filter=self.segment_train_filter,
                         segment_track_filter=self.segment_track_filter)

        drawing_module = CDCSVGDisplayModule()
        drawing_module.draw_tracks = True
        drawing_module.draw_track_trajectories = True
        # main_path.add_module(drawing_module)

        return main_path

        add_validation(
            main_path,
            track_candidates_store_array_name="ResultTrackCands",
            output_file_name="evaluation/Combiner%.1f_%s_%s_%s.root" % (self.segment_track_chooser_cut,
                                                                        self.segment_track_chooser_filter,
                                                                        self.segment_train_filter,
                                                                        self.segment_track_filter))

        if not os.path.exists("evaluation/OldCombiner.root"):
            add_validation(
                main_path,
                track_candidates_store_array_name="OldCombinerTrackCands",
                output_file_name="evaluation/OldCombiner.root")

        if not os.path.exists("evaluation/Legendre.root"):
            add_validation(
                main_path,
                track_candidates_store_array_name=self.legendre_track_cands_store_array_name,
                output_file_name="evaluation/Legendre.root")

        if not os.path.exists("evaluation/Naive.root"):
            self.add_mc_combination(main_path)
            add_validation(
                main_path,
                track_candidates_store_array_name="NaiveCombinerTrackCands",
                output_file_name="evaluation/Naive.root")

        if not os.path.exists("evaluation/Trasan.root"):
            trasan_track_finder = self.get_basf2_module("Trasan", GFTrackCandidatesColName="TrasanTrackCands")
            main_path.add_module(trasan_track_finder)
            add_validation(
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

    print pd.DataFrame(result)


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
    # p = Pool(8)
    # p.map(main, [5])
    main(5)
    print_data()
