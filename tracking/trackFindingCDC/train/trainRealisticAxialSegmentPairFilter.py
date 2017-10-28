#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class RealisticAxialSegmentPairFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record segment pairs encountered at the AxialSegmentPairCreator and retrain its mva method"""
    n_events = 10000
    generator_module = "generic"
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    truth = "truth_positive"

    @property
    def identifier(self):
        """Database identifier of the filter being trained"""
        return "trackfindingcdc_RealisticAxialSegmentPairFilter.xml"

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

        if self.task == "train":
            varSets = [
                "realistic",
                "filter(truth)",
            ]
            skim = "feasible"

        elif self.task == "eval":
            varSets = [
                "filter(truth)",
                "filter(realistic)",
                "filter(feasible)",
                "filter(simple)",
            ]
            skim = ""

        elif self.task == "explore":
            varSets = [
                "basic",
                "fitless",
                "hit_gap",
                "fit",
                # "filter(simple)",
                # "filter(feasible)",
                # "filter(realistic)",
                "filter(truth)",
            ]
            skim = "feasible"

        else:
            raise ValueError("Unknown task " + self.task)

        path.add_module("TFCDC_TrackFinderSegmentPairAutomaton",
                        SegmentPairAxialBridging=True,
                        AxialSegmentPairFilter="unionrecording",
                        AxialSegmentPairFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                            "skim": skim,
                        },
                        SegmentPairFilter='none',
                        SegmentPairRelationFilter='none')

        return path


def main():
    run = RealisticAxialSegmentPairFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
