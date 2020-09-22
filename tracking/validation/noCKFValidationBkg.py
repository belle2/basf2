#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Nils.Braun@kit.edu</contact>
  <input>EvtGenSim.root</input>
  <output>NoCKFValidationBkg.root</output>
  <description>This script validates the tracking chain without the CKF in Y(4S) runs with background.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import tracking
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'NoCKFValidationBkg.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)


def setupFinderModule(path):
    tracking.add_hit_preparation_modules(path, components=["SVD", "PXD"])
    tracking.add_track_finding(path, svd_ckf_mode="VXDTF2_alone")


class CKFBkg(TrackingValidationRun):
    """Validate the track-finding chain, excluding the Kalman fitter/filter, with Y(4S) events and background overlay"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: Y(4S) events with background overlay
    root_input_file = '../EvtGenSim.root'
    #: insert the track-finding components into the basf2 path
    finder_module = staticmethod(setupFinderModule)
    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseOnlyBeforeTOP': True,
        'UseNLoops': 1
    }
    #: fit the tracks in each event
    fit_tracks = True
    #: tell the validation module to use the fit information
    use_fit_information = True
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CKFBkg()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
