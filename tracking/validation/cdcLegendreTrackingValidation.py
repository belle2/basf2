#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Thomas.Hauth@kit.edu</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CDCLegendreTrackingValidation.root</output>
  <description>This module validates that legendre track finding is capable of reconstructing tracks in Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CDCLegendreTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

from tracking.validation.run import TrackingValidationRun


class CDCLegendre(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        path.add_module('WireHitPreparer')

        use_legendre_finder = True
        if use_legendre_finder:
            path.add_module('TrackFinderCDCLegendreTracking')
        else:
            path.add_module('AxialTrackCreatorHitLegendre')

        path.add_module('TrackExporter')

    tracking_coverage = {'UsePXDHits': False,
                         'UseSVDHits': False,
                         'UseCDCHits': True,
                         'UseOnlyAxialCDCHits': True}

    fit_geometry = None
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE

    # disabled plotting of tan lambda and theta for this validation because
    # the seed's theta is 90* for all and therefore this profile plot is useless
    exclude_profile_pr_parameter = ["Seed tan #lambda", "Seed #theta"]


def main():
    validation_run = CDCLegendre()
    validation_run.configure_and_execute_from_commandline()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
