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
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        path.add_module('WireHitPreparer')

        use_legendre_finder = True
        if use_legendre_finder:
            path.add_module('TFCDC_AxialTrackFinderLegendre')
        else:
            path.add_module('TFCDC_AxialTrackFinderHough')

        path.add_module('TrackExporter')

    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': True,
        "UseReassignedHits": True,
        'UseOnlyBeforeTOP': True,
        'UseNLoops': 1,
        'MinCDCAxialHits': 8,
        'EnergyCut': 0,
    }

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
