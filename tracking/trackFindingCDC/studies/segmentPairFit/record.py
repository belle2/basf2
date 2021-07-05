#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

from ROOT import Belle2  # make Belle2 namespace available

import os
import sys

from tracking.validation.utilities import is_primary

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
from tracking.harvest.run import HarvestingRun

import trackfindingcdc.harvest.cdc_peelers as cdc_peelers

import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentPairFitValidationRun(HarvestingRun):
    """Harvester to generate, postprocess and inspect MC events for track-segment-pair fit validation"""
    #: number of events to generate
    n_events = 10000
    #: use the low-momentum particle gun event generator
    generator_module = "simple_gun"  # Rather high momentum tracks should make the tracks rather straight.

    #: do not generate new events
    monte_carlo = "no"
    #: ordering of the segments in the pair
    segment_orientation = "outwards"

    #: use the Kalmanesk fuse of the two trajectory fits
    fit_method_name = "fuse-sz"

    @property
    def output_file_name(self):
        """Get the output ROOT filename"""
        file_name = self.fit_method_name
        file_name += "-mc-" + self.monte_carlo

        if self.root_input_file:
            file_name += "-" + os.path.split(self.root_input_file)[1]
        else:
            file_name += ".root"

        return file_name

    def harvesting_module(self, path=None):
        """Harvest and post-process the MC events"""
        harvesting_module = SegmentPairFitValidationModule(self.output_file_name)
        if path:
            path.add_module(harvesting_module)
        return harvesting_module

    def create_argument_parser(self, **kwds):
        """Convert command-line arguments to basf2 argument list"""
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
            choices=["zreco", "fuse-pre", "fuse-sz", "fuse-sz-re"],
            default=self.fit_method_name,
            dest="fit_method_name",
            help=("Choose which fit positional information of the segment should be used. \n"
                  "* 'zreco' means the z coordinate is reconstructed and a linear sz fit is made. "
                  "No covariance between the circle and the linear sz part can be made.\n"
                  "* 'fuse-sz' means the Kalmanesk fuse of the two trajectory fits.\n"
                  "* 'fuse-sz-re' means the Kalmanesk fuse of the two trajectory fits but reestimate the drift length."
                  )
        )

        return argument_parser

    def get_fit_method(self):
        """Determine which track-segment-pair fitter to use"""
        fit_method_name = self.fit_method_name

        if fit_method_name == 'zreco':
            sz_fitter = Belle2.TrackFindingCDC.CDCSZFitter.getFitter()

            def z_reconstruction_fit(pair):
                return sz_fitter.update(pair)
            return z_reconstruction_fit

        elif fit_method_name.startswith('fuse-pre'):
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion
            CDCSegmentPair = Belle2.TrackFindingCDC.CDCSegmentPair
            fusionFit = CDCAxialStereoFusion()

            def sz_segment_pair_preliminary_fit(pair):
                fusionFit.fusePreliminary(pair)
            return sz_segment_pair_preliminary_fit

        elif fit_method_name.startswith('fuse-sz'):
            CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion
            CDCSegmentPair = Belle2.TrackFindingCDC.CDCSegmentPair
            reestimateDriftLength = fit_method_name.endswith("re")
            fusionFit = CDCAxialStereoFusion(reestimateDriftLength)

            def sz_segment_pair_fusion_fit(pair):
                fusionFit.reconstructFuseTrajectories(pair)
                return

                trajectory3D = pair.getTrajectory3D()
                revFromSegment = pair.getFromSegment().reversed()
                revToSegment = pair.getToSegment().reversed()
                revPair = CDCSegmentPair(revToSegment, revFromSegment)

                CDCAxialStereoFusion.reconstructFuseTrajectories(revPair)
                revTrajectory3D = revPair.getTrajectory3D().reversed()

                # print("One origin x", trajectory3D.getLocalOrigin().x())
                # print("One origin y", trajectory3D.getLocalOrigin().y())
                # print("One origin z", trajectory3D.getLocalOrigin().z())

                # print("Rev origin x", revTrajectory3D.getLocalOrigin().x())
                # print("Rev origin y", revTrajectory3D.getLocalOrigin().y())
                # print("Rev origin z", revTrajectory3D.getLocalOrigin().z())

                print("One parameters", [trajectory3D.getLocalHelix().parameters()[i] for i in range(5)])
                print("Rev parameters", [revTrajectory3D.getLocalHelix().parameters()[i] for i in range(5)])

                print("One covariance")
                for j in range(5):
                    print([trajectory3D.getLocalHelix().helixCovariance()(i, j) for i in range(5)])

                print("Rev covariance")
                for j in range(5):
                    print([revTrajectory3D.getLocalHelix().helixCovariance()(i, j) for i in range(5)])

                # return revTrajectory3D
                # return trajectory3D

            return sz_segment_pair_fusion_fit

        else:
            raise ValueError("Unexpected fit_positions %s" % fit_method_name)

    def create_path(self):
        """
        Sets up a path that plays back pregenerated events or generates events
        based on the properties in the base class.
        """
        path = super().create_path()

        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards",
                        UseNLoops=0.5
                        )

        path.add_module("TFCDC_ClusterPreparer")

        #: Degree of refinement of the segment generation
        if self.monte_carlo == "no":
            # MC free - default
            path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                            SegmentOrientation="outwards"
                            )

            path.add_module("TFCDC_SegmentFitter",
                            inputSegments="CDCSegment2DVector",
                            updateDriftLength=True,
                            useAlphaInDriftLength=True,
                            )

        elif self.monte_carlo == "medium":
            # Medium MC - proper generation logic, but true facets and facet relations
            path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                            FacetFilter="truth",
                            FacetRelationFilter="truth",
                            SegmentOrientation="outwards"
                            )

            path.add_module("TFCDC_SegmentFitter",
                            inputSegments="CDCSegment2DVector",
                            updateDriftLength=True,
                            useAlphaInDriftLength=True,
                            )

        elif self.monte_carlo == "full":
            # Only true monte carlo segments
            # make the positions realistic but keep the true drift length
            path.add_module("TFCDC_SegmentCreatorMCTruth",
                            reconstructedDriftLength=False,
                            reconstructedPositions=True,
                            # segments="MCSegments"
                            )

            path.add_module("TFCDC_SegmentFitter",
                            inputSegments="CDCSegment2DVector",
                            updateDriftLength=False,
                            # useAlphaInDriftLength=True,
                            )

        else:
            raise ValueError("Invalid degree of Monte Carlo information")

        path.add_module("TFCDC_SegmentOrienter",
                        SegmentOrientation="outwards",
                        # SegmentOrientation="none",
                        inputSegments="CDCSegment2DVector",
                        segments="CDCSegment2DVectorOriented"
                        )

        path.add_module("TFCDC_TrackFinderSegmentPairAutomaton",
                        inputSegments="CDCSegment2DVectorOriented",
                        WriteSegmentPairs=True,
                        SegmentPairFilter="truth",
                        SegmentPairFilterParameters={"allowReverse": True},
                        SegmentPairRelationFilter="none"
                        )

        path.add_module(AxialStereoPairFitterModule(fit_method=self.get_fit_method()))
        return path


class SegmentPairFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segment pairs and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        """Constructor"""
        super(SegmentPairFitValidationModule, self).__init__(
            output_file_name=output_file_name,
            foreach="CDCSegmentPairVector"
        )

        #: by default, there is no method to find matching MC track segments
        self.mc_segment_lookup = None

    def initialize(self):
        """Receive signal at the start of event processing"""
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()
        super(SegmentPairFitValidationModule, self).initialize()

    def prepare(self):
        """Initialize the MC-hit lookup method"""
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, segment_pair):
        """Select segment pairs with 4 or more hits per segment and a matching primary MC particle"""
        mc_segment_lookup = self.mc_segment_lookup
        from_segment = segment_pair.getFromSegment()
        to_segment = segment_pair.getToSegment()
        mc_particle = mc_segment_lookup.getMCParticle(from_segment)
        return (mc_particle and
                is_primary(mc_particle) and
                from_segment.size() > 3 and
                to_segment.size() > 3)

    def peel(self, segment_pair):
        """Aggregate the track and MC information for track-segment analysis"""
        mc_segment_lookup = self.mc_segment_lookup

        # from_segment = segment_pair.getFromSegment()
        to_segment = segment_pair.getToSegment()

        # mc_particle = mc_segment_lookup.getMCParticle(from_segment)

        # Take the fit best at the middle of the segment pair
        fit3d_truth = mc_segment_lookup.getTrajectory3D(to_segment)

        fb_info = 1 if segment_pair.getAutomatonCell().getCellWeight() > 0 else -1
        truth_crops = dict(
            curvature_truth=fb_info * fit3d_truth.getCurvatureXY(),
            tan_lambda_truth=fb_info * fit3d_truth.getTanLambda(),
        )

        segment_pair_crops = cdc_peelers.peel_segment_pair(segment_pair)
        segment_pair_crops.update(truth_crops)
        return segment_pair_crops

    # Refiners to be executed at the end of the harvesting / termination of the module
    #: Save histograms in a sub folder
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree()

    #: Save curvature-pull auxiliary information in a sub folder
    save_curvature_pull_aux = refiners.save_pull_analysis(
        part_name="curvature",
        folder_name="aux",
        unit="1/cm",
        absolute=False,
        aux_names=["superlayer_id_pair", "tan_lambda_truth"],
        which_plots=["aux"],
        outlier_z_score=3.0)

    #: Save curvature-pull information in a sub folder
    save_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=False,
        groupby=[None, "superlayer_id_pair"],
        outlier_z_score=3.0)

    #: Save absolute curvature-pull information in a sub folder
    save_absolute_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        groupby=[None, "superlayer_id_pair"],
        outlier_z_score=3.0)

    #: Save tan(lambda)-fit pull information in a sub folder
    save_tan_lambda_pull = refiners.save_pull_analysis(
        part_name="tan_lambda",
        quantity_name="tan #lambda",
        absolute=False,
        groupby=[None, "superlayer_id_pair"],
        outlier_z_score=3.0)

    #: Save fit-quality histograms in a sub folder
    save_fit_quality_histograms = refiners.save_histograms(
        outlier_z_score=5.0,
        select={"ndf": "ndf",
                "chi2": "#chi2",
                "p_value": "p-value",
                },
        groupby=[None, "superlayer_id_pair"],
        description="Distribution of {part_name} in the segment fits",
    )

    #: Save tan(lambda)-fit-quality profile histograms in a sub folder
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


