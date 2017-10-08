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

VALIDATION_OUTPUT_FILE = 'FullTrackingValidationBkg_VXDTF2.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking
import validationtools

try:
    bg = validationtools.get_background_files()
except:
    print("No background files available. Please set $BELLE2_BACKGROUND_DIR to a proper value")
    bg = []

from tracking.validation.run import TrackingValidationRun


class FullBkg(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: Use back ground in the simulation (-so)
    bkg_files = bg
    root_input_file = '../EvtGenSim_VXDTF2.root'
    finder_module = staticmethod(lambda path: tracking.add_tracking_reconstruction(path, use_vxdtf2=True))
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
