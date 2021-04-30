#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from ROOT import Belle2  # make Belle2 namespace available

import os
import sys

from tracking.validation.utilities import is_primary

import tracking.harvest.peelers as peelers
import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
from tracking.harvest.run import HarvestingRun

import trackfindingcdc.harvest.cdc_peelers as cdc_peelers


import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class SegmentFitValidationRun(HarvestingRun):
    """Harvester to read, postprocess and inspect MC events for track-segment fit validation"""

    #: number of events to generate
    n_events = 10000
    #: use the low-momentum particle gun event generator
    generator_module = "low_gun"  # Rather high momentum tracks should make the tracks rather straight.
    #: read generated/simulated/reconstructed events from this ROOT file
    root_input_file = "low_gun.root"

    #: do not generate new events
    monte_carlo = "no"
    #: use the Riemann fit instead of the Karimaki fit
    karimaki_fit = False
    #: do not estimate the flight time
    flight_time_estimation = "none"
    #: do not re-estimate the flight time
    flight_time_reestimation = False
    #: use the alpha parameter for drift length
    use_alpha_in_drift_length = True
    #: assume a very heavy particle for flight time calculation
    flight_time_mass_scale = float("nan")

    #: fit the reconstructed hit position for the segment fit
    fit_positions = "recoPos"
    #: use the drift-length variance in the segment fit
    fit_variance = "proper"

    @property
    def output_file_name(self):
        """Get the output ROOT filename"""
        file_name = "karimaki" if self.karimaki_fit else "riemann"
        file_name += "-mc-" + self.monte_carlo
        if self.flight_time_reestimation:
            file_name += "-re"
        if self.root_input_file:
            file_name += "-" + os.path.split(self.root_input_file)[1]
        else:
            file_name += ".root"

        return file_name

    def harvesting_module(self, path=None):
        """Harvest and post-process the MC events"""
        harvesting_module = SegmentFitValidationModule(self.output_file_name)
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
        """
        Sets up a path that plays back pregenerated events or generates events
        based on the properties in the base class.
        """
        path = super().create_path()

        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation=self.flight_time_estimation,
                        UseNLoops=1
                        )

        path.add_module("TFCDC_ClusterPreparer")

        #: Degree of refinement of the segment generation
        if self.monte_carlo == "no":
            # MC free - default
            path.add_module(
                "TFCDC_SegmentFinderFacetAutomaton",
                # investigate=[],
                SegmentOrientation="none",
            )

        elif self.monte_carlo == "medium":
            # Medium MC - proper generation logic, but true facets and facet relations
            path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                            FacetFilter="truth",
                            FacetRelationFilter="truth",
                            SegmentOrientation="none",
                            # SegmentOrientation="outwards"
                            )

        elif self.monte_carlo == "full":
            # Only true monte carlo segments, but make the positions realistic
            path.add_module("TFCDC_SegmentCreatorMCTruth",
                            reconstructedDriftLength=False,
                            reconstructedPositions=True)

        else:
            raise ValueError("Invalid degree of Monte Carlo information")

        path.add_module("TFCDC_SegmentFitter",
                        inputSegments="CDCSegment2DVector",
                        karimakiFit=self.karimaki_fit,
                        fitPos=self.fit_positions,
                        fitVariance=self.fit_variance,
                        updateDriftLength=self.flight_time_reestimation,
                        useAlphaInDriftLength=self.use_alpha_in_drift_length,
                        tofMassScale=self.flight_time_mass_scale)

        return path


class SegmentFitValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the generated segments and
    compose validation plots on terminate."""

    def __init__(self, output_file_name):
        """Constructor"""
        super().__init__(foreach='CDCSegment2DVector',
                         output_file_name=output_file_name)

        #: by default, there is no method to find matching MC track segments
        self.mc_segment_lookup = None

    def initialize(self):
        """Receive signal at the start of event processing"""
        super().initialize()

        #: Method to find matching MC track segments
        self.mc_segment_lookup = Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()
        #: Method to find matching MC hits
        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()

    def prepare(self):
        """Initialize the MC-hit lookup method"""
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, segment):
        """Select segments with 4 or more hits and a matching primary MC particle"""
        mc_segment_lookup = self.mc_segment_lookup
        mc_particle = mc_segment_lookup.getMCParticle(segment)

        # Check that mc_particle is not a nullptr
        return mc_particle and is_primary(mc_particle) and segment.size() > 3

    def peel(self, segment):
        """Aggregate the track and MC information for track-segment analysis"""

        mc_segment_lookup = self.mc_segment_lookup
        mc_hit_lookup = self.mc_hit_lookup

        segment_crops = cdc_peelers.peel_segment2d(segment)

        mc_particle = mc_segment_lookup.getMCParticle(segment)
        mc_particle_crops = peelers.peel_mc_particle(mc_particle)

        fit3d_truth = mc_segment_lookup.getTrajectory3D(segment)
        curvature_truth = fit3d_truth.getCurvatureXY()

        reconstructed_backward = mc_segment_lookup.isForwardOrBackwardToMCTrack(segment)
        is_fake = mc_segment_lookup.getMCTrackId(segment) < 0

        if reconstructed_backward != 1:
            curvature_truth = -curvature_truth

        truth_crops = dict(
            tan_lambda_truth=fit3d_truth.getTanLambda(),
            curvature_truth=curvature_truth,
            abs_curvature_truth=abs(curvature_truth),
            curvature_residual=segment_crops["curvature_estimate"] - curvature_truth,
            curvature_pull=(segment_crops["curvature_estimate"] - curvature_truth) / segment_crops["curvature_variance"],
            reconstructed_backward=reconstructed_backward,
            is_fake=is_fake
        )

        rl_sum = 0
        n_correct = 0
        n_total = segment.size()
        first_i_incorrect = float("nan")
        last_i_incorrect = float("nan")
        first_l_incorrect = 0
        last_l_incorrect = 0

        n_rl_switch = 0
        last_rl_info = 0
        for i, reco_hit2d in enumerate(segment):
            rl_info = reco_hit2d.getRLInfo()
            hit = reco_hit2d.getWireHit().getHit()
            true_rl_info = mc_hit_lookup.getRLInfo(hit)

            if rl_info != last_rl_info:
                n_rl_switch += 1
                last_rl_info = rl_info

            if true_rl_info == rl_info:
                n_correct += 1
            else:
                if first_i_incorrect != first_i_incorrect:
                    first_i_incorrect = i
                    first_l_incorrect = reco_hit2d.getRefDriftLength()
                last_i_incorrect = i
                last_l_incorrect = reco_hit2d.getRefDriftLength()
            if rl_info == 1:
                rl_sum += 1
            elif rl_info == -1 or rl_info == 65535:  # <- The root interface mistakes the signed enum value for an unsigned value
                rl_sum -= 1

        alias_score = segment.getAliasScore()

        rl_crops = dict(
            n_total=n_total,
            n_correct=n_correct,
            n_incorrect=n_total - n_correct,
            rl_purity=n_correct / n_total,
            first_incorrect_location=first_i_incorrect / (n_total - 1),
            first_l_incorrect=first_l_incorrect,
            last_incorrect_location=last_i_incorrect / (n_total - 1),
            last_l_incorrect=last_l_incorrect,
            n_rl_switch=n_rl_switch,
            n_rl_asymmetry=rl_sum / n_total,
            n_abs_rl_asymmetry=abs(rl_sum) / n_total,
            may_alias=alias_score == 0,
            alias_score=alias_score
        )

        segment_crops.update(truth_crops)
        segment_crops.update(mc_particle_crops)
        segment_crops.update(rl_crops)
        return segment_crops

    # Refiners to be executed at the end of the harvesting / termination of the module
    #: Save histograms in a sub folder
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True)
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree()

    #: Save curvature-pull information in a sub folder
    save_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=False,
        aux_names=["tan_lambda_truth", "abs_curvature_truth"],
        groupby=[
            "stereo_kind",
            "superlayer_id"
        ],
        outlier_z_score=4.0,
        title_postfix="")

    #: Save right-left curvature-pull information in a sub folder
    save_curvature_pull_rl_pure = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        filter_on="rl_purity",
        filter=lambda rl_purity: rl_purity > 0.5,
        aux_names=["tan_lambda_truth", "abs_curvature_truth"],
        groupby=[
            "stereo_kind",
            "superlayer_id"
        ],
        outlier_z_score=4.0,
        title_postfix="",
        folder_name="rl_pure/{groupby_addition}"
    )

    #: Save absolute curvature-pull information in a sub folder
    save_absolute_curvature_pull = refiners.save_pull_analysis(
        part_name="curvature",
        unit="1/cm",
        absolute=True,
        aux_names=["tan_lambda_truth", "abs_curvature_truth"],
        groupby=[
            "stereo_kind",
            "superlayer_id"
        ],
        outlier_z_score=4.0,
        title_postfix="")

    #: Save fit-quality histograms in a sub folder
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


def main():
    run = SegmentFitValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
