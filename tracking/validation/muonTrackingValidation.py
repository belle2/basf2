#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Thomas.Hauth@kit.edu</contact>
  <input>MuonGenSimNoBkg.root</input>
  <output>MuonTrackingValidation.root</output>
  <description>This script validates the full track finding chain in easy 1 muon events.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'MuonTrackingValidation.root'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun


class Muon(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'gun'
    root_input_file = '../MuonGenSimNoBkg.root'
    finder_module = staticmethod(tracking.add_tracking_reconstruction)
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
    }
    # tracks will be already fitted by
    # add_tracking_reconstruction finder module set above
    fit_tracks = False
    # But we need to tell the validation module to use the fit information
    use_fit_information = True
    use_expert_folder = False
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Muon()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
