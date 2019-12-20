#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-03-01-02.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-04-00-03:

"""

from basf2 import create_path, process, set_random_seed, find_file, LogLevel, logging
from b2test_utils import configure_logging_for_tests
from mdst import add_mdst_dump

if __name__ == "__main__":
    configure_logging_for_tests()
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v04-00-03.root"))
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 3)
