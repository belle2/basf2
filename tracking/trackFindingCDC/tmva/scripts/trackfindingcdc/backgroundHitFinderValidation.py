#!/usr/bin/env python3
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

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import is_primary
from tracking.validation.plot import ValidationPlot

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.metamodules as metamodules

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

import logging

try:
    import root_pandas
except ImportError:
    print("do a pip install git+https://github.com/ibab/root_pandas")
import pandas
import matplotlib.pyplot as plt
import seaborn as sb


def get_logger():
    return logging.getLogger(__name__)

CONTACT = "ucddn@student.kit.edu"


class BackgroundHitFinderValidationRun(StandardEventGenerationRun):

    tmva_cut = 0.1
    tuples_file_name = "BackgroundHitFinderValidation"

    def create_argument_parser(self, **kwds):
        argument_parser = super(BackgroundHitFinderValidationRun, self).create_argument_parser(**kwds)
        argument_parser.add_argument(
            '-t',
            '--tmva-cut',
            dest='tmva_cut',
            default=self.tmva_cut,
            help='Cut for the TMVA in the module.'
        )

        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(BackgroundHitFinderValidationRun, self).create_path()

        background_hit_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
        background_hit_finder_module.param({
            "ClusterFilter": "tmva",
            "ClusterFilterParameters": {"cut": str(self.tmva_cut)},
            "RemainingCDCHitsStoreArrayName": "GoodCDCHits",
            "FacetFilter": "none",
            "FacetRelationFilter": "none",
        })

        main_path.add_module(background_hit_finder_module)
        main_path.add_module(BackgroundHitFinderValidationModule("CDCHits", output_file_name=self.tuples_file_name + "_old.root"))
        main_path.add_module(
            BackgroundHitFinderValidationModule(
                "GoodCDCHits",
                output_file_name=self.tuples_file_name +
                "_new.root"))

        return main_path


class BackgroundHitFinderValidationModule(harvesting.HarvestingModule):

    def __init__(self, cdc_hit_col_name, output_file_name):
        super(BackgroundHitFinderValidationModule, self).__init__(foreach=cdc_hit_col_name,
                                                                  output_file_name=output_file_name)

        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()

    def prepare(self):
        self.mc_hit_lookup.fill()

    def pick(self, hit):
        return True

    def peel(self, hit):
        mc_hit_lookup = self.mc_hit_lookup

        if self.foreach != "CDCHits":
            oldHit = hit.getRelated("CDCHits")
        else:
            oldHit = hit

        is_background = oldHit.getRelated("CDCSimHits").getBackgroundTag() != Belle2.CDCSimHit.bg_none

        return dict(
            is_background=is_background
        )

    save_tree = refiners.save_tree(folder_name="tree")


def main():
    run = BackgroundHitFinderValidationRun()
    run.configure_and_execute_from_commandline()


def plot():
    old_hits = root_pandas.read_root(
        "BackgroundHitFinderValidation_old.root",
        tree_key="tree/BackgroundHitFinderValidationModule_tree")
    new_hits = root_pandas.read_root(
        "BackgroundHitFinderValidation_new.root",
        tree_key="tree/BackgroundHitFinderValidationModule_tree")

    plt.hist(old_hits.is_background, label="before")
    plt.hist(new_hits.is_background, label="after")
    plt.legend()
    plt.savefig("background.png")

    bkg_old = len(old_hits.loc[old_hits.is_background == 1])
    signal_old = len(old_hits.loc[old_hits.is_background == 0])
    bkg_new = len(new_hits.loc[new_hits.is_background == 1])
    signal_new = len(new_hits.loc[new_hits.is_background == 0])

    print((bkg_old, "->", bkg_new, "(-%.2f %%)" % (100.0 - 100.0 * bkg_new / bkg_old)))
    print((signal_old, "->", signal_new, "(-%.2f %%)" % (100.0 - 100.0 * signal_new / signal_old)))


def run_many_times():
    for tmva_cut in np.arange(0, 1.0, 0.1):
        run = BackgroundHitFinderValidationRun()
        run.tmva_cut = tmva_cut
        run.tuples_file_name = "cut_values/cut_value_%.2f" % tmva_cut
        run.configure_and_execute_from_commandline()


def plot_many_times():

    results = pandas.DataFrame()
    for tmva_cut in np.arange(0, 1.0, 0.1):

        tuples_file_name = "cut_values/cut_value_%.2f" % tmva_cut

        old_hits = root_pandas.read_root(
            tuples_file_name +
            "_old.root",
            tree_key="tree/BackgroundHitFinderValidationModule_tree")
        new_hits = root_pandas.read_root(
            tuples_file_name +
            "_new.root",
            tree_key="tree/BackgroundHitFinderValidationModule_tree")

        bkg_old = len(old_hits.loc[old_hits.is_background == 1])
        signal_old = len(old_hits.loc[old_hits.is_background == 0])
        bkg_new = len(new_hits.loc[new_hits.is_background == 1])
        signal_new = len(new_hits.loc[new_hits.is_background == 0])

        results = results.append({"tmva_cut": tmva_cut,
                                  "bkg_old": bkg_old,
                                  "signal_old": signal_old,
                                  "bkg_new": bkg_new,
                                  "signal_new": signal_new}, ignore_index=True)

    results = results.set_index(["tmva_cut"])
    print(results)

    plt.clf()
    plt.plot(results.index, results.bkg_new)
    plt.xlabel("tmva cut")
    plt.ylabel("number of background hits")
    plt.title("Background")
    plt.savefig("background.pdf")

    plt.clf()
    plt.plot(results.index, 100.0 * results.bkg_new / results.bkg_old)
    plt.xlabel("tmva cut")
    plt.ylabel("ratio passed background hits / all background hits")
    plt.title("Background rate")
    plt.savefig("background_rate.pdf")

    plt.clf()
    plt.plot(results.index, results.signal_new)
    plt.xlabel("tmva cut")
    plt.ylabel("number of signal hits")
    plt.title("Signal")
    plt.savefig("signal.pdf")

    plt.clf()
    plt.plot(results.index, 100.0 * results.signal_new / results.signal_old)
    plt.xlabel("tmva cut")
    plt.ylabel("ratio passed signal hits / all signal hits")
    plt.title("Signal rate")
    plt.savefig("signal_rate.pdf")

    plt.clf()
    plt.plot(100.0 * results.signal_new / results.signal_old, 100.0 - 100.0 * results.bkg_new / results.bkg_old)
    plt.xlabel("Signal")
    plt.ylabel("Background rejection")
    plt.title("ROC")
    plt.savefig("roc.pdf")


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    run_many_times()
    plot_many_times()
