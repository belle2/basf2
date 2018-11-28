#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-01-00-04.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-01-00-04:
- The default magnetic field is changed affecting all momentum measurements.
- Track:
  + getQualityIndicator
- ECLCluster:
  + getClusterHadronIntensity
  + getNumberOfHadronDigits
  + hasPulseShapeDiscrimination
  + hasTriggerClusterMatching
  + isTriggerCluster
- KlIds are added
- TRGSummary:
  * getTimTypeBits is replaced by getTimType
"""

from basf2 import create_path, process, set_random_seed, find_file, LogLevel
from mdst import add_mdst_dump

if __name__ == "__main__":
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v01-00-04.root"), logLevel=LogLevel.WARNING)
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 5)
