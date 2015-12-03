#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging

import basf2

from tracking.run.tracked_event_generation import ReadOrGenerateTrackedEventsRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin
from tracking.modules import StandardTrackingReconstructionModule
import tracking.modules
from tracking.validation.hit_module import ExpertTrackingValidationModule
import tracking.utilities as utilities

TRACKING_MAILING_LIST = 'tracking@belle2.kek.jp'


def get_logger():
    return logging.getLogger(__name__)


class TrackingValidationRun(BrowseTFileOnTerminateRunMixin, ReadOrGenerateTrackedEventsRun):
    expert_level = None
    contact = TRACKING_MAILING_LIST
    output_file_name = 'TrackingValidation.root'  # Specification for BrowseTFileOnTerminateRunMixin
    pulls = True
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
            output_file_name=self.output_file_name,
            exclude_profile_mc_parameter=self.exclude_profile_mc_parameter,
            exclude_profile_pr_parameter=self.exclude_profile_pr_parameter
        )

        trackingValidationModule.trackCandidatesColumnName = \
            self.trackCandidatesColumnName

        main_path.add_module(trackingValidationModule)

    def create_argument_parser(self, **kwds):
        argument_parser = super(TrackingValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(TrackingValidationRun, self).create_path()

        # add the validation module to the path, but only if requested
        if not self.simulate_only:
            self.preparePathValidation(main_path)
        return main_path


def main():
    trackingValiddationRun = TrackingValidationRun()
    trackingValiddationRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
