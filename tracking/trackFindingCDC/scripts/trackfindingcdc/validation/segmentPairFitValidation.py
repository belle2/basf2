#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2

import ROOT
from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import os
import sys
import numpy as np
import array

from tracking.run.event_generation import ReadOrGenerateEventsRun

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot
from tracking.validation.pull import PullAnalysis

from tracking.modules import BrowseFileOnTerminateModule

import logging


def get_logger():
    return logging.getLogger(__name__)


from trackfindingcdc import (
    SegmentFitterModule,
    MCAxialStereoPairCreatorModule,
    AxialStereoPairFitterModule)


class SegmentFitValidationRun(ReadOrGenerateEventsRun):
    segment_finder_module = "SegmentFinderCDCMCTruth"
    segment_pair_finder_module = MCAxialStereoPairCreatorModule()
    fit_method_name = "fuse-xy"
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
        fit_method_name = self.fit_method_name

        if fit_method_name == 'zreco':
            sz_fitterSZ = Belle2.CDCLocalTracking.CDCSZFitter.getFitter()

            def z_reconstruction_fit(pair):
                return sz_iftter.update(pair)
            return z_reconstruction_fit

        elif fit_method_name == 'fuse-simple':
            CDCAxialStereoFusion = Belle2.CDCLocalTracking.CDCAxialStereoFusion

            def simple_axial_stereo_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructTrajectories(pair)
            return simple_axial_stereo_fusion_fit

        elif fit_method_name == 'fuse-xy-priority':
            CDCAxialStereoFusion = Belle2.CDCLocalTracking.CDCAxialStereoFusion

            def xy_axial_stereo_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructFuseTrajectories(pair, False)
            return xy_axial_stereo_fusion_fit

        elif fit_method_name == 'fuse-sz-priority':
            CDCAxialStereoFusion = Belle2.CDCLocalTracking.CDCAxialStereoFusion

            def sz_axial_stereo_fusion_fit(pair):
                return CDCAxialStereoFusion.reconstructFuseTrajectories(pair, True)
            return sz_axial_stereo_fusion_fit

        else:
            raise ValueError("Unexpected fit_positions %s" % fit_method_name)

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

        main_path.add_module(SegmentFitterModule())

        segment_pair_finder_module = self.get_basf2_module(self.segment_pair_finder_module)
        main_path.add_module(segment_pair_finder_module)

        main_path.add_module(AxialStereoPairFitterModule(fit_method=self.get_fit_method()))
        main_path.add_module(SegmentPairFitValidationModule(self.output_file_name))

        return main_path


