#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Thomas.Hauth@kit.edu</contact>
  <input>EvtGenSim.root</input>
  <output>FullTrackingValidationBkg.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs with background.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'FullTrackingValidationBkg.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun


class FullBkg(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../EvtGenSim.root'
    finder_module = staticmethod(tracking.add_tracking_reconstruction)
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseOnlyBeforeTOP': True,
        'UseNLoops': 1
    }
    # tracks will be already fitted by
    # add_tracking_reconstruction finder module set above
    fit_tracks = False
    # But we need to tell the validation module to use the fit information
    use_fit_information = True
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = FullBkg()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
