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


class Full(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../MuonGenSimNoBkg.root'
    finder_module = staticmethod(tracking.add_tracking_reconstruction)
    tracking_coverage = {
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
    }
    fit_geometry = None
    # tracks will be already fitted by
    # add_tracking_reconstruction finder module set above
    fit_tracks = False
    use_expert_folder = False
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
