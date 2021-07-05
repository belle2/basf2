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


from tracking.path_utils import add_cdc_track_finding
from tracking.run.event_generation import ReadOrGenerateEventsRun
from trackfindingcdc.run.training import TrainingRunMixin
from tracking.adjustments import adjust_module


class TrackFilterTrainingRun(TrainingRunMixin, ReadOrGenerateEventsRun):
    """Run to record tracks encountered at the TrackRejecter and retrain its mva method"""

    #: number of events to generate
    n_events = 3000
    #: use the generic event generator
    generator_module = "generic"
    #: overlay background hits from the events in these files
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    #: degree of MC truth-matching
    truth = "truth"

    @property
    def identifier(self):
        """Database identifier of the filter being trained"""
        return "trackfindingcdc_TrackFilter.xml"

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()

        add_cdc_track_finding(path)

        #: Process each event according to the user's desired task (train, eval, explore)
        if self.task == "train":
            filterName = "recording"

        elif self.task == "eval":
            filterName = "eval"
            #: Modify degree of MC truth-matching
            self.truth = "truth_accepted"

        elif self.task == "explore":
            # Change me.
            filterName = "recording"

        adjust_module(path, "TFCDC_SegmentTrackCombiner",
                      trackFilter=filterName,
                      trackFilterParameters={
                          "rootFileName": self.sample_file_name,
                      })

        return path

    def postprocess(self):
        """Post-process the results for MC-truth acceptance"""
        if self.task == "eval":
            self.truth = "truth_accept"
        super().postprocess()


def main():
    run = TrackFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
