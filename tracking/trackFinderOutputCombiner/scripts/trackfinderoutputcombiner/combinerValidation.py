#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.validation.module import SeparatedTrackingValidationModule

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfindingcdc.cdcLegendreTrackingValidation import ReassignHits

import logging

try:
    import root_pandas
except ImportError:
    print "do a pip install git+https://github.com/ibab/root_pandas"

import pandas
import matplotlib.pyplot as plt
import seaborn as sb


def get_logger():
    return logging.getLogger(__name__)

CONTACT = "ucddn@student.kit.edu"


class AddValidationMethod:

    def create_matcher_module(self, track_candidates_store_array_name):
        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': track_candidates_store_array_name,
        })

        return mc_track_matcher_module

    def create_validation(self, main_path, track_candidates_store_array_name, output_file_name):
        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name=output_file_name,
            trackCandidatesColumnName=track_candidates_store_array_name,
            expert_level=2)

        main_path.add_module(self.create_matcher_module(track_candidates_store_array_name))
        main_path.add_module(validation_module)


class MCTrackFinderRun(StandardEventGenerationRun):
    # output track cands
    mc_track_cands_store_array_name = "MCTrackCands"

    def create_path(self):
        main_path = super(MCTrackFinderRun, self).create_path()

        track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruth')
        track_finder_mc_truth_module.param({
            'UseCDCHits': True,
            'WhichParticles': [],
            'GFTrackCandidatesColName': self.mc_track_cands_store_array_name,
        })

        main_path.add_module(track_finder_mc_truth_module)

        return main_path


class LegendreTrackFinderRun(MCTrackFinderRun):
    # output (splitted) track cands
    legendre_track_cands_store_array_name = "LegendreTrackCands"
    # output not assigned cdc hits
    not_assigned_cdc_hits_store_array_name = "NotAssignedCDCHits"
    # input tmva cut
    tmva_cut = 0.1
    # input flag if to split
    splitting = True

    def create_argument_parser(self, **kwds):
        argument_parser = super(LegendreTrackFinderRun, self).create_argument_parser(**kwds)
        argument_parser.add_argument(
            '-t',
            '--tmva-cut',
            dest='tmva_cut',
            default=self.tmva_cut,
            type=float,
            help='Cut for the TMVA in the module.'
        )

        argument_parser.add_argument(
            '--splitting',
            dest='splitting',
            action="store_true",
            help='Split the tracks before searching for not assigned hits.'
        )

        argument_parser.add_argument(
            '--no-splitting',
            dest='splitting',
            action="store_false",
            help='Split the tracks before searching for not assigned hits.'
        )

        argument_parser.set_defaults(splitting=self.splitting)

        return argument_parser

    def create_path(self):
        main_path = super(LegendreTrackFinderRun, self).create_path()

        good_cdc_hits_store_array_name = "GoodCDCHits"
        temp_track_cands_store_array_name = "TempTrackCands"

        background_hit_finder_module = basf2.register_module("BackgroundHitFinder")
        background_hit_finder_module.param("TMVACut", float(self.tmva_cut))
        if self.tmva_cut > 0:
            background_hit_finder_module.param("GoodCDCHitsStoreObjName", good_cdc_hits_store_array_name)

        cdctracking = basf2.register_module('CDCLegendreTracking')
        if self.tmva_cut > 0:
            cdctracking.param('CDCHitsColName', good_cdc_hits_store_array_name)
        cdctracking.param('GFTrackCandidatesColName', temp_track_cands_store_array_name)
        cdctracking.set_log_level(basf2.LogLevel.WARNING)

        cdc_stereo_combiner = basf2.register_module('CDCLegendreHistogramming')
        if self.tmva_cut > 0:
            cdc_stereo_combiner.param('CDCHitsColName', good_cdc_hits_store_array_name)
        cdc_stereo_combiner.param('GFTrackCandidatesColName', temp_track_cands_store_array_name)
        cdc_stereo_combiner.set_log_level(basf2.LogLevel.WARNING)

        not_assigned_hits_searcher_module = basf2.register_module("NotAssignedHitsSearcher")
        not_assigned_hits_searcher_module.param({"TracksFromFinder": temp_track_cands_store_array_name,
                                                 "SplittedTracks": self.legendre_track_cands_store_array_name,
                                                 "NotAssignedCDCHits": self.not_assigned_cdc_hits_store_array_name,
                                                 })

        if self.tmva_cut > 0:
            not_assigned_hits_searcher_module.param("CDCHits", good_cdc_hits_store_array_name)

        if self.splitting:
            not_assigned_hits_searcher_module.param("MinimumDistanceToSplit", 0.2)
            not_assigned_hits_searcher_module.param("MinimalHits", 17)
        else:
            not_assigned_hits_searcher_module.param("MinimumDistanceToSplit", 1.1)

        if self.tmva_cut > 0:
            main_path.add_module(background_hit_finder_module)

        main_path.add_module(cdctracking)
        main_path.add_module(cdc_stereo_combiner)
        main_path.add_module(not_assigned_hits_searcher_module)

        if self.tmva_cut > 0:
            main_path.add_module(ReassignHits(
                old_cdc_hits_store_array_name=good_cdc_hits_store_array_name,
                new_cdc_hits_store_array_name="CDCHits",
                track_cands_store_array_name=self.legendre_track_cands_store_array_name))

        return main_path


