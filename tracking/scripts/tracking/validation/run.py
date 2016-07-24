#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging

import basf2

from tracking.run.tracked_event_generation import ReadOrGenerateTrackedEventsRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin
from tracking.validation.hit_module import ExpertTrackingValidationModule
import tracking.utilities as utilities

TRACKING_MAILING_LIST = 'tracking@belle2.kek.jp'


def get_logger():
    return logging.getLogger(__name__)


class TrackingValidationRun(BrowseTFileOnTerminateRunMixin, ReadOrGenerateTrackedEventsRun):
    expert_level = None
    contact = TRACKING_MAILING_LIST
    output_file_name = 'TrackingValidation.root'  # Specification for BrowseTFileOnTerminateRunMixin
    root_output_file = None  # If events file should be written
    pulls = True
    resolution = False
    use_expert_folder = True

    #: not fit by default
    #: can be overridden by derived validation class
    fit_geometry = None
    exclude_profile_mc_parameter = []
    exclude_profile_pr_parameter = []

    # The default way to add the validation module to the path
    # Derived classes can overload this method modify the validation module
    # or add more than one validation steps
    def preparePathValidation(self, main_path):
        # Validation module generating plots
        fit = bool(self.fit_geometry)
        trackingValidationModule = ExpertTrackingValidationModule(
            self.name,
            contact=self.contact,
            fit=fit,
            pulls=self.pulls,
            resolution=self.resolution,
            output_file_name=self.output_file_name,
            use_expert_folder=self.use_expert_folder,
            exclude_profile_mc_parameter=self.exclude_profile_mc_parameter,
            exclude_profile_pr_parameter=self.exclude_profile_pr_parameter
        )

        trackingValidationModule.trackCandidatesColumnName = \
            self.trackCandidatesColumnName

        # from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule
        # trackingValidationModule = CombinedTrackingValidationModule(self.name,
        #                                                             contact=self.contact,
        #                                                             output_file_name=self.output_file_name,
        #                                                             reco_tracks_name=self.trackCandidatesColumnName,
        #                                                             mc_reco_tracks_name='MCRecoTracks')

        main_path.add_module(trackingValidationModule)

    def create_argument_parser(self, **kwds):
        argument_parser = super(TrackingValidationRun, self).create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-o',
            '--output',
            dest='root_output_file',
            default=self.root_output_file,
            help='Output file to which the simulated events shall be written.')

        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(TrackingValidationRun, self).create_path()

        # add the validation module to the path, but only if requested
        if not self.simulate_only:
            self.preparePathValidation(main_path)

        # Add the (optional) output module
        if self.root_output_file is not None:
            root_output_module = basf2.register_module('RootOutput')
            root_output_params = {'outputFileName': self.root_output_file}
            root_output_module.param(root_output_params)
            main_path.add_module(root_output_module)

        return main_path


def main():
    trackingValiddationRun = TrackingValidationRun()
    trackingValiddationRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
