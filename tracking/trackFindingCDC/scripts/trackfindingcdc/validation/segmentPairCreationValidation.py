#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import numpy as np

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtrackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.metamodules as metamodules

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin

from trackfindingcdc import (
    SegmentFitterModule
)


import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentPairCreationValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    segment_finder_module = basf2.register_module("SegmentFinderCDCMCTruth")
    segment_finder_module.param({"MinCDCHits": 4})

    segment_pair_finder_module = basf2.register_module("TrackFinderCDCSegmentPairAutomatonDev")
    segment_pair_finder_module.param({
        "WriteSegmentPairs": True,
        "SegmentPairFilter": "all",
        "SegmentPairNeighborChooser": "none",
    })

    py_profile = True
    output_file_name = "SegmentCreationValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(SegmentPairCreationValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(SegmentPairCreationValidationRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        main_path.add_module(SegmentFitterModule())

        segment_pair_finder_module = self.get_basf2_module(self.segment_pair_finder_module)
        main_path.add_module(segment_pair_finder_module)

        # main_path.add_module(AxialStereoPairFitterModule())
        validation_module = SegmentPairCreationValidationModule(output_file_name=self.output_file_name)
        if self.py_profile:
            main_path.add_module(metamodules.PyProfilingModule(validation_module))
        else:
            main_path.add_module(validation_module)

        return main_path


class SegmentPairCreationValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super(SegmentPairCreationValidationModule, self).__init__(foreach="CDCAxialStereoSegmentPairVector",
                                                                  output_file_name=output_file_name)
        self.mc_segment_lookup = None
        self.mc_segment_pair_filter = None
        self.axial_stereo_fusion = None

    def initialize(self):
        super(SegmentPairCreationValidationModule, self).initialize()
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()
        self.mc_segment_pair_filter = Belle2.TrackFindingCDC.MCAxialStereoSegmentPairFilter()
        self.axial_stereo_fusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion

    def prepare(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, axial_stereo_segment_pair):
        mc_segment_lookup = self.mc_segment_lookup
        start_segment = axial_stereo_segment_pair.getStartSegment()
        end_segment = axial_stereo_segment_pair.getEndSegment()
        mc_particle = mc_segment_lookup.getMCParticle(start_segment)
        return (mc_particle and
                is_primary(mc_particle) and
                start_segment.size() > 3 and
                end_segment.size() > 3)

    def peel(self, axial_stereo_segment_pair):
        crops = self.peel_target(axial_stereo_segment_pair)
        crops.update(self.peel_mc(axial_stereo_segment_pair))
        crops.update(self.peel_fit(axial_stereo_segment_pair))
        return crops

    def peel_target(self, axial_stereo_segment_pair):
        mc_weight = self.mc_segment_pair_filter.isGoodAxialStereoSegmentPair(axial_stereo_segment_pair)
        mc_decision = np.isfinite(mc_weight)  # Filters for nan

        return dict(
            mc_weight=mc_weight,
            mc_decision=mc_decision,
        )

    def peel_mc(self, axial_stereo_segment_pair):
        mc_segment_lookup = self.mc_segment_lookup

        end_segment = axial_stereo_segment_pair.getEndSegment()

        # Take the fit best at the middle of the segment pair
        mc_particle = mc_segment_lookup.getMCParticle(end_segment)
        fit3d_truth = mc_segment_lookup.getTrajectory3D(end_segment)

        return dict(
            curvature_truth=fit3d_truth.getCurvatureXY(),
            tan_lambda_truth=fit3d_truth.getTanLambda(),
        )

    def peel_fit(self, axial_stereo_segment_pair):
        fitless_crops = self.peel_fitless(axial_stereo_segment_pair)

        select_fitless = fitless_crops["select_fitless"]
        if select_fitless:
            # Now fit
            self.fit(axial_stereo_segment_pair)
            fit3d = axial_stereo_segment_pair.getTrajectory3D()

            i_curv = 0
            i_tan_lambda = 3

            chi2 = fit3d.getChi2()
            ndf = fit3d.getNDF()

            curvature_estimate = fit3d.getCurvatureXY()
            curvature_variance = fit3d.getLocalVariance(i_curv)

            tan_lambda_estimate = fit3d.getTanLambda()
            tan_lambda_variance = fit3d.getLocalVariance(i_tan_lambda)

            chi2 = chi2
            ndf = ndf
            p_value = prob(chi2, ndf)
            select = True

        else:
            nan = float('nan')
            curvature_estimate = nan
            curvature_variance = nan

            tan_lambda_estimate = nan
            tan_lambda_variance = nan

            chi2 = nan
            ndf = nan
            p_value = nan

        crops = dict(
            curvature_estimate=curvature_estimate,
            curvature_variance=curvature_variance,

            tan_lambda_estimate=tan_lambda_estimate,
            tan_lambda_variance=tan_lambda_variance,

            chi2=chi2,
            ndf=ndf,
            p_value=p_value,
        )

        if select_fitless:
            crops["select"] = self.select(crops)
        else:
            crops["select"] = False

        crops.update(fitless_crops)

        return crops

    def peel_fitless(self, axial_stereo_segment_pair):
        # Try to make some judgements without executing the common fit.
        mc_segment_lookup = self.mc_segment_lookup

        start_segment = axial_stereo_segment_pair.getStartSegment()
        end_segment = axial_stereo_segment_pair.getEndSegment()

        start_fit2d = start_segment.getTrajectory2D()
        end_fit2d = end_segment.getTrajectory2D()

        start_superlayer_id = start_segment.getISuperLayer()
        end_superlayer_id = end_segment.getISuperLayer()

        sorted_superlayer_ids = sorted([start_superlayer_id, end_superlayer_id])

        superlayer_id_pair = 10.0 * sorted_superlayer_ids[1] + sorted_superlayer_ids[0]

        fitless_crops = dict(
            start_superlayer_id=start_superlayer_id,
            end_superlayer_id=end_superlayer_id,
            superlayer_id_pair=superlayer_id_pair,

            start_size=start_segment.size(),
            end_size=end_segment.size(),

            start_curvature_estimate=start_fit2d.getCurvature(),
            end_curvature_estimate=end_fit2d.getCurvature(),

            delta_phi=axial_stereo_segment_pair.computeDeltaPhiAtSuperLayerBound(),
            is_coaligned=axial_stereo_segment_pair.computeIsCoaligned(),

            start_is_before_end=axial_stereo_segment_pair.computeStartIsBeforeEnd(),
            end_is_after_start=axial_stereo_segment_pair.computeEndIsAfterStart(),
        )

        fitless_crops["select_fitless"] = self.select_fitless(fitless_crops)
        return fitless_crops

    def fit(self, axial_stereo_segment_pair):
        self.axial_stereo_fusion.reconstructFuseTrajectories(axial_stereo_segment_pair, True)

    delta_phi_cut_value = 1.0
    is_after_cut_value = 1.0

    def select_fitless(self, fitless_crops):
        delta_phi = fitless_crops["delta_phi"]
        start_is_before_end = fitless_crops["start_is_before_end"]
        end_is_after_start = fitless_crops["end_is_after_start"]
        is_after_select = (abs(start_is_before_end) < self.is_after_cut_value) & (abs(end_is_after_start) < self.is_after_cut_value)
        return (abs(delta_phi) < self.delta_phi_cut_value) & is_after_select

    def select(self, crops):
        return True

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    save_tree = refiners.save_tree()

    # Investigate the preselection
    save_fitless_selection_variables_histograms = refiners.save_histograms(
        select=["mc_decision", "delta_phi", "start_is_before_end", "end_is_after_start", "is_coaligned"],
        outlier_z_score=5.0,
        allow_discrete=True,
        stackby="mc_decision",
        folder_name="fitless_selection_variables",
    )

    save_view_is_after_cut_histograms = refiners.save_histograms(
        select=["mc_decision", "start_is_before_end", "end_is_after_start"],
        lower_bound=-is_after_cut_value,
        upper_bound=is_after_cut_value,
        stackby="mc_decision",
        folder_name="view_fitless_cuts",
    )

    save_view_delta_phi_cut_histograms = refiners.save_histograms(
        select=["mc_decision", "delta_phi"],
        lower_bound=-delta_phi_cut_value,
        upper_bound=delta_phi_cut_value,
        stackby="mc_decision",
        folder_name="view_fitless_cuts",
    )

    # Investigate the main selection
    save_selection_variables_after_fitless_selection_histograms = refiners.save_histograms(
        select=["mc_decision", "chi2", "ndf", "p_value"],
        outlier_z_score=5.0,
        allow_discrete=True,
        stackby="mc_decision",
        folder_name="selection_variables_after_fitless_selection",
        filter_on="select_fitless",
    )

    # TODO: Is this interesting enough to keep it.
    save_p_value_over_curvature_profile = refiners.save_profiles(
        select={"p_value": "p-value", "curvature_truth": "true curvature"},
        y="p-value",
        folder_name="selection_variables_after_fitless_selection",
        title=r"$p$-value versus true curvature after fitless selection",
        filter_on="select_fitless",
    )

    @refiners.context(groupby=[None, "superlayer_id_pair"], exclude_groupby=False)
    def print_signal_number(self, crops, tdirectory, **kwds):
        info = get_logger().info

        start_superlayer_ids = crops["start_superlayer_id"]
        end_superlayer_ids = crops["end_superlayer_id"]

        superlayer_id_pair = crops["superlayer_id_pair"]
        info("Number of pairs in superlayers %s : %s", np.unique(superlayer_id_pair), len(superlayer_id_pair))

        mc_decisions = crops["mc_decision"]
        n = len(mc_decisions)
        n_signal = np.sum(mc_decisions)
        n_background = n - n_signal
        info("#Signal : %s", n_signal)
        info("#Background : %s", n_background)

        fitless_selections = np.nonzero(crops["select_fitless"])
        info("#Signal after precut : %s", np.sum(mc_decisions[fitless_selections]))
        info("#Background after precut : %s", np.sum(1 - mc_decisions[fitless_selections]))


def main():
    run = SegmentPairCreationValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
