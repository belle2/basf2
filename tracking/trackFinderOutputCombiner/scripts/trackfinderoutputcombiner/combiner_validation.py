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


class TestingModule(basf2.Module):

    def calculate_matching_tracks(self, segment, perpS_of_tracks):
        front = segment.front()
        back = segment.back()

        list_of_matches = list()

        for track in perpS_of_tracks:

            # Calculate distance
            trajectory = perpS_of_tracks[track][0]

            distance = trajectory.getDist2D(front.getRecoPos2D())
            if abs(distance) > 10:
                continue

            # Calculate perpS
            perpS_min = perpS_of_tracks[track][1]
            perpS_max = perpS_of_tracks[track][2]

            frontRecoPos3D = front.reconstruct3D(trajectory)
            backRecoPos3D = back.reconstruct3D(trajectory)

            forwardZ = front.getWire().getSkewLine().forwardZ()
            backwardZ = front.getWire().getSkewLine().backwardZ()

            if frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ:
                continue

            perpS_segment = [trajectory.calcPerpS(frontRecoPos3D.xy()), trajectory.calcPerpS(backRecoPos3D.xy())]

            if np.min(perpS_segment) > perpS_min and np.max(perpS_segment) < perpS_max:
                wireHits = perpS_of_tracks[track][3]
                perpS_list = perpS_of_tracks[track][4]
                superlayer_segment = segment.getISuperLayer()

                hits_in_same_superlayer = 0

                for wireHit, perpS in zip(wireHits, perpS_list):
                    if wireHit.getISuperLayer() == superlayer_segment and perpS > 0.5 * \
                            np.min(perpS_segment) and perpS < 1.5 * np.max(perpS_segment):
                        hits_in_same_superlayer += 1

                if hits_in_same_superlayer > 3:
                    continue
            else:
                continue

            list_of_matches.append([track, [np.min(perpS_segment), np.max(perpS_segment)]])

        return list_of_matches

    def calculate_perpS_list_of_tracks(self):

        tracks = Belle2.PyStoreArray("LegendreTrackCands")
        cdcHits = Belle2.PyStoreArray("CDCHits")

        perpS_of_tracks = dict()
        wireHitTopology = Belle2.TrackFindingCDC.CDCWireHitTopology.getInstance()
        for track in tracks:
            perpS_list = list()
            pos = track.getPosSeed()
            mom = track.getMomSeed()
            trajectory = Belle2.TrackFindingCDC.CDCTrajectory2D(Belle2.TrackFindingCDC.Vector2D(pos.X(), pos.Y()),
                                                                Belle2.TrackFindingCDC.Vector2D(mom.X(), mom.Y()),
                                                                track.getChargeSeed())
            wireHits = [wireHitTopology.getWireHit(cdcHits[cdcHitID]) for cdcHitID in track.getHitIDs(Belle2.Const.CDC)]
            perpS_list = [trajectory.calcPerpS(wireHit.getRefPos2D()) for wireHit in wireHits if wireHit.getStereoType() == 0]
            perpS_of_tracks.update({track: [trajectory, np.min(perpS_list), np.max(perpS_list), wireHits, perpS_list]})

        return perpS_of_tracks

    def event(self):
        perpS_of_tracks = self.calculate_perpS_list_of_tracks()

        items = Belle2.PyStoreObj("CDCRecoSegment2DVector")
        wrapped_vector = items.obj()
        vector = wrapped_vector.get()

        circleFitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getFitter()

        track_to_segment_dict = {item: [] for item in perpS_of_tracks}

        for segment in vector:
            if segment.getStereoType() != 0:

                matching_tracks = self.calculate_matching_tracks(segment, perpS_of_tracks)

                if len(matching_tracks) == 1:
                    track_to_segment_dict[matching_tracks[0][0]].append([segment, matching_tracks[0][1]])

        for track, segment_list in track_to_segment_dict.items():
            segment = None

            if len(segment_list) == 1:
                self.add_segment_to_track(segment_list[0][0], track)
            elif len(segment_list) > 1:
                # we have more than one possibility

                segments_by_superlayer = {i: [] for i in xrange(9)}

                for segment in segment_list:
                    segments_by_superlayer[segment[0].getISuperLayer()].append(segment)

                for id, segment_list in segments_by_superlayer.items():
                    if len(segment_list) == 1:
                        self.add_segment_to_track(segment_list[0][0], track)
                    elif len(segment_list) > 1:
                        # Put together segments that create a whole track part
                        perpS_segments = [segment[1] for segment in segment_list]

                        def is_non_overlapping_segment_list(l):
                            return reduce(operator.and_, [y[0] - x[1] > 0 for x, y in zip(l[:-1], l[1:])], True)

                        is_good = [
                            is_non_overlapping_segment_list(
                                np.sort(c)) for c in chain.from_iterable(
                                combinations(
                                    perpS_segments,
                                    r) for r in xrange(
                                    2,
                                    len(segment_list) + 1))]
                        all_combinations = chain.from_iterable(
                            combinations(
                                segment_list,
                                r) for r in xrange(
                                2,
                                len(segment_list) +
                                1))
                        possibly_track_parts = [comb for comb, flag in zip(all_combinations, is_good) if flag]

                        if len(possibly_track_parts) == 0:

                            trajectory = perpS_of_tracks[track][0]
                            perpS_list = perpS_of_tracks[track][4]
                            wireHits = perpS_of_tracks[track][3]

                            distances = []

                            for possible_match in segment_list:
                                perpS_segment_front, perpS_segment_back = possible_match[1]
                                distances.append(np.min(abs(np.subtract(perpS_segment_front, perpS_list))) +
                                                 np.min(abs(np.subtract(perpS_segment_back, perpS_list))))

                            segment = segment_list[np.argmin(distances)][0]

                            self.add_segment_to_track(segment, track)

                        elif len(possibly_track_parts) == 1:
                            for segment in possibly_track_parts[0]:
                                self.add_segment_to_track(segment[0], track)

                        else:
                            print possibly_track_parts

    def add_segment_to_track(self, segment, track):
        for recoHit in segment.items():
            track.addHit(Belle2.Const.CDC, recoHit.getWireHit().getStoreIHit())

        # segment.clear()


