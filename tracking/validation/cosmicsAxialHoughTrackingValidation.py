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
  <input>CosmicsSimNoBkg.root</input>
  <output>CosmicsAxialHoughTrackingValidation.root</output>
  <description>Validates the axial hough finder working on segments in cosmics events.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import logging
import trackfindingcdc.cdcdisplay as cdcdisplay
import os
import basf2
VALIDATION_OUTPUT_FILE = 'CosmicsAxialHoughTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = False


class CosmicsAxialHough(TrackingValidationRun):
    """Validate the CDC axial Hough track finder with cosmic rays"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    #: no background overlay
    root_input_file = '../CosmicsSimNoBkg.root'
    #: use full detector for validation
    components = None

    def finder_module(self, path):
        """Add the CDC track-finding module to the basf2 path"""
        path.add_module('TFCDC_WireHitPreparer')
        path.add_module("TFCDC_ClusterPreparer")
        path.add_module('TFCDC_SegmentFinderFacetAutomaton',
                        SegmentOrientation="downwards")
        path.add_module('TFCDC_AxialTrackCreatorSegmentHough')
        path.add_module('TFCDC_TrackExporter')

        interactive_display = False
        if interactive_display:
            cdc_display_module = cdcdisplay.CDCSVGDisplayModule(os.getcwd(), interactive=True)
            cdc_display_module.draw_recotracks = True
            cdc_display_module.draw_recotrack_seed_trajectories = True
            path.add_module(cdc_display_module)

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': True,
        "UseReassignedHits": True,
    }

    #: Include pulls in the validation output
    pulls = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    basf2.set_random_seed(1337)
    validation_run = CosmicsAxialHough()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
