#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from tracking.run.event_generation import StandardEventGenerationRun
from tracking import modules
from trackfinderoutputcombiner.validation import add_mc_track_finder

import logging
import sys

import basf2
import IPython
import ROOT
from ROOT import Belle2
from ROOT import genfit
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import numpy as np

from trackfindingcdc.validation.harvester import ReconstructionPositionHarvester
from trackfindingcdc.quadtree.quadTreePlotter import StereoQuadTreePlotter


class TestingModule(basf2.Module):
    #: A small track cleaner module to reset all tracls that were fitted by
    #: genfit (to see only not fitted tracks in the event display

    def __init__(self, track_store_array_name="TrackCands"):
        super(TestingModule, self).__init__()
        self.track_store_array_name = track_store_array_name

    def event(self):
        #: Reset all fitted tracks
        tracks = Belle2.PyStoreArray(self.track_store_array_name)
        for track in tracks:
            rel = Belle2.DataStore.getRelationsWithObj(track, "TrackFitResults")
            if rel.size() > 0:
                print("Deleting fitted tracks")
                track.swap(genfit.TrackCand())


class FullRun(StandardEventGenerationRun):
    #: Tester module for all cdc tracking in on path (plus combiners)

    n_events = 1
    show_not_fitted_tracks = False

    def create_path(self):
        #: Creates the path with all the modules in a row
        main_path = super(FullRun, self).create_path()

        add_mc_track_finder(main_path)
        main_path.add_module(modules.CDCFullFinder(output_track_cands_store_array_name="TrackCands"))

        # Enable these modules for further testing
        # main_path.add_module(StereoQuadTreePlotter())
        # main_path.add_module(modules.CDCNotAssignedHitsCombiner(output_track_cands_store_array_name="TrackCands"))
        # main_path.add_module(modules.CDCHitUniqueAssumer())
        # main_path.add_module(modules.CDCMCFiller())
        # main_path.add_module(ReconstructionPositionHarvester(output_file_name="reconstruction_position.root"))
        # main_path.add_module(HitCleaner())
        main_path.add_module(modules.CDCFitter(input_track_cands_store_array_name="TrackCands"))
        main_path.add_module(modules.CDCEventDisplay(full_display=True))

        if self.show_not_fitted_tracks:
            main_path.add_module(TestingModule(track_store_array_name="TrackCands"))

            display_module = CDCSVGDisplayModule()
            display_module.track_cands_store_array_name = "TrackCands"
            display_module.draw_gftrackcand_trajectories = True
            display_module.draw_gftrackcands = True
            main_path.add_module(display_module)
        else:
            main_path.add_module(
                modules.CDCValidation(
                    track_candidates_store_array_name="TrackCands",
                    output_file_name="output.root"))

        return main_path

if __name__ == "__main__":
    #: Main function
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    run = FullRun()
    run.configure_and_execute_from_commandline()
