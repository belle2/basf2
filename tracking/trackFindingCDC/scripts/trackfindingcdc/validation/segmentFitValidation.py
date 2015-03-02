#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2

from trackfindingcdc import SegmentFitterModule

import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import os
import sys
import numpy as np
import array

from tracking.run.event_generation import StandardEventGenerationRun

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot

from tracking.modules import BrowseFileOnTerminateModule

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentFitValidationRun(StandardEventGenerationRun):
    segment_finder_module = "SegmentFinderCDCMCTruth"
    # segment_finder_module = "SegmentFinderCDCFacetAutomaton"
    fitter = Belle2.TrackFindingCDC.CDCRiemannFitter()
    fit_positions = "rl"
    output_file_name = "SegmentFitValidation.root"
    show_results = False

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
        riemann_fitter = Belle2.TrackFindingCDC.CDCRiemannFitter()
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

        argument_parser.add_argument(
            '-s',
            '--show',
            action='store_true',
            default=self.show_results,
            dest='show_results',
            help='Show generated plots in a TBrowser immediatly.',
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
                # Make a filtering step stripping potential Monte Carlo information in the reconstructed positions
                rlWireHitSegment = segment.getRLWireHitSegment()
                recoSegment = Belle2.CDCLocalTracking.CDCRecoSegment2D.reconstructUsingFacets(rlWireHitSegment)
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

        if self.show_results:
            browseFileOnTerminateModule = \
                BrowseFileOnTerminateModule(self.output_file_name)
            main_path.add_module(browseFileOnTerminateModule)

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        main_path.add_module(SegmentFitterModule(fit_method=self.get_fit_method()))
        main_path.add_module(SegmentFitValidationModule(self.output_file_name))

        return main_path


class SegmentFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super(SegmentFitValidationModule, self).__init__(foreach='CDCRecoSegment2DVector',
                                                         output_file_name=output_file_name)

        self.mc_segment_lookup = None

    def initialize(self):
        super(SegmentFitValidationModule, self).initialize()
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()

    def event(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()
        super(SegmentFitValidationModule, self).event()

    def pick(self, segment):
        mc_segment_lookup = self.mc_segment_lookup
        mc_particle = mc_segment_lookup.getMCParticle(segment)

        # Check that mc_particle is not a nullptr
        return mc_particle and is_primary(mc_particle) and segment.size() > 3

    def peel(self, segment):
        mc_segment_lookup = self.mc_segment_lookup

        mc_particle = mc_segment_lookup.getMCParticle(segment)

        # Retrive values
        fit3d_truth = mc_segment_lookup.getTrajectory3D(segment)
        fit2d_truth = fit3d_truth.getTrajectory2D()
        fit2d = segment.getTrajectory2D()
        iCurv = 0

        chi2 = fit2d.getChi2()
        ndf = fit2d.getNDF()

        curvature_truth = fit2d_truth.getCurvature()
        curvature_estimate = fit2d.getCurvature()

        return dict(
            size=segment.size(),
            stereo_type=segment.getStereoType(),
            superlayer_id=segment.getISuperLayer(),
            tan_lambda_truth=fit3d_truth.getTanLambda(),
            curvature_truth=curvature_truth,
            curvature_estimate=curvature_estimate,
            curvature_variance=fit2d.getLocalVariance(iCurv),
            chi2=chi2,
            ndf=ndf,
            p_value=prob(chi2, ndf),
        )

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    save_tree = refiners.save_tree()

    save_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=False,
        groupby=[None, "stereo_type", "superlayer_id"],
        outlier_z_score=3.0)

    save_absolute_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        groupby=[None, "stereo_type", "superlayer_id"],
        outlier_z_score=3.0)

    save_fit_quality_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        select={"ndf": "ndf",
                "chi2": "#chi2",
                "p_value": "p-value",
                "tan_lambda_truth": "true tan #lambda",
                },
        groupby=[None, "stereo_type", "superlayer_id"],
        description="Distribution of {part_name} in the segment fits",
    )

    @refiners.groupby(by=[None, "stereo_type", "superlayer_id"])
    def plot(self, crops, tdirectory, **kwds):
        curvature_truths = crops["curvature_truth"]
        curvature_estimates = crops["curvature_estimate"]
        curvature_residuals = curvature_estimates - curvature_truths

        p_values = crops["p_value"]
        tan_lambda_truths = crops["tan_lambda_truth"]

        validation_plots = []

        # Curvature residual versus tan lambda profile
        curvature_residuals_by_tan_lambda_truths_profile = ValidationPlot("curvature_residuals_versus_tan_lambda_truths")
        curvature_residuals_by_tan_lambda_truths_profile.profile(tan_lambda_truths, curvature_residuals, outlier_z_score=5.0)
        curvature_residuals_by_tan_lambda_truths_profile.xlabel = "true tan #lambda"
        curvature_residuals_by_tan_lambda_truths_profile.ylabel = "curvature residual (1/cm)"
        curvature_residuals_by_tan_lambda_truths_profile.title = "Curvature residuals versus tan lambda truths"
        curvature_residuals_by_tan_lambda_truths_profile.contact = CONTACT
        curvature_residuals_by_tan_lambda_truths_profile.check = "The residuals should be essentially the same over the whole range of the tan lambda spectrum"
        curvature_residuals_by_tan_lambda_truths_profile.description = ("Investigating the reconstruction quality for different tan lambda regions of the CDC. "
                                                                        "Most notably is the superlayer dependency. For stereo superlayers the curve is not flat but has distinct slope.")
        curvature_residuals_by_tan_lambda_truths_profile.fit_line()
        validation_plots.append(curvature_residuals_by_tan_lambda_truths_profile)

        # P value versus tan lambda profile
        p_values_by_tan_lambda_truths_profile = ValidationPlot("p_values_versus_tan_lambda_truths")
        p_values_by_tan_lambda_truths_profile.profile(tan_lambda_truths, p_values)
        p_values_by_tan_lambda_truths_profile.xlabel = "true tan #lambda"
        p_values_by_tan_lambda_truths_profile.ylabel = "p-value"
        p_values_by_tan_lambda_truths_profile.title = "Fit p-value versus tan #lambda truths"
        p_values_by_tan_lambda_truths_profile.contact = CONTACT
        p_values_by_tan_lambda_truths_profile.check = "The residuals should be essentially the same over the whole range of the tan lambda spectrum"
        p_values_by_tan_lambda_truths_profile.description = "Investigating the reconstruction quality for different tan lambda regions of the CDC."
        p_values_by_tan_lambda_truths_profile.fit_line()
        validation_plots.append(p_values_by_tan_lambda_truths_profile)

        for plot in validation_plots:
            plot.write(tdirectory)


def main():
    run = SegmentFitValidationRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
