#!/usr/bin/env python

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
                print "Deleting fitted tracks"
                track.swap(genfit.TrackCand())


class HitCleaner(basf2.Module):
    #: A small hit cleaner module to set the track information according to mc information.
    #: This is surely not for later usage but for testing the genfitter module

    def __init__(self):
        super(HitCleaner, self).__init__()

        self.number_of_tracks = 0
        self.number_of_deleted_hits = 0
        self.number_of_hits = 0

    def initialize(self):
        self.cdc_hit_look_up = Belle2.TrackFindingCDC.CDCMCHitLookUp()
        self.mc_matcher_lookup = Belle2.TrackMatchLookUp("MCTrackCands", "TrackCands")

    def event(self):
        tracks = Belle2.PyStoreArray("TrackCands")
        cdc_hits = Belle2.PyStoreArray("CDCHits")
        mc_particles = Belle2.PyStoreArray("MCParticles")

        cdc_hit_lookup = self.cdc_hit_look_up
        cdc_hit_lookup.fill()

        mc_matcher_lookup = self.mc_matcher_lookup

        self.number_of_tracks += tracks.getEntries()

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

            self.number_of_deleted_hits += deleted_hits
            self.number_of_hits += len(hitIDs)

            # Set the position and momentum
            mc_track = mc_matcher_lookup.getMatchedMCTrackCand(track)

            if mc_track:
                mc_trajectory = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(mc_track.getPosSeed()),
                                                                       Belle2.TrackFindingCDC.Vector3D(mc_track.getMomSeed()),
                                                                       mc_track.getChargeSeed())
                startingPosition = Belle2.TrackFindingCDC.Vector3D(track.getPosSeed().X(), track.getPosSeed().Y(), 0)
                sStartingPosition = mc_trajectory.calcPerpS(startingPosition)
                zStartingPosition = mc_trajectory.getTrajectorySZ().mapSToZ(sStartingPosition)
                mc_trajectory.setLocalOrigin(Belle2.TrackFindingCDC.Vector3D(startingPosition.xy(), zStartingPosition))

                pos = ROOT.TVector3(mc_trajectory.getSupport().x(), mc_trajectory.getSupport().y(), mc_trajectory.getSupport().z())
                mom = ROOT.TVector3(
                    mc_trajectory.getMom3DAtSupport().x(),
                    mc_trajectory.getMom3DAtSupport().y(),
                    mc_trajectory.getMom3DAtSupport().z())

                # track.setPosMomSeedAndPdgCode(pos, mom , int(mc_track.getChargeSeed() * 211))
                track.setPdgCode(int(track.getChargeSeed() * 211))

            else:
                # track.reset()
                track.setPdgCode(int(track.getChargeSeed() * 211))

    def terminate(self):
        print("Number of tracks in total: %d" % self.number_of_tracks)
        print("Number of hits in total: %d" % self.number_of_hits)
        print("Number of deleted hits: %d" % self.number_of_deleted_hits)

        print("Number of deleted hits per track: %f" % (1.0 * self.number_of_deleted_hits / self.number_of_tracks))
        print("Ratio of deleted hits: %f" % (100.0 * self.number_of_deleted_hits / self.number_of_hits))


class FullRun(StandardEventGenerationRun):
    #: Tester module for all cdc tracking in on path (plus combiners)

    n_events = 1
    show_not_fitted_tracks = False

    def create_path(self):
        #: Creates the path with all the modules in a row
        main_path = super(FullRun, self).create_path()

        add_mc_track_finder(main_path)
        main_path.add_module(modules.CDCFullFinder(output_track_cands_store_array_name="TrackCands"))

        main_path.add_module(StereoQuadTreePlotter())

        # main_path.add_module(modules.CDCNotAssignedHitsCombiner(output_track_cands_store_array_name="TrackCands"))

        # main_path.add_module(modules.CDCHitUniqueAssumer())

        # Enable these modules for further testing
        # main_path.add_module(modules.CDCMCFiller())
        # main_path.add_module(ReconstructionPositionHarvester(output_file_name="reconstruction_position.root"))
        # main_path.add_module(HitCleaner())
        # main_path.add_module(modules.CDCFitter(input_track_cands_store_array_name="TrackCands"))
        # main_path.add_module(modules.CDCEventDisplay(full_display=True))

        return main_path

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
