#!/usr/bin/env python
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

from trackfindingcdc import SegmentFitterModule, AxialStereoPairFitterModule

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentPairFitValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    segment_finder_module = basf2.register_module("SegmentFinderCDCMCTruth")
    segment_finder_module.param({"MinCDCHits": 4})

    segment_pair_finder_module = basf2.register_module("TrackFinderCDCSegmentPairAutomatonDev")
    segment_pair_finder_module.param({
        "WriteSegmentPairs": True,
        "SegmentPairFilter": "mc",
        "SegmentPairNeighborChooser": "none",
    })
    fit_method_name = "fuse-xy"
    output_file_name = "SegmentFitValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(SegmentPairFitValidationRun, self).create_argument_parser(**kwds)

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

        argument_parser.add_argument(
            "--fit",
            choices=["zreco", "fuse-simple", "fuse-sz", "fuse-xy"],
            default=self.fit_method_name,
            dest="fit_method_name",
            help=("Choose which fit positional information of the segment should be used. \n"
                  "* 'zreco' means the z coordinate is reconstructed and a linear sz fit is made. No covariance between the circle and the linear sz part can be made.\n"
                  "* 'fuse-simple' means the axial-stereo fusion technique is used without a prior z reconstruction step.\n"
                  "* 'fuse-xy-priority' means the axial-stereo fusion technique is used with a prior z reconstruction step and a linear sz fit, on which the fusion technique acts as a correction..\n"
                  "* 'fuse-sz-priority' is like 'fuse-xy-priority' but the z position is reestimated after the sz linear fit has been performed, which means that the residuals become visible in xy plane again.")
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

            def simple_axial_stereo_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructTrajectories(pair)
            return simple_axial_stereo_fusion_fit

        elif fit_method_name == 'fuse-xy':
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion

            def xy_axial_stereo_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructFuseTrajectories(pair, False)
            return xy_axial_stereo_fusion_fit

        elif fit_method_name == 'fuse-sz':
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion

            def sz_axial_stereo_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructFuseTrajectories(pair, True)
            return sz_axial_stereo_fusion_fit

        else:
            raise ValueError("Unexpected fit_positions %s" % fit_method_name)

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(SegmentPairFitValidationRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        main_path.add_module(SegmentFitterModule())

        segment_pair_finder_module = self.get_basf2_module(self.segment_pair_finder_module)
        main_path.add_module(segment_pair_finder_module)

        main_path.add_module(AxialStereoPairFitterModule(fit_method=self.get_fit_method()))
        main_path.add_module(SegmentPairFitValidationModule(self.output_file_name))

        return main_path


class SegmentPairFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segment pairs and compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super(SegmentPairFitValidationModule, self).__init__(output_file_name=output_file_name,
                                                             foreach="CDCAxialStereoSegmentPairVector")
        self.mc_segment_lookup = None

    def initialize(self):
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()
        super(SegmentPairFitValidationModule, self).initialize()

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
        mc_segment_lookup = self.mc_segment_lookup

        start_segment = axial_stereo_segment_pair.getStartSegment()
        end_segment = axial_stereo_segment_pair.getEndSegment()

        mc_particle = mc_segment_lookup.getMCParticle(start_segment)

        # Take the fit best at the middle of the segment pair
        fit3d_truth = mc_segment_lookup.getTrajectory3D(end_segment)
        fit3d = axial_stereo_segment_pair.getTrajectory3D()
        start_fit2d = start_segment.getTrajectory2D()
        end_fit2d = end_segment.getTrajectory2D()
        i_curv = 0
        i_tan_lambda = 3

        chi2 = fit3d.getChi2()
        ndf = fit3d.getNDF()
        curvature_truth = fit3d_truth.getCurvatureXY()
        curvature_estimate = fit3d.getCurvatureXY()

        start_superlayer_id = start_segment.getISuperLayer()
        end_superlayer_id = end_segment.getISuperLayer()

        sorted_superlayer_ids = sorted([start_superlayer_id, end_superlayer_id])

        superlayer_id_pair = 10.0 * sorted_superlayer_ids[1] + sorted_superlayer_ids[0]

        return dict(
            start_superlayer_id=start_superlayer_id,
            end_superlayer_id=end_superlayer_id,
            superlayer_id_pair=superlayer_id_pair,
            start_curvature_estimate=start_fit2d.getCurvature(),
            end_curvature_estimate=end_fit2d.getCurvature(),

            curvature_truth=curvature_truth,
            curvature_estimate=curvature_estimate,
            curvature_variance=fit3d.getLocalVariance(i_curv),

            tan_lambda_truth=fit3d_truth.getTanLambda(),
            tan_lambda_estimate=fit3d.getTanLambda(),
            tan_lambda_variance=fit3d.getLocalVariance(i_tan_lambda),

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

    @refiners.context(groupby=[None, "superlayer_id_pair"])
    def plot(self, crops, tdirectory, **kwds):
        tan_lambda_truths = crops["tan_lambda_truth"]
        p_values = crops["p_value"]

        validation_plots = []

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
    run = SegmentPairFitValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
