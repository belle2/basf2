#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2

import sys

from trackfindingcdc.validation.recording import RecordingRun

import logging


def get_logger():
    return logging.getLogger(__name__)


class CDCSegmentPairRecordingRun(RecordingRun):
    n_events = 100
    generator_name = "simple_gun"
    # bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    output_file_name = "CDCSegmentPairTruthRecords.root"
    varsets = ["skimmed_fitless", "truth", ]

    recording_filter_parameter_name = "SegmentPairFilterParameters"

    recording_finder_module = basf2.register_module("TrackFinderCDCAutomatonDev")
    recording_finder_module.param({
        "SegmentPairFilter": "unionrecording",
        "SegmentPairFilterParameters": {
            "root_file_name": output_file_name,
            "varsets": ",".join(varsets),
        },
        "SegmentPairRelationFilter": "none"
    })


def main():
    run = CDCSegmentPairRecordingRun()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
