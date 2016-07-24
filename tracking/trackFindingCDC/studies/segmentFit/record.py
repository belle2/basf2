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

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot

import tracking.harvest.peelers as peelers
import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners

import trackfindingcdc.validation.cdc_peelers as cdc_peelers

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentFitValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    n_events = 10000
    generator_module = "simple_gun"  # Rather high momentum tracks should make the tracks rather straight.
    monte_carlo = "no"
    karimaki_fit = False
    flight_time_estimation = "none"
    flight_time_reestimation = False
    use_alpha_in_drift_length = False
    flight_time_mass_scale = float("nan")

    fit_positions = "rlDriftCircle"
    fit_variance = "proper"

    output_file_name = "SegmentFitValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

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
            "-k",
            "--karimaki",
            dest="karimaki_fit",
            action="store_true",
            help='Use Karimaki fit instead of Riemann fit'
        )

        argument_parser.add_argument(
            "-fp",
            "--fit-pos",
            choices=["recoPos", "rlDriftCircle", "wirePos"],
            default=self.fit_positions,
            dest="fit_positions",
            help=("Choose which positional information the segment fit should be used. \n"
                  "* 'wirePos' means only the wire position\n"
                  "* 'recoPos' means only the reconstructed position\n"
                  "* 'rlDriftCircle' means only the drift circle with the right left passage\n")
        )

        argument_parser.add_argument(
            "-fv",
            "--fit-var",
            choices=["unit", "driftLength", "pseudo", "proper"],
            default=self.fit_variance,
            dest="fit_variance",
            help=("Choose which variance information the segment fit should be used. \n"
                  "* 'unit' means equal variance of 1\n"
                  "* 'driftLength' means inserting the drift length as variance, very improper because dimension mismatch\n"
                  "* 'pseudo' means the squared dirft length + plus the drift length variance "
                  "(correct dimension, proper lower bound)\n"
                  "* 'proper' means only the drift length variance\n")
        )

        argument_parser.add_argument(
            "-ft",
            "--flight-time-estimation",
            choices=["none", "outwards", "downwards"],
            default=self.flight_time_estimation,
            dest="flight_time_estimation",
            help=("Choose which estimation method for the time of flight should be use. \n"
                  "* 'none' no time of flight corrections\n"
                  "* 'outwards' means the minimal time needed to travel to the wire from the interaction point \n"
                  "* 'downwards' means the minimal time needed to travel to the wire from the y = 0 plane downwards \n")
        )

        argument_parser.add_argument(
            "-fr",
            "--flight-time-reestimation",
            action="store_true",
            dest="flight_time_reestimation",
            help=("Switch to reestimate drift length before fitting.")
        )

        argument_parser.add_argument(
            "-fa",
            "--use-alpha-in-drift-length",
            action="store_true",
            dest="use_alpha_in_drift_length",
            help=("Switch to serve the alpha angle to the drift length translator.")
        )

        argument_parser.add_argument(
            "-fm",
            "--flight-time-mass-scale",
            type=float,
            dest="flight_time_mass_scale",
            help=("Mass parameter to estimate the velocity in the time of flight estimation")
        )

        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        path.add_module("WireHitTopologyPreparer",
                        flightTimeEstimation=self.flight_time_estimation)

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
                        inputSegments="CDCRecoSegment2DVector",
                        karimakiFit=self.karimaki_fit,
                        fitPos=self.fit_positions,
                        fitVariance=self.fit_variance,
                        updateDriftLength=self.flight_time_reestimation,
                        useAlphaInDriftLength=self.use_alpha_in_drift_length,
                        tofMassScale=self.flight_time_mass_scale)

        path.add_module(SegmentFitValidationModule(self.output_file_name))

        return path


class SegmentFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super().__init__(foreach='CDCRecoSegment2DVector',
                         output_file_name=output_file_name)

        self.mc_segment_lookup = None

    def initialize(self):
        super().initialize()
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()

    def prepare(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, segment):
        mc_segment_lookup = self.mc_segment_lookup
        mc_particle = mc_segment_lookup.getMCParticle(segment)

        # Check that mc_particle is not a nullptr
        return mc_particle and is_primary(mc_particle) and segment.size() > 3

    def peel(self, segment):
        mc_segment_lookup = self.mc_segment_lookup

        segment_crops = cdc_peelers.peel_segment2d(segment)

        mc_particle = mc_segment_lookup.getMCParticle(segment)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)

        fit3d_truth = mc_segment_lookup.getTrajectory3D(segment)
        curvature_truth = fit3d_truth.getCurvatureXY()

        reconstructed_backward = mc_segment_lookup.isForwardOrBackwardToMCTrack(segment)

        if reconstructed_backward != 1:
            curvature_truth = -curvature_truth

        truth_crops = dict(
            tan_lambda_truth=fit3d_truth.getTanLambda(),
            curvature_truth=curvature_truth,
            curvature_residual=segment_crops["curvature_estimate"] - curvature_truth,
            curvature_pull=(segment_crops["curvature_estimate"] - curvature_truth) / segment_crops["curvature_variance"],
            reconstructed_backward=reconstructed_backward,
        )

        segment_crops.update(truth_crops)
        segment_crops.update(mc_particle_crops)
        return segment_crops

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    save_tree = refiners.save_tree()

    save_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=False,
        groupby=[None, "stereo_kind", "superlayer_id"],
        outlier_z_score=5.0,
        title_postfix="")

    save_absolute_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        groupby=[None, "stereo_kind", "superlayer_id"],
        outlier_z_score=5.0,
        title_postfix="")

    save_curvature_pull_by_tan_lambda = refiners.save_profiles(
        x="tan_lambda_truth",
        y="curvature_pull",
        groupby=[None, "stereo_kind", "superlayer_id"],
        outlier_z_score=5.0)

    save_curvature_residual_by_tan_lambda = refiners.save_profiles(
        x="tan_lambda_truth",
        y="curvature_residual",
        groupby=[None, "stereo_kind", "superlayer_id"],
        outlier_z_score=5.0)

    save_fit_quality_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        select={
            "ndf": "ndf",
            "chi2": "#chi2",
            "p_value": "p-value",
            "tan_lambda_truth": "true tan #lambda",
        },
        groupby=[None, "stereo_kind", "superlayer_id"],
        title="Histogram of {part_name}{stacked_by_indication}{stackby}",
        description="Distribution of {part_name} in the segment fits",
    )

    save_fit_quality_by_tan_lambda_profiles = refiners.save_profiles(
        select={
            'curvature_residual': 'curvature residual',
            "p_value": "fit p-value",
            "tan_lambda_truth": "true tan #lambda",
        },
        groupby=[None, "stereo_kind", "superlayer_id"],
        y=["curvature residual", "fit p-value"],
        x="true tan #lambda",
        check=("The {y_part_name} should be essentially the same over"
               "the whole range of the tan lambda spectrum"),
        description=("Investigating the reconstruction quality for different "
                     "tan lambda regions of the CDC. Most notably is the superlayer dependency."
                     "For stereo superlayers the curve is not flat but has distinct slope."),
        title="Profile of {y_part_name} by {x_part_name}",
        fit='line',
    )


def main():
    run = SegmentFitValidationRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
