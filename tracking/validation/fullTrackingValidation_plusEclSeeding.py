#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>simon.kurz@desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>toCDCfromEclCKFTrackingValidation.root</output>
  <description>This module validates the full track reconstruction including the ECL seeding.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'fullTrackingValidation_plusECL.root'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

from basf2 import *

from tracking.path_utils import *
from tracking import *
import reconstruction

import logging
import tracking

from tracking.validation.run import TrackingValidationRun


class fullTrackingValidation_plusECL(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        components = ["SVD", "CDC", "ECL"]
        reconstruction.add_pretracking_reconstruction(path, components=components)

        tracking.add_tracking_reconstruction(path, components=components,
                                             use_svd_to_cdc_ckf=True, use_ecl_to_cdc_ckf=True)

    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseNLoops': 1
    }

    # Already fitted in the finder_module
    fit_tracks = False
    use_fit_information = True
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE
    extended = True


def main():
    validation_run = fullTrackingValidation_plusECL()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    if ACTIVE:
        main()
