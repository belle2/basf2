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


class Full(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../EvtGenSim.root'
    finder_module = staticmethod(tracking.add_track_finding)
    tracking_coverage = {
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
    }
    fit_geometry = "default"
    fit_tracks = True
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
