#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-03-01-02.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-03-01-02:
"""

from basf2 import create_path, process, set_random_seed, find_file, LogLevel
from mdst import add_mdst_dump

if __name__ == "__main__":
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v03-01-02.root"), logLevel=LogLevel.WARNING)
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 3)