class CombinerTrackFinderRun(LegendreTrackFinderRun):

    local_track_cands_store_array_name = "LocalTrackCands"
    use_segment_quality_check = True
    do_combining = True

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CombinerTrackFinderRun, self).create_path()

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
            "UseOnlyCDCHitsRelatedFrom": self.not_assigned_cdc_hits_store_array_name,
            "CreateGFTrackCands": True,
            "FitSegments": True,
        })

        segment_quality_check = basf2.register_module("SegmentQualityCheck")
        segment_quality_check.param("RecoSegments", 'CDCRecoSegment2DVector')

        not_assigned_hits_combiner = basf2.register_module("NotAssignedHitsCombiner")
        not_assigned_hits_combiner.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                          "ResultTrackCands": "ResultTrackCands",
                                          "BadTrackCands": "BadTrackCands",
                                          "RecoSegments": 'CDCRecoSegment2DVector',
                                          "MinimalChi2": 0.8,
                                          "MinimalThetaDifference": 0.3,
                                          "MinimalZDifference": 10,
                                          "MinimalChi2Stereo": 0.000001})

        track_finder_output_combiner_naive = basf2.register_module("NaiveCombiner")
        track_finder_output_combiner_naive.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                                  "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                                  "ResultTrackCands": "NaiveResultTrackCands",
                                                  "UseMCInformation": False
                                                  })

        mc_track_matcher_module_local = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_local.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.legendre_track_cands_store_array_name,
        })

        mc_track_matcher_module_legendre = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_legendre.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.local_track_cands_store_array_name,
        })

        track_finder_output_combiner_mc = basf2.register_module("NaiveCombiner")
        track_finder_output_combiner_mc.param({"TracksFromLegendreFinder": self.legendre_track_cands_store_array_name,
                                               "NotAssignedTracksFromLocalFinder": self.local_track_cands_store_array_name,
                                               "ResultTrackCands": "BestResultTrackCands",
                                               "UseMCInformation": True
                                               })

        main_path.add_module(local_track_finder)
        if self.use_segment_quality_check:
            main_path.add_module(segment_quality_check)

        if self.do_combining:
            main_path.add_module(not_assigned_hits_combiner)
            main_path.add_module(track_finder_output_combiner_naive)
            main_path.add_module(mc_track_matcher_module_legendre)
            main_path.add_module(mc_track_matcher_module_local)
            main_path.add_module(track_finder_output_combiner_mc)

        return main_path


class TrasanTrackFinderRun(CombinerTrackFinderRun):
    trasan_track_cands_store_array_name = "TrasanTrackCands"

    def create_path(self):
        main_path = super(TrasanTrackFinderRun, self).create_path()

        cdctracking = basf2.register_module('Trasan')
        cdctracking.param({
            'GFTrackCandidatesColName': self.trasan_track_cands_store_array_name,
        })

        main_path.add_module(cdctracking)

        return main_path


class CombinerValidationRun(TrasanTrackFinderRun, AddValidationMethod):

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CombinerValidationRun, self).create_path()

        if self.splitting:
            suffix = ""
        else:
            suffix = "_no_splitting"

        self.create_validation(
            main_path,
            track_candidates_store_array_name="ResultTrackCands",
            output_file_name="evaluation/result_%.2f%s.root" % (self.tmva_cut, suffix))
        self.create_validation(
            main_path,
            track_candidates_store_array_name="NaiveResultTrackCands",
            output_file_name="evaluation/naive_%.2f%s.root" % (self.tmva_cut, suffix))
        self.create_validation(
            main_path,
            track_candidates_store_array_name=self.legendre_track_cands_store_array_name,
            output_file_name="evaluation/legendre_%.2f%s.root" % (self.tmva_cut, suffix))

        self.create_validation(
            main_path,
            track_candidates_store_array_name=self.trasan_track_cands_store_array_name,
            output_file_name="evaluation/trasan_%.2f%s.root" % (self.tmva_cut, suffix))

        self.create_validation(
            main_path,
            track_candidates_store_array_name="BestResultTrackCands",
            output_file_name="evaluation/mc_%.2f%s.root" % (self.tmva_cut, suffix))

        return main_path


def main():
    run = CombinerValidationRun()
    run.configure_and_execute_from_commandline()
    print basf2.statistics


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
    # main()
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
