#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CDCAutomatonTrackingValidation.root</output>
  <description>
  This module validates that cdc cellular automaton track finding
  is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'CDCAutomatonTrackingValidation.root'
CONTACT = 'software-tracking@belle2.org'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)


class CDCAutomaton(TrackingValidationRun):
    """Validate the CDC TrackFinderAutomaton"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSimNoBkg.root'

    def finder_module(self, path):
        """Add the CDC TrackFinderAutomaton to the basf2 path"""
        path.add_module('TFCDC_TrackFinderAutomaton',
                        # UseNLoops = 1,
                        )

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
        "UseNLoops": 1.0,
        "UseOnlyBeforeTOP": True,
        'MinCDCAxialHits': 8,
        'MinCDCStereoHits': 6,
        "AllowFirstCDCSuperLayerOnly": True,
        'EnergyCut': 0,
    }
    #: Include pulls in the validation output
    pulls = True
    #: name of the contact person
    contact = CONTACT
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCAutomaton()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
