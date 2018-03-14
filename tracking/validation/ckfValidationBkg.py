#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Nils.Braun@kit.edu</contact>
  <input>EvtGenSim.root</input>
  <output>CKFValidationBkg.root</output>
  <description>This script validates the CKF tracking chain in Y(4S) runs with background.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CKFValidationBkg.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun


def setupFinderModule(path):
    tracking.add_hit_preparation_modules(path, components=["SVD", "PXD"])
    tracking.add_ckf_based_track_finding(path)


class CKFBkg(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '../EvtGenSim.root'
    finder_module = staticmethod(setupFinderModule)
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseOnlyBeforeTOP': True,
        'UseNLoops': 1
    }
    fit_tracks = True
    # But we need to tell the validation module to use the fit information
    use_fit_information = True
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CKFBkg()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
