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
    n_events = 10000
    # n_loops = 0.5
    # bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    @property
    def root_output_file_name(self):
        if self.root_input_file:
            file_name = os.path.split(self.root_input_file)[1]
            return file_name
        else:
            return "CDCSegmentPairTruthRecords.root"

    # Train feasible
    # n_events = 1000
    # n_loops = 0.5
    # skim = ""
    # varsets = ["feasible", "filter(truth)",]

    # Validate feasiblity cut
    # n_events = 10000
    # # n_loops = float("nan")
    # n_loops = 1
    # skim = ""
    # varsets = ["feasible", "filter(simple)", "filter(fitless)", "filter(feasible)", "filter(truth)"]

    # investigate next level
    # n_events = 10000
    # n_loops = 1.0
    # skim = "feasible"
    # varsets = [
    #     "basic",
    #     # "feasible",
    #     "fitless",
    #     "pre_fit",
    #     # "fit",
    #     "truth",
    #     # "old_fit",
    #     # "filter(fitless)",
    #     # "filter(simple)",
    #     # "filter(feasible)",
    #     "filter(truth)",
    #     ]

    # Train final cut
    # n_events = 10000
    # n_loops = 1
    # skim = "feasible"
    # varsets = [
    #     "realistic",
    #     "filter(truth)",
    #     "truth"
    #     ]

    # validate final cut
    n_events = 10000
    n_loops = 1.0
    # n_loops = float("nan")
    skim = "feasible"
    varsets = [
        "filter(fitless)",
        "filter(simple)",
        "filter(feasible)",
        "filter(realistic)",
        "filter(truth)",
    ]

    recording_filter_parameter_name = "SegmentPairFilterParameters"
    recording_finder_module = basf2.register_module("TrackFinderCDCAutomatonDev",
                                                    # SegmentOrientation="outwards",
                                                    SegmentOrientation="none",
                                                    )

    recording_finder_module.param({
        "SegmentPairFilter": "unionrecording",
        "SegmentPairFilterParameters": {
            # "rootFileName": root_output_file_name,
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
