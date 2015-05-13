#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2
import ROOT
from ROOT import Belle2

from trackfinderoutputcombiner.validation import TrasanTrackFinderRun, AddValidationMethod, LegendreTrackFinderRun

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import logging
from functools import reduce

try:
    import root_pandas
except ImportError:
    print "do a pip install git+https://github.com/ibab/root_pandas"

import pandas
import matplotlib.pyplot as plt
import seaborn as sb

from itertools import chain, combinations
import operator


def get_logger():
    return logging.getLogger(__name__)

CONTACT = "ucddn@student.kit.edu"


class CombinerValidationRun(LegendreTrackFinderRun, AddValidationMethod):

    local_track_cands_store_array_name = "LocalTrackCands"

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.

        self.stereo_assignment = True

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

        display_module = CDCSVGDisplayModule()
        display_module.draw_segments_id = True
        # main_path.add_module(display_module)

        combiner_module = basf2.register_module("SegmentTrackCombinerDev")
        combiner_module.param({'SegmentTrackChooser': 'tmva',
                               'SegmentTrackChooserParameters': {"cut": "0.3"},
                               'SegmentTrainFilter': 'simple',
                               'SegmentTrackFilter': 'simple',
                               'WriteGFTrackCands': False,
                               'SkipHitsPreparation': True,
                               'TracksStoreObjNameIsInput': True})
        # combiner_module.set_log_level(basf2.LogLevel.DEBUG)
        combiner_module.set_debug_level(200)
        main_path.add_module(combiner_module)

        naive_combiner_module = basf2.register_module("NaiveCombiner")
        naive_combiner_module.param({"TracksFromLegendreFinder": "TrackCands",
                                     "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                     "ResultTrackCands": "NaiveCombinerTrackCands"})
        main_path.add_module(naive_combiner_module)

        rest_track_finder = basf2.register_module("TrackFinderCDCSegmentPairAutomatonDev")
        rest_track_finder.param({'SkipHitsPreparation': True,
                                 "WriteGFTrackCands": True,
                                 "SegmentPairFilter": "mc",
                                 "SegmentPairNeighborChooser": "mc",
                                 "TracksStoreObjNameIsInput": True,
                                 "GFTrackCandsStoreArrayName": "ResultTrackCands"})
        main_path.add_module(rest_track_finder)

        display_module = CDCSVGDisplayModule()
        display_module.draw_track_trajectories = True
        display_module.draw_tracks = True
        # main_path.add_module(display_module)

        display_module = CDCSVGDisplayModule()
        display_module.draw_gftrackcand_trajectories = True
        display_module.draw_gftrackcands = True
        display_module.track_cands_store_array_name = "ResultTrackCands"
        # main_path.add_module(display_module)

        self.create_validation(main_path, track_candidates_store_array_name="ResultTrackCands", output_file_name="Combiner.root")
        self.create_validation(main_path, track_candidates_store_array_name="TrackCands", output_file_name="Legendre.root")
        self.create_validation(
            main_path,
            track_candidates_store_array_name="NaiveCombinerTrackCands",
            output_file_name="Naive.root")

        return main_path


def main():
    run = CombinerValidationRun()
    run.n_events = 1
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
