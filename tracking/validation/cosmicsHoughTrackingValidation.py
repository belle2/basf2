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
    root_input_file = '../CosmicsSimNoBkg.root'
    components = None

    wire_hit_topology_preparer = basf2.register_module('WireHitTopologyPreparer')

    segment_finder_module = basf2.register_module('SegmentFinderCDCFacetAutomaton',
                                                  SegmentOrientation="downwards")

    axial_hough_module = basf2.register_module('TrackFinderCDCAxialSegmentHough',
                                               WriteRecoTracks=False,
                                               TracksStoreObjName="AxialCDCTracks")

    stereo_combiner_module = basf2.register_module('StereoHitFinderCDCLegendreHistogramming',
                                                   TracksStoreObjNameIsInput=True,
                                                   WriteRecoTracks=False,
                                                   TracksStoreObjName="AxialCDCTracks")

    track_exporter = basf2.register_module('TrackExporter',
                                           inputTracks="AxialCDCTracks")

    finder_module = [
        wire_hit_topology_preparer,
        segment_finder_module,
        axial_hough_module,
        stereo_combiner_module,
        track_exporter
        ]

    del segment_finder_module  # do not let the names show up in the class name space
    del axial_hough_module  # do not let the names show up in the class name space
    del stereo_combiner_module  # do not let the names show up in the class name space
    del track_exporter

    interactive_display = False
    if interactive_display:
        cdc_display_module = cdcdisplay.CDCSVGDisplayModule(os.getcwd(), interactive=True)

        cdc_display_module.draw_gftrackcands = True
        cdc_display_module.draw_gftrackcand_trajectories = True
        finder_module.append(cdc_display_module)
        del cdc_display_module  # do not let the names show up in the class name space

    tracking_coverage = {'UsePXDHits': False, 'UseSVDHits': False,
                         'UseCDCHits': True, 'UseOnlyAxialCDCHits': False}
    fit_geometry = None
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CosmicsHough()
    validation_run.configure_and_execute_from_commandline()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
