#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

import logging
import argparse

from tracking.run.tracked_event_generation import ReadOrGenerateTrackedEventsRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin
from tracking.validation.hit_module import ExpertTrackingValidationModule

#: Current location of the tracking mailing list
TRACKING_MAILING_LIST = 'software-tracking@belle2.org'


class TrackingValidationRun(BrowseTFileOnTerminateRunMixin, ReadOrGenerateTrackedEventsRun):
    """Run setup to compose a path to validate the the tracking procedures from pre-simulated events
    or from events simulated on the fly. Considering parameters from the commandline."""

    #: Default contact email address for the validation results
    contact = TRACKING_MAILING_LIST

    #: Name of the output file for the validation results
    output_file_name = 'TrackingValidation.root'  # Also specification for BrowseTFileOnTerminateRunMixin

    #: Optional file name as a destination of all event data which is discarded otherwise
    root_output_file = None

    #: Include the pull plots of the fit parameters in the validation
    pulls = True

    #: Include the residual plots of the fit parameters in the validation
    resolution = False

    #: Use the "expert" folder in the validation file as the destination of the pull and residual plots
    use_expert_folder = True

    #: not add the geometry modules by default
    #: can be overridden by derived validation class
    fit_geometry = None

    #: Exclude some of the perigee parameters from the mc side plots
    exclude_profile_mc_parameter = []

    #: Exclude some of the perigee parameters from the pr side plots
    exclude_profile_pr_parameter = []

    #: Do not fit the tracks but access the fit information for pulls etc.
    use_fit_information = False

    def preparePathValidation(self, path):
        """The default way to add the validation module to the path.

        Derived classes can overload this method modify the validation module
        or add more than one validation steps.
        """

        # Validation module generating plots
        trackingValidationModule = ExpertTrackingValidationModule(
            self.name,
            contact=self.contact,
            fit=self.use_fit_information or self.fit_tracks,
            pulls=self.pulls,
            resolution=self.resolution,
            output_file_name=self.output_file_name,
            use_expert_folder=self.use_expert_folder,
            exclude_profile_mc_parameter=self.exclude_profile_mc_parameter,
            exclude_profile_pr_parameter=self.exclude_profile_pr_parameter
        )

        trackingValidationModule.trackCandidatesColumnName = "RecoTracks"
        path.add_module(trackingValidationModule)

    def create_argument_parser(self, **kwds):
        """Create command line argument parser"""
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-o',
            '--output',
            dest='root_output_file',
            default=argparse.SUPPRESS,
            help='Output file to which the simulated events shall be written.'
        )

        return argument_parser

    def create_path(self):
        """Create path from parameters"""
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        # add the validation module to the path, but only if requested
        if not self.simulate_only:
            self.preparePathValidation(path)

        # Add the (optional) output module
        if self.root_output_file is not None:
            path.add_module('RootOutput',
                            outputFileName=self.root_output_file)

        return path


def main():
    trackingValiddationRun = TrackingValidationRun()
    trackingValiddationRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