class SegmentPairFitValidationModule(basf2.Module):

    """Module to collect information about the generated segment pairs and compose validation plots on terminate."""

    def __init__(self, output_file_name):
        self.output_file_name = output_file_name
        super(SegmentPairFitValidationModule, self).__init__()
        self.mc_segment_lookup = None

        self.start_superlayer_ids = array.array("f")
        self.end_superayer_id = array.array("f")

        self.start_sizes = array.array("f")
        self.end_sizes = array.array("f")

        self.curvature_estimates = array.array("f")
        self.curvature_variances = array.array("f")
        self.curvature_truths = array.array("f")

        self.start_curvature_estimates = array.array("f")
        self.end_curvature_estimates = array.array("f")

        self.tan_lambda_estimates = array.array("f")
        self.tan_lambda_variances = array.array("f")
        self.tan_lambda_truths = array.array("f")

        self.chi2s = array.array("f")
        self.ndfs = array.array("f")
        self.p_values = array.array("f")

    def initialize(self):
        self.mc_segment_lookup = Belle2.CDCLocalTracking.CDCMCSegmentLookUp.getInstance()

    def event(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()
        self.gather_segment_pairs()

    def gather_segment_pairs(self):
        mc_segment_lookup = self.mc_segment_lookup

        # TODO: fix me
        axial_stereo_segment_pairs = CDCAxialStereoSegmentPair.getStoreArray()

        for axial_stereo_segment_pair in axial_stereo_segment_pairs:
            if not self.pick_segment_pair(axial_stereo_segment_pair):
                continue

            mc_particle = mc_segment_lookup.getMCParticle(start_segment)

            start_segment = axial_stereo_segment_pair.getStartSegment()
            end_segment = axial_stereo_segment_pair.getEndSegment()

            start_superlayer_id = start_segment.getISuperLayer()
            end_superlayer_id = end_segment.getISuperLayer()

            # Take the fit best at the middle of the segment pair
            fit3d_truth = mc_segment_lookup.getTrajectory3D(end_segment)

            fit3d = axial_stereo_segment_pair.getTrajectory3D()
            start_fit2d = start_segment.getTrajectory2D()
            end_fit2d = end_segment.getTrajectory2D()

            curvature_estimate = fit3d.getCurvatureXY()
            i_curv = 0
            curvature_variance = fit3d.getLocalVariance(i_curv)
            curvature_truth = fit3d_truth.getCurvatureXY()

            tan_lambda_estimate = fit3d.getTanLambda()
            i_tan_lambda = 3
            tan_lambda_variance = fit3d.getLocalVariance(i_tan_lambda)
            tan_lambda_truth = fit3d_truth.getTanLambda()

            chi2 = fit3d.getChi2()
            ndf = fit3d.getNDF()
            p_value = prob(chi2, ndf)

            self.start_superlayer_ids.append(start_superlayer_id)
            self.end_superlayer_ids.append(end_superlayer_id)

            self.start_sizes.append(start_segment.size())
            self.end_sizes.append(end_segment.size())

            self.curvature_estimates.append(curvature_estimate)
            self.curvature_variances.append(curvature_variance)
            self.curvature_truths.append(curvature_truth)

            self.start_curvature_estimates.append(start_fit2d.getCurvature())
            self.end_curvature_estimates.append(end_fFit2d.getCurvature())

            self.tan_lambda_estimates.append(tan_lambda_estimate)
            self.tan_lambda_variances.append(tan_lambda_variance)
            self.tan_lambda_truths.append(tan_lambda_truth)

            self.chi2s.append(chi2)
            self.ndfs.append(ndf)
            self.p_values.append(p_value)

    def pick_segment_pair(self, segment_pair):
        mc_segment_lookup = self.mc_segment_lookup
        start_segment = segment_pair.getStartSegment()
        end_segment = segment_pair.getEndSegment()
        mc_particle = mc_segment_lookup.getMCParticle(start_segment)
        return (mc_particle and
                is_primary(mc_particle) and
                start_segment.size() > 3 and
                end_segment.size() > 3)

    def terminate(self):
        self.make_numpy_arrays()

        # Save everything to a ROOT file
        output_tfile = ROOT.TFile(self.output_file_name, 'recreate')

        # Show all parameters and the fit result in the plots
        # if viewed in the browser or the validation
        opt_fit = 0112
        ROOT.gStyle.SetOptFit(opt_fit)

        self.plot(output_tfile)

        output_tfile.Close()

    def make_numpy_arrays(self):
        # Make arrays from lists
        for key, value in vars(self).items():
            if isinstance(value, array.array):
                setattr(self, key + "_raw", value)
                setattr(self, key, np.array(value))

    def plot(self, output_tfile):
        start_superlayer_ids = self.start_superlayer_ids
        end_superlayer_ids = self.end_superlayer_ids

        curvature_estimates = self.curvature_estimates
        curvature_truths = self.curvature_truths
        curvature_variances = self.curvature_variances

        start_curvature_estimates = self.start_curvature_estimates
        end_curvature_estimates = self.end_curvature_estimates

        start_check = (start_curvature_estimates <= curvature_estimates) & (curvature_estimates <= end_curvature_estimates)
        end_check = (start_curvature_estimates >= curvature_estimates) & (curvature_estimates >= end_curvature_estimates)

        check = start_check | end_check
        if np.all(check):
            print "Average check succeeded."
        else:
            print "There are fused curvatures that are not between the initial curvatures."

        # To investigate if the absolute estimation worked we take the curvature modulus the sign of the truths
        absolute_curvature_truths = curvature_truths.copy()
        absolute_curvature_truths[curvature_truths < 0] = - absolute_curvature_truths[curvature_truths < 0]

        absolute_curvature_estimates = curvature_estimates.copy()
        absolute_curvature_estimates[curvature_truths < 0] = - absolute_curvature_estimates[curvature_truths < 0]

        absolute_curvature_residuals = absolute_curvature_estimates - absolute_curvature_truths

        tan_lambda_estimates = self.tan_lambda_estimates
        tan_lambda_truths = self.tan_lambda_truths
        tan_lambda_variances = self.tan_lambda_variances

        chi2s = self.chi2s
        ndfs = self.ndfs
        p_values = self.p_values

        # superlayer_id_selections = [ (7, 8, ) ]
        superlayer_id_selections = [(None, None, ), ] + [(i, i + 1, ) for i in range(8)]

        for start_superlayer_id, end_superlayer_id in superlayer_id_selections:
            if start_superlayer_id is None and end_superlayer_id is None:
                in_sl = slice(None)
                root_folder_name = "all"
                print "All pairs", "#", len(startISuperLayers)

            else:
                in_sl = ((start_superlayer_ids == start_superlayer_id) & (end_superlayer_ids == end_superlayer_id)) | \
                    ((end_superlayer_ids == start_superlayer_id) & (start_superlayer_ids == end_superlayer_id))
                root_folder_name = "sl_%s_%s" % (str(start_superlayer_id), str(end_superlayer_id))
                print "Pairs", start_superlayer_id, end_superlayer_id, "#", np.sum(in_sl)

            # Create a subfolder in the TFile for each super layer selection and switch to it
            # such that subsequent writes end up in the folder
            output_tdirectory = output_tfile.mkdir(root_folder_name, root_folder_name)
            output_tdirectory.cd()
            opt_fit = 0112
            ROOT.gStyle.SetOptFit(opt_fit)

            validation_plots = []

            get_logger().info("Start pull analysis - curvature.")
            curvature_pull_analysis = PullAnalysis("curvature", unit="1/cm")
            curvature_pull_analysis.analyse(curvature_truths[in_sl],
                                            curvature_estimates[in_sl],
                                            curvature_variances[in_sl])
            validation_plots.append(curvature_pull_analysis)

            absolute_curvature_pull_analysis = PullAnalysis("absolute curvature", unit="1/cm")
            absolute_curvature_pull_analysis.analyse(absolute_curvature_truths[in_sl],
                                                     absolute_curvature_estimates[in_sl],
                                                     curvature_variances[in_sl])
            validation_plots.append(absolute_curvature_pull_analysis)

            get_logger().info("Start pull analysis - tan lambda.")
            tan_lambda_pull_analysis = PullAnalysis("tan #lambda")
            tan_lambda_pull_analysis.analyse(tan_lambda_truths[in_sl],
                                             tan_lambda_estimates[in_sl],
                                             tan_lambda_variances[in_sl])
            validation_plots.append(tan_lambda_pull_analysis)

            get_logger().info("Finished analysis.")

            # NDF distribution
            ndfs_hist = ValidationPlot("ndfs")
            ndfs_hist.hist(ndfs[in_sl])
            ndfs_hist.title = "Distribution of ndf of the segment pair fits"
            ndfs_hist.xlabel = "ndf"
            ndfs_hist.contact = CONTACT
            ndfs_hist.check = "The width of the distribution should be reasonably small. Average should be two superlayers worth of hits."
            ndfs_hist.description = "NDF distribution of the three dimensional fits."
            validation_plots.append(ndfs_hist)

            # Chi2 distribution
            chi2s_hist = ValidationPlot("chi2s")
            chi2s_hist.hist(chi2s[in_sl], outlier_z_score=5.0)
            chi2s_hist.title = "Distribution of #chi^2 of the segment pair fits"
            chi2s_hist.xlabel = "#chi^2"
            chi2s_hist.contact = CONTACT
            chi2s_hist.check = "The width of the distribution should be reasonably small."
            chi2s_hist.description = "Chi square distribution of the three dimensional fits."
            validation_plots.append(chi2s_hist)

            # P value distribution
            p_values_hist = ValidationPlot("p_values")
            p_values_hist.hist(p_values[in_sl])
            p_values_hist.title = "Distribution of p-values of the segment pair fits"
            p_values_hist.xlabel = "p-value"
            p_values_hist.contact = CONTACT
            p_values_hist.check = "The distribution should be aproximatly flat."
            p_values_hist.description = "P-value composed from the chi square and the ndf of the segment pair fits."
            validation_plots.append(p_values_hist)

            # P value versus tan lambda profile
            p_values_by_tan_lambda_truths_profile = ValidationPlot("p_values_versus_tan_lambda_truths")
            p_values_by_tan_lambda_truths_profile.profile(tan_lambda_truths[in_sl], p_values[in_sl])
            p_values_by_tan_lambda_truths_profile.xlabel = "true tan #lambda"
            p_values_by_tan_lambda_truths_profile.ylabel = "p-value"
            p_values_by_tan_lambda_truths_profile.title = "Fit p-value versus tan #lambda truths"
            p_values_by_tan_lambda_truths_profile.contact = CONTACT
            p_values_by_tan_lambda_truths_profile.check = "The residuals should be essentially the same over the whole range of the tan lambda spectrum"
            p_values_by_tan_lambda_truths_profile.description = "Investigating the reconstruction quality for different tan lambda regions of the CDC."
            p_values_by_tan_lambda_truths_profile.fit_line()
            validation_plots.append(p_values_by_tan_lambda_truths_profile)

            for plot in validation_plots:
                plot.write(output_tdirectory)


def main():
    run = SegmentPairFitValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
