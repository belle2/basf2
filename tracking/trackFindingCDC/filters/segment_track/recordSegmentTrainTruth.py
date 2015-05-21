#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from trackfinderoutputcombiner.validation import (add_mc_track_finder, add_legendre_track_finder,
                                                  add_local_track_finder, add_new_combiner,
                                                  add_background_hit_finder)
from tracking.run.event_generation import StandardEventGenerationRun
import logging
import sys


class SegmentTrackTrainRecordingRun(StandardEventGenerationRun):
    n_events = 50
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(SegmentTrackTruthRecordingRun, self).create_path()

        add_mc_track_finder(main_path)
        add_background_hit_finder(main_path)
        add_legendre_track_finder(main_path)
        add_local_track_finder(main_path)
        add_new_combiner(main_path, segment_track_chooser_filter="tmva",
                         segment_track_chooser_cut=0.3, segment_train_filter="recording")

        return main_path


def main():
    run = SegmentTrackTrainRecordingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