class CombinerValidationRun(LegendreTrackFinderRun, AddValidationMethod):

    local_track_cands_store_array_name = "LocalTrackCands"

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.

        self.stereo_assignment = True

        main_path = super(CombinerValidationRun, self).create_path()

        main_path.add_module(basf2.register_module("RootOutput"))

        if self.splitting:
            suffix = ""
        else:
            suffix = "_no_splitting"

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "UseOnlyCDCHitsRelatedFrom": self.not_assigned_cdc_hits_store_array_name,
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
        })

        main_path.add_module(local_track_finder)

        display_module = CDCSVGDisplayModule()
        display_module.draw_segments_id = True
        main_path.add_module(display_module)

        # main_path.add_module(TestingModule())

        combiner_module = basf2.register_module("SegmentTrackCombiner")
        combiner_module.param("GFTrackCandsStoreArrayName", "ResultTrackCands")
        combiner_module.set_debug_level(110)
        combiner_module.set_log_level(basf2.LogLevel.DEBUG)
        main_path.add_module(combiner_module)

        display_module = CDCSVGDisplayModule()
        display_module.track_cands_store_array_name = "ResultTrackCands"
        display_module.draw_gftrackcands = True
        display_module.draw_gftrackcand_trajectories = True
        main_path.add_module(display_module)

        return main_path


def main():
    run = CombinerValidationRun()
    run.configure_and_execute_from_commandline()


def plot(tmva_cut, splitting):
    if splitting:
        suffix = ""
    else:
        suffix = "_no_splitting"

    def catch_rates(prefix):
        fom = root_pandas.read_root(
            "evaluation/%s_%.2f%s.root" % (prefix, tmva_cut, suffix),
            tree_key="ExpertMCSideTrackingValidationModule_overview_figures_of_merit")
        rates = root_pandas.read_root(
            "evaluation/%s_%.2f%s.root" % (prefix, tmva_cut, suffix),
            tree_key="ExpertPRSideTrackingValidationModule_overview_figures_of_merit")

        return {"tmva_cut": tmva_cut,
                "splitting": splitting,
                "finding_efficiency": fom.finding_efficiency[0],
                "hit_efficiency": fom.hit_efficiency[0],
                "fake_rate": rates.fake_rate[0],
                "clone_rate": rates.clone_rate[0],
                "prefix": prefix}

    dataframe = pandas.DataFrame()

    dataframe = dataframe.append(catch_rates("trasan"), ignore_index=True)
    dataframe = dataframe.append(catch_rates("legendre"), ignore_index=True)
    dataframe = dataframe.append(catch_rates("result"), ignore_index=True)
    dataframe = dataframe.append(catch_rates("naive"), ignore_index=True)
    dataframe = dataframe.append(catch_rates("mc"), ignore_index=True)

    return dataframe

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
    results = pandas.DataFrame()
    for tmva_cut in np.arange(0, 0.6, 0.1):
        results = results.append(plot(tmva_cut=tmva_cut, splitting=False), ignore_index=True)
        results = results.append(plot(tmva_cut=tmva_cut, splitting=True), ignore_index=True)

    results.sort_index(by=["prefix", "splitting", "tmva_cut"], inplace=True)
    results.index = range(1, len(results) + 1)

    plt.clf()
    plt.plot(results.index, 100.0 * results.finding_efficiency, label="Finding Efficiency")
    plt.plot(results.index, 100.0 * results.hit_efficiency, label="Hit Efficiency")
    plt.plot(results.index, 100.0 * results.fake_rate, label="Fake Rate")
    plt.plot(results.index, 100.0 * results.clone_rate, label="Clone Rate")
    plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3, ncol=4, mode="expand", borderaxespad=0.)
    plt.ylabel("Rates in %")
    plt.xticks(results.index, ["splitting " + str(t) + " " + str(l) if s
                               else "no splitting " + str(t) + " " + str(l)
                               for (s, t, l) in zip(results.splitting, results.tmva_cut, results.prefix)], rotation="vertical")
    plt.subplots_adjust(bottom=0.5)
    plt.savefig("tracking_validation.pdf")

    plt.clf()
    plt.plot(results.index, 100.0 * results.finding_efficiency / results.iloc[0].finding_efficiency, label="Finding Efficiency")
    plt.plot(results.index, 100.0 * results.hit_efficiency / results.iloc[0].hit_efficiency, label="Hit Efficiency")
    plt.plot(results.index, 100.0 * results.fake_rate / results.iloc[0].fake_rate, label="Fake Rate")
    plt.plot(results.index, 100.0 * results.clone_rate / results.iloc[0].clone_rate, label="Clone Rate")
    plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3, ncol=4, mode="expand", borderaxespad=0.)
    plt.ylabel("Deviation in %")
    plt.xticks(results.index, ["splitting " + str(t) + " " + str(l) if s
                               else "no splitting " + str(t) + " " + str(l)
                               for (s, t, l) in zip(results.splitting, results.tmva_cut, results.prefix)], rotation="vertical")
    plt.subplots_adjust(bottom=0.5)
    plt.savefig("tracking_validation_rates.pdf")
