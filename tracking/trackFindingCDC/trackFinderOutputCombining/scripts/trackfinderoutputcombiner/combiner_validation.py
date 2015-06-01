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
import tracking.modules as modules
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
    use_hard_candidates = False
    mode = "Naive"

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.

        not_assigned_hits_combiner = modules.CDCNotAssignedHitsCombiner(
            output_track_cands_store_array_name="OldCombinerTrackCands",
            use_second_stage=self.use_hard_candidates)

        segment_track_combiner = modules.CDCSegmentTrackCombiner(output_track_cands_store_array_name="ResultTrackCands",
                                                                 segment_track_chooser_filter=self.segment_track_chooser_filter,
                                                                 segment_track_chooser_cut=self.segment_track_chooser_cut,
                                                                 segment_train_filter=self.segment_train_filter,
                                                                 segment_track_filter=self.segment_track_filter)

        main_path = super(CombinerValidationRun, self).create_path()

        add_mc_track_finder(main_path)
        main_path.add_module(modules.CDCFullFinder(output_track_cands_store_array_name=self.legendre_track_cands_store_array_name))

        if self.mode == "SegmentTrackCombiner":
            main_path.add_module(not_assigned_hits_combiner)
            main_path.add_module(segment_track_combiner)

            main_path.add_module(
                modules.CDCValidation(
                    track_candidates_store_array_name="ResultTrackCands",
                    output_file_name="evaluation/Combiner%.2f_%s_%s_%s_%s.root" %
                    (self.segment_track_chooser_cut, self.segment_track_chooser_filter,
                     self.segment_train_filter, self.segment_track_filter, str(self.use_hard_candidates))))

            if not os.path.exists("evaluation/OldCombiner_%s.root" % self.use_hard_candidates):
                main_path.add_module(modules.CDCValidation(
                    track_candidates_store_array_name="OldCombinerTrackCands",
                    output_file_name="evaluation/OldCombiner_%s.root" % self.use_hard_candidates))

        elif self.mode == "Naive":
            if not os.path.exists("evaluation/Naive.root"):
                main_path.add_module(self.get_basf2_module("NaiveCombiner", SkipHitsPreparation=True,
                                                           TracksStoreObjNameIsInput=True,
                                                           WriteGFTrackCands=True,
                                                           GFTrackCandsStoreArrayName="NaiveCombinerTrackCands"))
                main_path.add_module(modules.CDCValidation(track_candidates_store_array_name="NaiveCombinerTrackCands",
                                                           output_file_name="evaluation/Naive.root"))

            if not os.path.exists("evaluation/Legendre.root"):
                main_path.add_module(
                    modules.CDCValidation(
                        track_candidates_store_array_name=self.legendre_track_cands_store_array_name,
                        output_file_name="evaluation/Legendre.root"))

            if not os.path.exists("evaluation/Trasan.root"):
                main_path.add_module(self.get_basf2_module("Trasan", GFTrackCandidatesColName="TrasanTrackCands"))
                main_path.add_module(modules.CDCValidation(track_candidates_store_array_name="TrasanTrackCands",
                                                           output_file_name="evaluation/Trasan.root"))

        return main_path


def print_data():

    root_files = glob.glob("evaluation/*.root")

    result = initialize_results()
    result.update({"file_name": []})

    for file_name in root_files:
        result = extract_information_from_file(file_name, result)
        result["file_name"].append(file_name)

    print pd.DataFrame(result)


class ParameterScanner():

    def __init__(self, lambda_function, parameter_ranges):
        self.lambda_function = lambda_function
        self.parameter_ranges = parameter_ranges

    def scan_parameter_space(self):
        import itertools

        every_parameter_combination = itertools.product(*(self.parameter_ranges))
        p = Pool(8)
        p.map(extract_parameters, every_parameter_combination)
        p.close()


def extract_parameters(x):
    return main(*x)


def main(mode, tmva_cut, segment_chooser_filter, segment_train_filter, track_filter, use_hard_candidates):

    if segment_chooser_filter != "tmva" and tmva_cut > 0:
        return

    if mode != "SegmentTrackCombiner" and (
            segment_chooser_filter != "simple" or segment_train_filter != "simple" or
            track_filter != "simple" or tmva_cut != 0 or use_hard_candidates):
        return

    print mode, tmva_cut, segment_chooser_filter, segment_train_filter, track_filter, use_hard_candidates

    run = CombinerValidationRun()
    run.mode = mode
    run.segment_track_chooser_cut = tmva_cut
    run.segment_track_chooser_filter = segment_chooser_filter
    run.segment_train_filter = segment_train_filter
    run.segment_track_filter = track_filter
    run.use_hard_candidates = use_hard_candidates
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    from multiprocessing import Pool
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')

    scanner = ParameterScanner(main,
                               [[0.0, 0.02, 0.05, 0.07, 0.1, 0.2, 0.5],
                                ["tmva", "simple", "mc"],
                                   ["simple", "mc"],
                                   ["simple", "mc"],
                                   [True, False]
                                ])

    short_scanner = ParameterScanner(main,
                                     [["Naive", "SegmentTrackCombiner"], [0.0], ["simple", "mc"], ["simple"], ["simple"], [False]])
    short_scanner.scan_parameter_space()
    print_data()
