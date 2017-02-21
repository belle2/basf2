#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import shutil

import basf2

from tracking import add_cdc_track_finding
from tracking.run.event_generation import ReadOrGenerateEventsRun
from tracking.run.mixins import PostProcessingRunMixin
from tracking.adjustments import adjust_module

import subprocess


class TrackFilterTrainingRun(PostProcessingRunMixin, ReadOrGenerateEventsRun):
    """Run to record tracks encountered at the TrackRejecter and retrain its mva method"""
    n_events = 3000
    generator_module = "generic"
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()

        add_cdc_track_finding(path)
        adjust_module(path, "SegmentTrackCombiner",
                      trackFilter="recording",
                      trackFilterParameters={"rootFileName": "TrackFilter.root"})
        return path

    def postprocess(self):
        """Run the training as post-processing job

        To run only the training run with --postprocess-only
        """
        super().postprocess()
        cmd = [
            "trackfindingcdc_teacher",
            "--identifier", "trackfindingcdc_TrackFilter.xml",
            "TrackFilter.root",
        ]
        subprocess.call(cmd)

        # Move training file to the right location
        tracking_data_dir_path = os.path.join(os.environ["BELLE2_LOCAL_DIR"], "tracking", "data")
        shutil.copy("trackfindingcdc_TrackFilter.xml", tracking_data_dir_path)


def main():
    run = TrackFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
