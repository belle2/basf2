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
  <input>EvtGenSim.root</input>
  <output>CDCFullTrackingValidationBkg.root</output>
  <description>This script validates the full CDC tracking chain with a legendre step first
  and a cellular automaton step second in Y(4S) runs.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import tracking
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'CDCFullTrackingValidationBkg.root'
N_EVENTS = 1000
ACTIVE = True


class CDCFull(TrackingValidationRun):
    """Validate the full CDC track-finding chain"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSim.root'

    def finder_module(self, path):
        """Add the CDC track-finding module to the basf2 path"""
        tracking.path_utils.add_cdc_track_finding(path)
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
        'EnergyCut': 0,
    }
    #: Include pulls in the validation output
    pulls = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    basf2.set_random_seed(1337)
    validation_run = CDCFull()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
