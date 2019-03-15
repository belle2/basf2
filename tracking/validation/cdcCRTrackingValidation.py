#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>CosmicsSimNoBkg.root</input>
  <output>CDCCRTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CDCCRTrackingValidation.root'
CONTACT = 'software-tracking@belle2.org'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

import tracking
from tracking.adjustments import adjust_module
from tracking.validation.run import TrackingValidationRun


class CDCCR(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    root_input_file = '../CosmicsSimNoBkg.root'
    components = None

    # data_taking_period has to be "gcr2017" (or one of the other cosmic runs) else the full cosmics finding (CDC+SVD)
    # is added, or no cdc constants are defined
    def finder_module(self, path):
        tracking.add_cr_track_finding(path, data_taking_period="gcr2017")

    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
    }

    use_fit_information = True
    fit_tracks = True
    pulls = True
    resolution = True
    contact = CONTACT
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCCR()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
