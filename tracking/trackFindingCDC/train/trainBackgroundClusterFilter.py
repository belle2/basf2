#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2

from tracking.run.event_generation import ReadOrGenerateEventsRun
from tracking.run.mixins import PostProcessingRunMixin

import subprocess


class BackgroundClusterFilterTrainingRun(PostProcessingRunMixin, ReadOrGenerateEventsRun):
    """Run to record clusters encountered at the ClusterBackgroundDetector and retrain its mva method"""
    n_events = 1000
    generator_module = "generic"
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()
        path.add_module("WireHitPreparer",
                        flightTimeEstimation="outwards")

        path.add_module("SegmentFinderCDCFacetAutomaton",
                        ClusterFilter="unionrecording",
                        ClusterFilterParameters={
                            "rootFileName": "BackgroundClusterFilter.root",
                            "varSets": ["basic", "bkg_truth", ]
                        },
                        FacetFilter="none",
                        FacetRelationFilter="none")
        return path

    def postprocess(self):
        """Run the training as post-processing job

        To run only the training run with --postprocess-only
        """
        super().postprocess()
        cmd = [
            "trackfindingcdc_teacher",
            "--identifier", "trackfindingcdc_BackgroundClusterFilter",
            "BackgroundClusterFilter.root",
        ]
        subprocess.call(cmd)


def main():
    run = BackgroundClusterFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
