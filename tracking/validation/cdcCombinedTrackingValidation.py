#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CDCCombinedTrackingValidation.root</output>
  <description>This script validates the combined CDC tracking chain with a legendre step first
  and a cellular automaton step second in Y(4S) runs.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
from tracking.path_utils import add_cdc_track_finding
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'CDCCombinedTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = False


class CDCCombined(TrackingValidationRun):
    """Validate the combined CDC track-finding chain include Legendre step"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSimNoBkg.root'
    #: contact person
    contact = 'software-tracking@belle2.org - why are we not founding this?'

    def finder_module(self, path):
        """Add the CDC track-finding module to the basf2 path"""
        add_cdc_track_finding(path, with_cdc_cellular_automaton=True)
        # adjust_module(path, "TFCDC_WireHitPreparer",
        #               UseNLoops=1)

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
        "UseNLoops": 1,
        "UseOnlyBeforeTOP": True,
        'MinCDCAxialHits': 8,
        'MinCDCStereoHits': 6,
        "AllowFirstCDCSuperLayerOnly": True,
        "MergeDecayInFlight": True,
        'EnergyCut': 0,
    }
    #: Include pulls in the validation output
    pulls = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    basf2.set_random_seed(1337)
    validation_run = CDCCombined()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
