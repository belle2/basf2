#!/usr/bin/env python

from tracking.run.event_generation import StandardEventGenerationRun
from tracking import modules
from trackfinderoutputcombiner.validation import add_mc_track_finder

import logging
import sys

import basf2


class FullRun(StandardEventGenerationRun):
    # Tester module for all cdc tracking in on path (plus combiners)

    n_events = 1

    def create_path(self):
        # Creates the path with all the modules in a row
        main_path = super(FullRun, self).create_path()

        main_path.add_module(modules.CDCFullFinder(output_track_cands_store_array_name="TrackCands"))

        main_path.add_module(modules.CDCFitter(input_track_cands_store_array_name="TrackCands"))

        genfitter_visualization = StandardEventGenerationRun.get_basf2_module('Display', showTrackCandidates=True, showMCInfo=False)
        main_path.add_module(genfitter_visualization)

        return main_path

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    run = FullRun()
    run.configure_and_execute_from_commandline()
