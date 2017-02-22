#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class RealisticSegmentPairRelationFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record segment pair relations encountered at the SegmentPairRelationCreator and retrain its mva method"""
    n_events = 10000
    generator_module = "generic"
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    truth = "truth_positive"

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()
        path.add_module("WireHitPreparer",
                        flightTimeEstimation="outwards",
                        UseNLoops=1.0)

        if self.task == "train":
            varSets = [
                "realistic",
                "filter(truth)",
                "truth",
            ]

        elif self.task == "eval":
            varSets = [
                "filter(simple)",
                "filter(realistic)",
                "filter(truth)",
            ]

        elif self.task == "explore":
            varSets = [
                "basic",
                "fit",
                "filter(simple)",
                "filter(truth)",
            ]

        else:
            raise ValueError("Unknown task " + self.task)

        path.add_module("TrackFinderCDCAutomaton",
                        SegmentPairRelationFilter="unionrecording",
                        SegmentPairRelationFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                        })

        return path


def main():
    run = RealisticSegmentPairRelationFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
