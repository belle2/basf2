#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys
import os.path

from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin


class BackgroundClusterFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record clusters encountered at the ClusterBackgroundDetector and retrain its mva method"""

    #: number of events to generate
    n_events = 1000
    #: use the generic event generator
    generator_module = "generic"
    #: overlay background hits from the events in these files
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()
        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards")

        #: Process each event according to the user's desired task (train, eval, explore)
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

        path.add_module("TFCDC_ClusterPreparer",
                        ClusterFilter="unionrecording",
                        ClusterFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": varSets,
                        })

        return path


def main():
    run = BackgroundClusterFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
