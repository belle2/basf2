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

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

import trackfindingcdc.validation.cdc_peelers as cdc_peelers

from trackfindingcdc import SegmentFitterModule

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentFitValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    n_events = 1000
    generator_module = "simple_gun"  # Rather high momentum tracks should make the tracks rather straight.

    # All MC segments
    # segment_finder_module = "SegmentFinderCDCMCTruth"

    # MC Free
    segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
    segment_finder_module.param({
        "FacetFilter": "realistic",
        "FacetRelationFilter": "simple",
        "SegmentOrientation": "outwards",  # as is from mc filters
    })

    # Proper generation logic of facets, filters use MC information
    # segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
    # segment_finder_module.param({
    #         "FacetFilter" : "mc",
    #         "FacetFilterParameters" : { "allowReverse" : false },
    #         "FacetRelationFilter" : "mc",
    #         "FacetRelationFilterParameters" : { "allowReverse" : false },
    #         })

    fitter = Belle2.TrackFindingCDC.CDCRiemannFitter()
    fit_positions = "rl"
    output_file_name = "SegmentFitValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(SegmentFitValidationRun, self).create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-f',
            '--segment-finder',
            choices=NonstrictChoices([
                'SegmentFinderCDCMCTruth',
                'SegmentFinderCDCFacetAutomaton'
            ]),
            default=self.segment_finder_module,
            dest='segment_finder_module',
            help='Name of the segment finder module to be used to generate the segments',
        )

        karimaki_fitter = Belle2.TrackFindingCDC.CDCKarimakiFitter()
        karimaki_fitter.useOnlyOrientation()
        riemann_fitter = Belle2.TrackFindingCDC.CDCRiemannFitter()
        riemann_fitter.useOnlyOrientation()
        argument_parser.add_argument(
            "-k",
            "--karimaki",
            dest="fitter",
            action="store_const",
            const=karimaki_fitter, default=riemann_fitter,
            help='Use Karimaki fit instead of Riemann fit'
        )

        argument_parser.add_argument(
            "-p",
            "--positions",
            choices=["wire", "reco", "circle", "rl"],
            default=self.fit_positions,
            dest="fit_positions",
            help=("Choose which positional information the segment fit should use. \n"
                  "* 'wire' means only the wire position\n"
                  "* 'reco' means only the reconstructed position\n"
                  "* 'circle' means only the drift circle *without* the right left passage\n"
                  "* 'rl' means only the drift circle with the right left passage\n")
        )

        return argument_parser

    def get_fit_method(self):
        fitter = self.fitter

        fit_positions = self.fit_positions

        if fit_positions == "wire":
            def fit_method(segment):
                wireSegment = segment.getWireSegment()
                return fitter.fit(wireSegment)

        elif fit_positions == "reco":
            fitter.useOnlyPosition()

            def fit_method(segment):
                # Make a filtering step stripping potential Monte Carlo information
                # in the reconstructed positions
                rlWireHitSegment = segment.getRLWireHitSegment()
                CDCRecoSegment2D = Belle2.TrackFindingCDC.CDCRecoSegment2D
                recoSegment = CDCRecoSegment2D.reconstructUsingFacets(rlWireHitSegment)
                return fitter.fit(recoSegment)

        elif fit_positions == "circle":
            def fit_method(segment):
                wireHitSegment = segment.getWireHitSegment()
                return fitter.fit(wireHitSegment)

        elif fit_positions == "rl":
            def fit_method(segment):
                return fitter.fit(segment)

        else:
            raise ValueError("Unexpected fit_positions %s" % fit_positions)

        return fit_method

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(SegmentFitValidationRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        main_path.add_module(SegmentFitterModule(fit_method=self.get_fit_method()))
        main_path.add_module(SegmentFitValidationModule(self.output_file_name))

        return main_path


class SegmentFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super(SegmentFitValidationModule, self).__init__(foreach='CDCRecoSegment2DVector',
                                                         output_file_name=output_file_name)

        self.mc_segment_lookup = None

    def initialize(self):
        super(SegmentFitValidationModule, self).initialize()
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
        fit3d_truth = mc_segment_lookup.getTrajectory3D(segment)
        curvature_truth = fit3d_truth.getCurvatureXY()

        truth_crops = dict(
            tan_lambda_truth=fit3d_truth.getTanLambda(),
            curvature_truth=curvature_truth,
            curvature_residual=segment_crops["curvature_estimate"] - curvature_truth,
        )
        segment_crops.update(truth_crops)

        return segment_crops

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    save_tree = refiners.save_tree()

    save_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=False,
        groupby=[None, "stereo_type", "superlayer_id"],
        outlier_z_score=3.0,
        title_postfix="")

    save_absolute_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        groupby=[None, "stereo_type", "superlayer_id"],
        outlier_z_score=3.0,
        title_postfix="")

    save_fit_quality_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        select={"ndf": "ndf",
                "chi2": "#chi2",
                "p_value": "p-value",
                "tan_lambda_truth": "true tan #lambda",
                },
        groupby=[None, "stereo_type", "superlayer_id"],
        title="Histogram of {part_name}{stacked_by_indication}{stackby}",
        description="Distribution of {part_name} in the segment fits",
    )

    save_fit_quality_by_tan_lambda_profiles = refiners.save_profiles(
        select={
            (lambda curvature_estimate, curvature_truth, **kwd:
             curvature_estimate - curvature_truth): 'curvature residual',
            "p_value": "fit p-value",
            "tan_lambda_truth": "true tan #lambda",
        },
        groupby=[None, "stereo_type", "superlayer_id"],
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
