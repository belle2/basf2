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
  <output>FullTrackingValidation_withQualityIndicator.root</output>
  <description>This script validates the full track finding chain including track quality indicators.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
from tracking import add_tracking_reconstruction
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'FullTrackingValidation_withQualityIndicator.root'
N_EVENTS = 1000
ACTIVE = True


class Full(TrackingValidationRun):
    """Validate the full track-finding chain"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSimNoBkg.root'
    #: use the complete track-reconstruction chain with track quality indicators

    @staticmethod
    def finder_module(path):
        add_tracking_reconstruction(path, add_cdcTrack_QI=True, add_vxdTrack_QI=True, add_recoTrack_QI=True)
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
    #: draw validation plots of track quality indicator
    plotTrackQualityIndicator = True


def main():
    basf2.set_random_seed(1337)
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
