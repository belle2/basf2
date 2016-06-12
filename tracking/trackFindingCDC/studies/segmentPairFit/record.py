#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import os
import sys
import numpy as np

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin

from tracking.validation.utilities import is_primary
from tracking.validation.plot import ValidationPlot

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

import trackfindingcdc.validation.cdc_peelers as cdc_peelers

from trackfindingcdc import AxialStereoPairFitterModule

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentPairFitValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    n_events = 10000
    generator_module = "simple_gun"  # Rather high momentum tracks should make the tracks rather straight.

    monte_carlo = "no"
    flight_time_estimation = "none"
    flight_time_reestimation = False
    use_alpha_in_drift_length = False

    fit_method_name = "fuse-xy"
    # Specification for BrowseTFileOnTerminateRunMixin
    output_file_name = "SegmentPairFitValidation.root"

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-m',
            '--monte-carlo',
            choices=["no", "medium", "full"],
            default=self.monte_carlo,
            dest='monte_carlo',
            help='Amount of monte carlo information to be used in the segment generation.',
        )

        argument_parser.add_argument(
            "--fit",
            choices=["zreco", "fuse-simple", "fuse-sz", "fuse-xy"],
            default=self.fit_method_name,
            dest="fit_method_name",
            help=("Choose which fit positional information of the segment should be used. \n"
                  "* 'zreco' means the z coordinate is reconstructed and a linear sz fit is made. "
                  "No covariance between the circle and the linear sz part can be made.\n"
                  "* 'fuse-simple' means the axial-stereo fusion technique is used "
                  "without a prior z reconstruction step.\n"
                  "* 'fuse-xy-priority' means the axial-stereo fusion technique is used "
                  "with a prior z reconstruction step and a linear sz fit, "
                  "on which the fusion technique acts as a correction..\n"
                  "* 'fuse-sz-priority' is like 'fuse-xy-priority' but the z position is "
                  "reestimated after the sz linear fit has been performed, "
                  "which means that the residuals become visible in xy plane again.")
        )

        return argument_parser

    def get_fit_method(self):
        fit_method_name = self.fit_method_name

        if fit_method_name == 'zreco':
            sz_fitterSZ = Belle2.TrackFindingCDC.CDCSZFitter.getFitter()

            def z_reconstruction_fit(pair):
                return sz_iftter.update(pair)
            return z_reconstruction_fit

        elif fit_method_name == 'fuse-simple':
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion

            def simple_segment_pair_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructTrajectories(pair)
            return simple_segment_pair_fusion_fit

        elif fit_method_name == 'fuse-xy':
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion

            def xy_segment_pair_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructFuseTrajectories(pair, False)
            return xy_segment_pair_fusion_fit

        elif fit_method_name == 'fuse-sz':
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion

            def sz_segment_pair_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructFuseTrajectories(pair, True)
            return sz_segment_pair_fusion_fit

        else:
            raise ValueError("Unexpected fit_positions %s" % fit_method_name)

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        path.add_module("WireHitTopologyPreparer",
                        flightTimeEstimation="outwards")

        if self.monte_carlo == "no":
            # MC free - default
            path.add_module("SegmentFinderCDCFacetAutomaton",
                            SegmentOrientation="outwards")

        elif self.monte_carlo == "medium":
            # Medium MC - proper generation logic, but true facets and facet relations
            path.add_module("SegmentFinderCDCFacetAutomaton",
                            FacetFilter="truth",
                            FacetRelationFilter="truth",
                            SegmentOrientation="outwards")

        elif self.monte_carlo == "full":
            # Only true monte carlo segments, but make the positions realistic
            path.add_module("SegmentCreatorMCTruth",
                            reconstructedPositions=True)

        else:
            raise ValueError("Invalid degree of Monte Carlo information")

        path.add_module("SegmentFitter",
                        segments="CDCRecoSegment2DVector",
                        updateDriftLength=True,
                        useAlphaInDriftLength=True)

        path.add_module("TrackFinderCDCSegmentPairAutomatonDev",
                        WriteSegmentPairs=True,
                        SegmentPairFilter="truth",
                        SegmentPairRelationFilter="none")

        path.add_module(AxialStereoPairFitterModule(fit_method=self.get_fit_method()))
        path.add_module(SegmentPairFitValidationModule(self.output_file_name))

        return path


class SegmentPairFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segment pairs and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super(SegmentPairFitValidationModule, self).__init__(
            output_file_name=output_file_name,
            foreach="CDCSegmentPairVector"
        )
        self.mc_segment_lookup = None

    def initialize(self):
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()
        super(SegmentPairFitValidationModule, self).initialize()

    def prepare(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, segment_pair_relation):
        mc_segment_lookup = self.mc_segment_lookup
        from_segment = segment_pair_relation.getFromSegment()
        to_segment = segment_pair_relation.getToSegment()
        mc_particle = mc_segment_lookup.getMCParticle(from_segment)
        return (mc_particle and
                is_primary(mc_particle) and
                from_segment.size() > 3 and
                to_segment.size() > 3)

    def peel(self, segment_pair_relation):
        mc_segment_lookup = self.mc_segment_lookup

        from_segment = segment_pair_relation.getFromSegment()
        to_segment = segment_pair_relation.getToSegment()

        mc_particle = mc_segment_lookup.getMCParticle(from_segment)

        # Take the fit best at the middle of the segment pair
        fit3d_truth = mc_segment_lookup.getTrajectory3D(to_segment)

        truth_crops = dict(
            curvature_truth=fit3d_truth.getCurvatureXY(),
            tan_lambda_truth=fit3d_truth.getTanLambda(),
        )

        segment_pair_crops = cdc_peelers.peel_segment_pair_relation(segment_pair_relation)
        segment_pair_crops.update(truth_crops)
        return segment_pair_crops

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    save_tree = refiners.save_tree()

    save_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=False,
        groupby=[None, "superlayer_id_pair"],
        outlier_z_score=3.0)

    save_absolute_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        groupby=[None, "superlayer_id_pair"],
        outlier_z_score=3.0)

    save_tan_lambda_pull = refiners.save_pull_analysis(
        part_name="tan_lambda",
        quantity_name="tan #lambda",
        absolute=False,
        groupby=[None, "superlayer_id_pair"],
        outlier_z_score=3.0)

    save_fit_quality_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        select={"ndf": "ndf",
                "chi2": "#chi2",
                "p_value": "p-value",
                },
        groupby=[None, "superlayer_id_pair"],
        description="Distribution of {part_name} in the segment fits",
    )

    save_fit_quality_by_tan_lambda_profiles = refiners.save_profiles(
        select={
            "p_value": "fit p-value",
            "tan_lambda_truth": "true tan #lambda",
        },
        groupby=[None, "superlayer_id_pair"],
        x="true tan #lambda",
        y="fit p-value",
        check=("The {y_part_name} should be essentially the same over"
               "the whole range of the tan lambda spectrum"),
        description=("Investigating the reconstruction quality for different "
                     "tan lambda regions of the CDC. Most notably is the superlayer dependency."
                     "For stereo superlayers the curve is not flat but has distinct slope."),
        fit='line',
    )


def main():
    run = SegmentPairFitValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
