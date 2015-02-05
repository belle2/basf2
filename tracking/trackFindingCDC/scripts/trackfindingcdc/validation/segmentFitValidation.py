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

from tracking.run.event_generation import ReadOrGenerateEventsRun

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot
from tracking.validation.pull import PullAnalysis

from tracking.modules import BrowseFileOnTerminateModule

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentFitValidationRun(ReadOrGenerateEventsRun):
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


class SegmentFitValidationModule(basf2.Module):

    """Module to collect information about the generated segments and compose validation plots on terminate."""

    def __init__(self, output_file_name):
        self.output_file_name = output_file_name
        super(SegmentFitValidationModule, self).__init__()
        self.mc_segment_lookup = None

        self.curvature_truths = array.array("f")
        self.curvature_estimates = array.array("f")
        self.curvature_variances = array.array("f")

        self.chi2s = array.array("f")
        self.ndfs = array.array("f")
        self.p_values = array.array("f")
        self.sizes = array.array("f")

        self.tan_lambda_truths = array.array("f")
        self.superlayer_ids = array.array("f")

    def initialize(self):
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()

    def event(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()
        self.gather_segments()

    def gather_segments(self):
        mc_segment_lookup = self.mc_segment_lookup

        stored_segments = Belle2.PyStoreObj('CDCRecoSegment2DVector')
        if not stored_segments:
            raise KeyError("CDCRecoSegment2DVector has not been created on the DataStore.")

        wrapped_segments = stored_segments.obj()
        segments = wrapped_segments.get()

        curvature_truths = self.curvature_truths
        curvature_estimates = self.curvature_estimates
        curvature_variances = self.curvature_variances

        chi2s = self.chi2s
        ndfs = self.ndfs
        p_values = self.p_values
        sizes = self.sizes

        tan_lambda_truths = self.tan_lambda_truths
        superlayer_ids = self.superlayer_ids

        for segment in segments:
            mc_particle = mc_segment_lookup.getMCParticle(segment)

            # Filter
            if not self.pick_segment(segment):
                continue

            # Retrive values
            fit3d_truth = mc_segment_lookup.getTrajectory3D(segment)
            fit2d_truth = fit3d_truth.getTrajectory2D()

            tan_lambda_truth = fit3d_truth.getTanLambda()
            curvature_truth = fit2d_truth.getCurvature()

            fit2d = segment.getTrajectory2D()

            chi2 = fit2d.getChi2()
            ndf = fit2d.getNDF()
            p_value = prob(chi2, ndf)
            size = segment.size()

            curvature_estimate = fit2d.getCurvature()
            iCurv = 0
            curvature_variance = fit2d.getLocalVariance(iCurv)

            superlayer_id = segment.getISuperLayer()

            # Append values to the arrays
            curvature_truths.append(curvature_truth)
            curvature_estimates.append(curvature_estimate)
            curvature_variances.append(curvature_variance)

            chi2s.append(chi2)
            ndfs.append(ndf)
            p_values.append(p_value)
            sizes.append(size)

            tan_lambda_truths.append(tan_lambda_truth)
            superlayer_ids.append(superlayer_id)

    def pick_segment(self, segment):
        mc_segment_lookup = self.mc_segment_lookup
        mc_particle = mc_segment_lookup.getMCParticle(segment)

        # Check that mc_particle is not a nullptr
        return mc_particle and is_primary(mc_particle) and segment.size() > 3

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
        curvature_truths = self.curvature_truths
        curvature_estimates = self.curvature_estimates
        curvature_variances = self.curvature_variances
        curvature_residuals = curvature_estimates - curvature_truths

        # To investigate if the absolute estimation worked we take the curvature modulus the sign of the truths
        absolute_curvature_truths = curvature_truths.copy()
        absolute_curvature_truths[curvature_truths < 0] = - absolute_curvature_truths[curvature_truths < 0]

        absolute_curvature_estimates = curvature_estimates.copy()
        absolute_curvature_estimates[curvature_truths < 0] = - absolute_curvature_estimates[curvature_truths < 0]

        absolute_curvature_residuals = absolute_curvature_estimates - absolute_curvature_truths

        chi2s = self.chi2s
        ndfs = self.ndfs
        p_values = self.p_values
        sizes = self.sizes

        superlayer_ids = self.superlayer_ids
        tan_lambda_truths = self.tan_lambda_truths

        info = get_logger().info
        info("Start plotting")

        # Draw a histogram of the result
        info("Minimum of curvature : %s", np.nanmin(curvature_estimates))
        info("Maximum of curvature : %s", np.nanmax(curvature_estimates))
        info("Minimum of variance : %s", np.nanmin(curvature_variances))
        info("Maximum of variance : %s", np.nanmax(curvature_variances))

        superlayer_id_selection = [
            None,  # all
            "axial",
            0, 2, 4, 6, 8,  # Each axials
            1, 3, 5, 7,  # Each stereo
        ]

        for superlayer_id in superlayer_id_selection:
            # Validation plots that will be written to file
            root_folder_name = ""

            if superlayer_id is None:
                in_sl = slice(None)
                root_folder_name = "all"
                info("#Segments in all superlayers : %s", len(superlayer_ids))

            elif superlayer_id == "axial":
                in_sl = (superlayer_ids % 2) == 0
                root_folder_name = "axial"
                info("#Segments in axial superlayers : %s", np.sum(in_sl))

            else:
                in_sl = superlayer_ids == superlayer_id
                root_folder_name = "sl_%s" % str(superlayer_id)
                info("#Segments in superlayer %s : %s", superlayer_id, np.sum(in_sl))

            # Create a subfolder in the TFile for each super layer selection and switch to it
            # such that subsequent writes end up in the folder
            output_tdirectory = output_tfile.mkdir(root_folder_name, root_folder_name)
            output_tdirectory.cd()
            opt_fit = 0112
            ROOT.gStyle.SetOptFit(opt_fit)

            validation_plots = []
            curvature_pull_analysis = PullAnalysis("curvature", unit="1/cm", outlier_z_score=3.0)
            curvature_pull_analysis.analyse(curvature_truths[in_sl],
                                            curvature_estimates[in_sl],
                                            curvature_variances[in_sl])

            validation_plots.append(curvature_pull_analysis)

            absolute_curvature_pull_analysis = PullAnalysis("absolute curvature", unit="1/cm", outlier_z_score=3.0)
            absolute_curvature_pull_analysis.analyse(absolute_curvature_truths[in_sl],
                                                     absolute_curvature_estimates[in_sl],
                                                     curvature_variances[in_sl])
            validation_plots.append(absolute_curvature_pull_analysis)

            # NDF distribution
            ndfs_hist = ValidationPlot("ndfs")
            ndfs_hist.hist(ndfs[in_sl])
            ndfs_hist.title = "Distribution of ndf of the segment fits"
            ndfs_hist.xlabel = "ndf"
            ndfs_hist.contact = CONTACT
            ndfs_hist.check = "The width of the distribution should be reasonably small. Average should be one superlayer worth of hits."
            ndfs_hist.description = "NDF distribution of the two dimensional fits."
            validation_plots.append(ndfs_hist)

            # Chi2 distribution
            chi2s_hist = ValidationPlot("chi2s")
            chi2s_hist.hist(chi2s[in_sl], outlier_z_score=5.0)
            chi2s_hist.title = "Distribution of #chi^2 of the segment fits"
            chi2s_hist.xlabel = "#chi^2"
            chi2s_hist.contact = CONTACT
            chi2s_hist.check = "The width of the distribution should be reasonably small."
            chi2s_hist.description = "Chi square distribution of the two dimensional fits."
            validation_plots.append(chi2s_hist)

            # P value distribution
            p_values_hist = ValidationPlot("p_values")
            p_values_hist.hist(p_values[in_sl])
            p_values_hist.title = "Distribution of p-values of the segment fits"
            p_values_hist.xlabel = "p-value"
            p_values_hist.contact = CONTACT
            p_values_hist.check = "The distribution should be aproximatly flat."
            p_values_hist.description = "P-value composed from the chi square and the ndf of the segment fits."
            validation_plots.append(p_values_hist)

            # Distribution of the true tan lambdas
            tan_lambda_truths_hist = ValidationPlot("tan_lambda_truths")
            tan_lambda_truths_hist.hist(tan_lambda_truths[in_sl])
            tan_lambda_truths_hist.title = "Distribution of true tan #lambda of the segments"
            tan_lambda_truths_hist.xlabel = "tan #lambda"
            tan_lambda_truths_hist.contact = CONTACT
            tan_lambda_truths_hist.check = "This plot is just for information purposes."
            tan_lambda_truths_hist.description = "Distribution of the true tan #lambda to show how the segments are distributed in the CDC."
            validation_plots.append(tan_lambda_truths_hist)

            # Curvature residual versus tan lambda profile
            curvature_residuals_by_tan_lambda_truths_profile = ValidationPlot("curvature_residuals_versus_tan_lambda_truths")
            curvature_residuals_by_tan_lambda_truths_profile.profile(tan_lambda_truths[in_sl], curvature_residuals[in_sl], outlier_z_score=5.0)
            curvature_residuals_by_tan_lambda_truths_profile.xlabel = "true tan #lambda"
            curvature_residuals_by_tan_lambda_truths_profile.ylabel = "curvature residual (1/cm)"
            curvature_residuals_by_tan_lambda_truths_profile.title = "Curvature residuals versus tan lambda truths"
            curvature_residuals_by_tan_lambda_truths_profile.contact = CONTACT
            curvature_residuals_by_tan_lambda_truths_profile.check = "The residuals should be essentially the same over the whole range of the tan lambda spectrum"
            curvature_residuals_by_tan_lambda_truths_profile.description = "Investigating the reconstruction quality for different tan lambda regions of the CDC."
            curvature_residuals_by_tan_lambda_truths_profile.fit_line()
            validation_plots.append(curvature_residuals_by_tan_lambda_truths_profile)

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
    run = SegmentFitValidationRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
