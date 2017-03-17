#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <input>CosmicsSimNoBkg.root</input>
  <output>CosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

from tracking.validation.run import TrackingValidationRun


class Cosmics(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    root_input_file = '../CosmicsSimNoBkg.root'
    components = None
    finder_module = 'TrackFinderCDCCosmics'
    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
    }
    pulls = True
    contact = CONTACT
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Cosmics()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
