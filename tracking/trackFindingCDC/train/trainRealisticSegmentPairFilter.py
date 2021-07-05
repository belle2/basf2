#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class RealisticSegmentPairFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record segment pairs encountered at the SegmentPairCreator and retrain its mva method"""
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
        return "trackfindingcdc_RealisticSegmentPairFilter.xml"

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()

        # In contrast to other training use only the first *half* loop for more aggressive training
        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards")

        path.add_module('TFCDC_ClusterPreparer',
                        SuperClusterDegree=3,
                        SuperClusterExpandOverApogeeGap=True)

        path.add_module("TFCDC_SegmentFinderFacetAutomaton")

        #: Process each event according to the user's desired task (train, eval, explore)
        if self.task == "train":
            varSets = [
                "realistic",
                "filter(truth)",
                "truth",  # for weighting
            ]
            skim = "feasible"

        elif self.task == "eval":
            varSets = [
                "filter(truth)",
                "filter(realistic)",
                "filter(feasible)",
                "filter(fitless)",
                "filter(simple)",
            ]
            skim = ""

        elif self.task == "explore":
            varSets = [
                "basic",
                # "realistic",
                "fitless",
                "pre_fit",
                "fit",
                "truth",  # for weighting
                # "old_fit",
                # "filter(fitless)",
                # "filter(simple)",
                "filter(realistic)",
                "filter(truth)",
            ]
            skim = "feasible"

        else:
            raise ValueError("Unknown task " + self.task)

        path.add_module("TFCDC_TrackFinderSegmentPairAutomaton",
                        SegmentPairFilter="unionrecording",
                        SegmentPairFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                            "skim": skim,
                        })

        return path


def main():
    run = RealisticSegmentPairFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
