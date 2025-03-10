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
  <output>FullInvertedTrackingValidationBkg.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs with background.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
from tracking import add_tracking_reconstruction
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'FullInvertedTrackingValidationBkg.root'
N_EVENTS = 1000
ACTIVE = True


class InvertedFullBkg(TrackingValidationRun):
    """Validate the full track-finding chain, including background overlay"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Y(4S) events with background overlay
    root_input_file = '../EvtGenSim.root'

    def finder_module(self, path):
        """Add the inverted track finding to the basf2 path"""
        add_tracking_reconstruction(path, inverted_tracking=True)

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
    #: tracks will be already fitted by
    #: add_tracking_reconstruction finder module set above
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: do not create expert-level output histograms in dedicated folder but in the main folder
    use_expert_folder = False
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE
    #: define empty list of non expert parameters so that no shifter plots are created (to revert just remove following line)
    non_expert_parameters = []


def main():
    basf2.set_random_seed(1337)
    validation_run = InvertedFullBkg()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
