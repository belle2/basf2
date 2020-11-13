#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for a udst file produced with release-04-01-04.

This is not always guaranteed. Analysis objects may break backwards
compatibility for older files. Some niche corner cases might be problematic for
file produced with release older than release-05-00-00.

See https://confluence.desy.de/display/BI/Backward+Compatibility for more
"""

import basf2
import b2test_utils
import udst

if __name__ == "__main__":

    # clear out stray env variables and set seeding for reproducible results
    b2test_utils.configure_logging_for_tests()
    basf2.set_random_seed(1)

    # configure processing path - input file is the first event from the Phil's
    # favourite test udst file from https://questions.belle2.org/question/9758/
    main = basf2.create_path()
    main.add_module(
        "RootInput", inputFileName=basf2.find_file("analysis/tests/udst-v04-02-04.root")
    )
    main.add_module("EventInfoPrinter")

    # this function does the hard work
    udst.add_udst_dump(main, True)

    basf2.process(main, 1)
