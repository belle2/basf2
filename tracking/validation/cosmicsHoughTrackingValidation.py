#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <input>CosmicsSimNoBkg.root</input>
  <description>Validates the hough finder working on segments in cosmics events.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CosmicsHoughTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = False

import basf2
basf2.set_random_seed(1337)

import os
import trackfindingcdc.cdcdisplay as cdcdisplay

import logging

from tracking.validation.run import TrackingValidationRun


class CosmicsHough(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    root_input_file = '../CosmicsSimNoBkg.root'
    components = None

    def finder_module(self, path):
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

    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
    }
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CosmicsHough()
    validation_run.configure_and_execute_from_commandline()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
