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

    #: Select a recording task
    task = ""

    #: Change the default orientation of segments
    segment_orientation = ""

    @property
    def root_output_file_name(self):
        if self.task:
            file_name = self.task + "_"
        else:
            file_name = ""

        if self.root_input_file:
            file_name += os.path.split(self.root_input_file)[1]
            return file_name
        else:
            if file_name:
                file_name += ".root"
                return file_name
            else:
                return "CDCSegmentPairTruthRecords.root"

    @property
    def recording_finder_module(self):
        finder_module = basf2.register_module(
            "TrackFinderCDCAutomatonDev",
            SegmentPairFilter="unionrecording",
            SegmentPairRelationFilter="none",
            SegmentPairFilterParameters={
                "rootFileName": self.root_output_file_name,
                "varSets": self.varsets,
                "skim": self.skim,
            },
        )
        if self.segment_orientation:
            finder_module.param(dict(SegmentOrientation=self.segment_orientation))

        return finder_module

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)
        argument_parser.add_argument(
            "--task",
            choices=["train_feasible", "eval_feasible", "train_realistic", "eval_realistic", ],
            default=self.task,
            dest="task",
            help=("Select a perpared recording task")
        )
        return argument_parser

    def setup_task(self, task):
        if task == "train_feasible":
            self.n_events = 10000
            self.n_loops = 0.5
            self.flight_time_estimation = "outwards"
            self.segment_orientation = "outwards"
            self.skim = ""
            self.varsets = [
                "feasible",
                "filter(truth)",
            ]

        elif task == "eval_feasible":
            self.n_events = 10000
            self.n_loops = 0.5
            self.flight_time_estimation = "outwards"
            self.segment_orientation = "outwards"
            self.skim = ""
            self.varsets = [
                "filter(feasible)",
                "filter(truth)"
            ]

        elif task == "train_realistic":
            self.n_events = 10000
            self.n_loops = 1
            self.skim = "feasible"
            self.varsets = [
                "realistic",
                "filter(truth)",
                "truth"
            ]

        elif task == "eval_realistic":
            self.n_events = 10000
            self.n_loops = 1
            # self.n_loops = float("nan")
            self.skim = "feasible"
            self.varsets = [
                "filter(fitless)",
                "filter(simple)",
                "filter(feasible)",
                "filter(realistic)",
                "filter(truth)",
            ]

        else:
            # investigate next level
            self.n_events = 10000
            self.n_loops = 1.0
            self.skim = "feasible"
            self.varsets = [
                "basic",
                # "feasible",
                "fitless",
                "pre_fit",
                # "fit",
                "truth",
                # "old_fit",
                # "filter(fitless)",
                # "filter(simple)",
                # "filter(feasible)",
                "filter(truth)",
            ]

    def configure(self, arguments):
        super().configure(arguments)
        self.setup_task(self.task)


def main():
    """Execute the segment pair recording"""
    run = CDCSegmentPairRecordingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
