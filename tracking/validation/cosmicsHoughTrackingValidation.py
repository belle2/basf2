#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>CosmicsSimNoBkg.root</input>
  <description>Validates the hough finder working on segments in cosmics events.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import logging
import trackfindingcdc.cdcdisplay as cdcdisplay
import os
import basf2
VALIDATION_OUTPUT_FILE = 'CosmicsHoughTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = False

basf2.set_random_seed(1337)


class CosmicsHough(TrackingValidationRun):
    """Validate the CDC Hough track-segment finder with cosmic rays"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    #: no background overlay
    root_input_file = '../CosmicsSimNoBkg.root'
    #: use full detector for validation
    components = None

    def finder_module(self, path):
        """Add the CDC Hough track finder and related modules to the basf2 path"""
        path.add_module('TFCDC_WireHitPreparer')
        path.add_module('TFCDC_ClusterPreparer')
        path.add_module('TFCDC_SegmentFinderFacetAutomaton',
                        SegmentOrientation="downwards")
        path.add_module('TFCDC_AxialTrackCreatorSegmentHough',
                        tracks="CDCAxialTrackVector")
        path.add_module('TFCDC_StereoHitFinder',
                        inputTracks="CDCAxialTrackVector")
        path.add_module('TFCDC_TrackExporter',
                        inputTracks="CDCAxialTrackVector")

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
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
    }
    #: Include pulls in the validation output
    pulls = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CosmicsHough()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
