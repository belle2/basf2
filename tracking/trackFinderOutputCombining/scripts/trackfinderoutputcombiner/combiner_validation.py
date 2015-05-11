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

        self.stereo_assignment = False

        main_path = super(CombinerValidationRun, self).create_path()

        display_module = CDCSVGDisplayModule()
        display_module.draw_tracks = True
        display_module.draw_track_trajectories = True
        # main_path.add_module(display_module)

        local_track_finder = basf2.register_module('SegmentFinderCDCFacetAutomaton')
        local_track_finder.param({
            "GFTrackCandsStoreArrayName": self.local_track_cands_store_array_name,
            "TracksStoreObjName": "TempCDCTracks",
            "WriteClusters": True,
            "WriteGFTrackCands": False,
            'SkipHitsPreparation': True,
        })
        main_path.add_module(local_track_finder)

        display_module = CDCSVGDisplayModule()
        display_module.draw_segments_id = True
        main_path.add_module(display_module)

        combiner_module = basf2.register_module("SegmentTrackCombiner")
        combiner_module.param({'WriteGFTrackCands': False,
                               'SkipHitsPreparation': True,
                               'TracksStoreObjNameIsInput': True})
        combiner_module.set_debug_level(200)
        # combiner_module.set_log_level(basf2.LogLevel.DEBUG)
        main_path.add_module(combiner_module)

        display_module = CDCSVGDisplayModule()
        display_module.draw_track_trajectories = True
        display_module.draw_tracks = True
        main_path.add_module(display_module)

        display_module = CDCSVGDisplayModule()
        display_module.draw_gftrackcand_trajectories = True
        display_module.draw_gftrackcands = True
        display_module.track_cands_store_array_name = "MCTrackCands"
        # main_path.add_module(display_module)

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
