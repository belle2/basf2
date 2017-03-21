#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class BackgroundClusterFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record clusters encountered at the ClusterBackgroundDetector and retrain its mva method"""
    n_events = 1000
    generator_module = "generic"
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()
        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards")

        if self.task == "train":
            varSets = [
                "basic",
                "bkg_truth",
            ]

        elif self.task == "eval":
            varSets = [
                "filter(mva_bkg)",
                "bkg_truth",
            ]

        elif self.task == "explore":
            # Change me.
            varSets = [
                "basic",
                "bkg_truth",
                "filter(mva_bkg)",
            ]

        path.add_module("SegmentFinderCDCFacetAutomaton",
                        ClusterFilter="unionrecording",
                        ClusterFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                        },
                        FacetFilter="none",
                        FacetRelationFilter="none")
        return path


def main():
    run = BackgroundClusterFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
