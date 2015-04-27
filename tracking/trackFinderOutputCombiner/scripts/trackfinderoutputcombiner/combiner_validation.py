#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2

from trackfinderoutputcombiner.validation import TrasanTrackFinderRun, AddValidationMethod

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

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
