#!/usr/bin/env python

from trackfinderoutputcombiner.validation import (add_mc_track_finder, add_printer,
                                                  add_background_hit_finder, add_legendre_track_finder,
                                                  add_local_track_finder, add_old_combiner, add_new_combiner,
                                                  add_validation)
from tracking.run.event_generation import StandardEventGenerationRun

import logging
import sys

import basf2


class FullRun(StandardEventGenerationRun):
    # Tester module for all cdc tracking in on path (plus combiners)

    def create_path(self):
        # Creates the path with all the modules in a row
        main_path = super(FullRun, self).create_path()
        add_mc_track_finder(main_path)
        add_background_hit_finder(main_path)
        add_legendre_track_finder(main_path)
        add_local_track_finder(main_path)
        add_old_combiner(main_path, output_track_cands_store_array_name="ResultTrackCands")
        basf2.print_path(main_path, defaults=True)

        return main_path

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    run = FullRun()
    run.configure_and_execute_from_commandline()
