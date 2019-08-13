#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class RealisticSegmentRelationFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record segment relations encountered at the SegmentLinker after the feasible filter and retrain its mva method"""

    #: number of events to generate
    n_events = 10000
    #: use the generic event generator
    generator_module = "generic"
    #: overlay background hits from the events in these files
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    #: degree of MC truth-matching
    truth = "truth_positive"

    @property
    def identifier(self):
        """Database identifier of the filter being trained"""
        return "trackfindingcdc_RealisticSegmentRelationFilter.xml"

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()
        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards")

        path.add_module("TFCDC_ClusterPreparer",
                        SuperClusterDegree=3,
                        SuperClusterExpandOverApogeeGap=True)

        #: Process each event according to the user's desired task (train, eval, explore)
        if self.task == "train":
            varSets = [
                "realistic",
                "filter(truth)",
            ]
            skim = "feasible"

        elif self.task == "eval":
            varSets = [
                "filter(feasible)",
                "filter(realistic)",
                "filter(truth)",
            ]
            skim = ""

        elif self.task == "explore":
            varSets = [
                "realistic",
                "hit_gap",
                "fit",
                "filter(truth)",
            ]
            skim = "feasible"

        else:
            raise ValueError("Unknown task " + self.task)

        path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                        SegmentRelationFilter="unionrecording",
                        SegmentRelationFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                            "skim": skim,
                        })

        return path


def main():
    run = RealisticSegmentRelationFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
