#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2
from trackfinderoutputcombiner.validation import LegendreTrackFinderRun
import logging


def get_logger():
    return logging.getLogger(__name__)


class SegmentTrackTruthRecordingRun(LegendreTrackFinderRun):
    n_events = 50
    bkg_files = os.path.join(os.environ["VO_BELLE2_SW_DIR"], "bkg")
    segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
    segment_finder_module.param({
        "TracksStoreObjName": "TempCDCTracks",
        "WriteClusters": True,
        "WriteGFTrackCands": False,
        'SkipHitsPreparation': True,
    })
    combiner_module = basf2.register_module("SegmentTrackCombinerDev")
    combiner_module.param({'SegmentTrackChooser': 'tmva',
                           'SegmentTrackChooserParameters': {"cut": "0.05"},
                           'SegmentTrainFilter': 'recording',
                           'WriteGFTrackCands': False,
                           'SkipHitsPreparation': True,
                           'TracksStoreObjNameIsInput': True})

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(SegmentTrackTruthRecordingRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        combiner_module = self.get_basf2_module(self.combiner_module)
        main_path.add_module(combiner_module)

        return main_path


def main():
    run = SegmentTrackTruthRecordingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
