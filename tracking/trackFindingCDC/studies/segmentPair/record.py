#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2

from trackfindingcdc.run.recording import RecordingRun

import logging


class CDCSegmentPairRecordingRun(RecordingRun):
    """Run for recording segment pairs encountered at the filter"""
    n_events = 100
    generator_name = "simple_gun"
    # bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    root_output_file_name = "CDCSegmentPairTruthRecords.root"
    varsets = ["skimmed_fitless", "filter(truth)", ]

    recording_filter_parameter_name = "SegmentPairFilterParameters"

    recording_finder_module = basf2.register_module("TrackFinderCDCAutomatonDev")
    recording_finder_module.param({
        "SegmentPairFilter": "unionrecording",
        "SegmentPairFilterParameters": {
            "rootFileName": root_output_file_name,
            "varSets": varsets,
        },
        "SegmentPairRelationFilter": "none"
    })


def main():
    """Execute the segment pair recording"""
    run = CDCSegmentPairRecordingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
