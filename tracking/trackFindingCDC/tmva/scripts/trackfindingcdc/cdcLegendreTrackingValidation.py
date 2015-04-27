#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>CDCLegendreTrackingValidation.root</output>
  <description>This module validates that legendre track finding is capable of reconstructing tracks in Y(4S) runs.</description>
</header>
"""

import basf2

from tracking.validation.run import TrackingValidationRun
from tracking.run.event_generation import StandardEventGenerationRun
from tracking.validation.module import SeparatedTrackingValidationModule
from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

from ROOT import Belle2
from collections import deque
import numpy as np
from root_pandas import read_root
import pandas
import matplotlib.pyplot as plt
import seaborn as sb
sb.set_context("talk", font_scale=1.5)


class ReassignHits(basf2.Module):

    def __init__(
            self,
            old_cdc_hits_store_array_name="GoodCDCHits",
            new_cdc_hits_store_array_name="CDCHits",
            track_cands_store_array_name="TrackCands"):
        super(ReassignHits, self).__init__()

        self.old_cdc_hits_store_array_name = old_cdc_hits_store_array_name
        self.new_cdc_hits_store_array_name = new_cdc_hits_store_array_name
        self.track_cands_store_array_name = track_cands_store_array_name

    def event(self):
        track_cands = Belle2.PyStoreArray(self.track_cands_store_array_name)
        old_cdc_hits = Belle2.PyStoreArray(self.old_cdc_hits_store_array_name)

        for track_cand in track_cands:
            old_cdc_hit_ids = track_cand.getHitIDs(Belle2.Const.CDC)
            track_cand.reset()

            for old_cdc_hit_id in old_cdc_hit_ids:
                old_cdc_hit = old_cdc_hits[old_cdc_hit_id]
                new_cdc_hit = old_cdc_hit.getRelated(self.new_cdc_hits_store_array_name)
                track_cand.addHit(Belle2.Const.CDC, new_cdc_hit.getArrayIndex())


class CDCLegendre(StandardEventGenerationRun):
    tmva_cut = 0.0

    display_module = CDCSVGDisplayModule()
    display_module.draw_gftrackcand_trajectories = True
    display_module.draw_gftrackcand = True

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(CDCLegendre, self).create_path()

        background_hit_finder_module = basf2.register_module("BackgroundHitFinder")
        background_hit_finder_module.param("TMVACut", float(self.tmva_cut))

        cdctracking = basf2.register_module('CDCLegendreTracking')
        cdctracking.set_log_level(basf2.LogLevel.WARNING)

        if self.tmva_cut > 0:
            cdctracking.param('CDCHitsColName', "GoodCDCHits")

        cdc_stereo_combiner = basf2.register_module('CDCLegendreHistogramming')
        if self.tmva_cut > 0:
            cdc_stereo_combiner.param('CDCHitsColName', "GoodCDCHits")

        cdc_stereo_combiner.set_log_level(basf2.LogLevel.WARNING)

        track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruth')
        track_finder_mc_truth_module.param({
            'UseCDCHits': True,
            'WhichParticles': [],
            'GFTrackCandidatesColName': "MCTrackCands",
        })

        mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': "TrackCands",
        })

        validation_module = SeparatedTrackingValidationModule(
            name="",
            contact="",
            output_file_name="cut_values/validation_%.2f.root" %
            self.tmva_cut,
            expert_level=2)

        main_path.add_module(track_finder_mc_truth_module)

        if self.tmva_cut > 0:
            main_path.add_module(background_hit_finder_module)

        main_path.add_module(cdctracking)
        # main_path.add_module(cdc_stereo_combiner)
        if self.tmva_cut > 0:
            main_path.add_module(
                ReassignHits(
                    old_cdc_hits_store_array_name="GoodCDCHits",
                    new_cdc_hits_store_array_name="CDCHits",
                    track_cands_store_array_name="TrackCands"))
        main_path.add_module(mc_track_matcher_module)

        main_path.add_module(validation_module)

        return main_path


def run_many_times():
    for tmva_cut in np.arange(0, 1.0, 0.1):
        run = CDCLegendre()
        run.tmva_cut = tmva_cut
        run.configure_and_execute_from_commandline()
        print basf2.statistics


def plot_many_times():

    results = pandas.DataFrame()

    for tmva_cut in np.arange(0, 1.0, 0.1):
        output_file_name = "cut_values/validation_%.2f.root" % tmva_cut

        efficiency = read_root(output_file_name, tree_key="ExpertMCSideTrackingValidationModule_overview_figures_of_merit")
        finding_efficiency = efficiency.finding_efficiency[0]
        hit_efficiency = efficiency.hit_efficiency[0]
        rates = root_pandas.read_root(output_file_name, tree_key="ExpertPRSideTrackingValidationModule_overview_figures_of_merit")
        fake_rate = rates.fake_rate[0]
        clone_rate = rates.clone_rate[0]

        results = results.append({"tmva_cut": tmva_cut,
                                  "finding_efficiency": finding_efficiency,
                                  "hit_efficiency": hit_efficiency,
                                  "fake_rate": fake_rate,
                                  "clone_rate": clone_rate}, ignore_index=True)

    results = results.set_index(["tmva_cut"])

    plt.clf()
    plt.plot(results.index, 100.0 * results.finding_efficiency / results.finding_efficiency[0] - 100.0, label="Finding Efficiency")
    plt.plot(results.index, 100.0 * results.hit_efficiency / results.hit_efficiency[0] - 100.0, label="Hit Efficiency")
    plt.plot(results.index, 100.0 * results.fake_rate / results.fake_rate[0] - 100.0, label="Fake Rate")
    plt.plot(results.index, 100.0 * results.clone_rate / results.clone_rate[0] - 100.0, label="Clone Rate")
    plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3, ncol=2, mode="expand", borderaxespad=0.)
    plt.xlabel("tmva cut")
    plt.ylabel("Deviation in %")
    plt.xticks(results.index)
    plt.subplots_adjust(top=0.85, bottom=0.15)
    plt.savefig("tracking_validation.pdf")

    plt.clf()
    plt.plot(results.index, 100.0 * results.finding_efficiency, label="Finding Efficiency")
    plt.plot(results.index, 100.0 * results.hit_efficiency, label="Hit Efficiency")
    plt.plot(results.index, 100.0 * results.fake_rate, label="Fake Rate")
    plt.plot(results.index, 100.0 * results.clone_rate, label="Clone Rate")
    plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3, ncol=2, mode="expand", borderaxespad=0.)
    plt.xlabel("tmva cut")
    plt.ylabel("Rates in %")
    plt.xticks(results.index)
    plt.subplots_adjust(top=0.85, bottom=0.15)
    plt.savefig("tracking_validation_rates.pdf")


def run():
    validation_run = CDCLegendre()
    validation_run.configure_and_execute_from_commandline()

if __name__ == '__main__':
    # run_many_times()
    plot_many_times()
