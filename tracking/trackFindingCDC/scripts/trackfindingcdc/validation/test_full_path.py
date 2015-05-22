#!/usr/bin/env python

from tracking.run.event_generation import StandardEventGenerationRun
from tracking import modules
from trackfinderoutputcombiner.validation import add_mc_track_finder

import logging
import sys

import basf2
import IPython
from ROOT import Belle2
from ROOT import genfit
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import numpy as np


class TestingModule(basf2.Module):

    def __init__(self, track_store_array_name="TrackCands"):
        super(TestingModule, self).__init__()
        self.number_fit_failed = 0
        self.number_fits = 0
        self.chi2_cutoff = 2
        self.track_store_array_name = track_store_array_name

    def event(self):
        tracks = Belle2.PyStoreArray(self.track_store_array_name)
        for track in tracks:
            rel = Belle2.DataStore.getRelationsWithObj(track, "TrackFitResults")
            if rel.size() > 0:
                print "Deleting fitted tracks"
                track.swap(genfit.TrackCand())

    def terminate(self):
        print "%d, failed %d" % (self.number_fit_failed, self.number_fits)


class HitCleaner(basf2.Module):

    def initialize(self):
        self.cdc_hit_look_up = Belle2.TrackFindingCDC.CDCMCHitLookUp()

    def event(self):
        tracks = Belle2.PyStoreArray("TrackCands")
        cdc_hits = Belle2.PyStoreArray("CDCHits")
        mc_particles = Belle2.PyStoreArray("MCParticles")

        cdc_hit_lookup = self.cdc_hit_look_up
        cdc_hit_lookup.fill()

        for track in tracks:
            # Store all Hit IDs and reset the track
            hitIDs = track.getHitIDs(Belle2.Const.CDC)
            hits = [cdc_hits[i] for i in hitIDs]
            good_hits = []

            relation_track_particle = [0] * mc_particles.getEntries()

            # Now only add those hits which do belong to the track (with MC Info)
            for i in hitIDs:
                current_mc_track = cdc_hit_lookup.getMCTrackId(cdc_hits[i])
                if 0 <= current_mc_track:
                    relation_track_particle[current_mc_track] += 1

            should_belong_to_track = np.argmax(relation_track_particle)

            deleted_hits = sum(relation_track_particle) - relation_track_particle[should_belong_to_track]

            plane_IDs_of_good_hits = []
            for i, hitID in enumerate(hitIDs):
                current_mc_track = cdc_hit_lookup.getMCTrackId(cdc_hits[hitID])
                if current_mc_track == should_belong_to_track:
                    good_hits.append(hitID)

            track.reset()
            sorting_parameter = 0
            for hitID in good_hits:
                track.addHit(Belle2.Const.CDC, hitID, cdc_hit_lookup.getRLInfo(cdc_hits[hitID]), sorting_parameter)
                sorting_parameter += 1

            print "having deleted", deleted_hits, "hits"


class FullRun(StandardEventGenerationRun):
    # Tester module for all cdc tracking in on path (plus combiners)

    n_events = 10
    show_not_fitted_tracks = False

    def create_path(self):
        # Creates the path with all the modules in a row
        main_path = super(FullRun, self).create_path()

        add_mc_track_finder(main_path, mc_track_cands_store_array_name="MCTrackCands")
        main_path.add_module(modules.CDCFullFinder(output_track_cands_store_array_name="TrackCands"))

        main_path.add_module(HitCleaner())

        # main_path.add_module(basf2.register_module("Trasan"))
        main_path.add_module(modules.CDCFitter(input_track_cands_store_array_name="TrackCands"))

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
                    output_file_name="output_with_cleaner.root"))

        return main_path

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    run = FullRun()
    run.configure_and_execute_from_commandline()