class AxialStereoPairFitterModule(basf2.Module):
    """Fit axial and stereo track segment pairs"""

    @staticmethod
    def default_fit_method(segmentPair):
        """Default method to fit the generated segment pairs."""

        CDCAxialStereoFusion = Belle2.TrackFindingCDC.CDCAxialStereoFusion
        CDCAxialStereoFusion.reconstructFuseTrajectories(segmentPair,
                                                         True)

    def __init__(self, fit_method=None):
        """Constructor"""

        #: fit_method : function
        #:    A function called on each stored segment pair as its only argument to update its fit inplace.
        #:    See default_fit_method for an example. Defaults to None meaning the default_fit_method is used
        #: Method used to fit the individual segment pairs
        self.fit_method = fit_method
        if not fit_method:
            self.fit_method = self.default_fit_method

        super(AxialStereoPairFitterModule, self).__init__()

    def event(self):
        """Called by basf2 for each event"""
        self.fitStoredPairs()

    def fitStoredPairs(self):
        """Fits all pairs in the StoreArray with designated fit method."""

        fit_method = self.fit_method

        stored_segment_pair_relations = Belle2.PyStoreObj("CDCSegmentPairVector")
        wrapped_segment_pair_relations = stored_segment_pair_relations.obj()
        segment_pair_relations = wrapped_segment_pair_relations.get()

        for segment_pair_relation in segment_pair_relations:
            fit_method(segment_pair_relation)


def main():
    run = SegmentPairFitValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
