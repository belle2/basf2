#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-01-00-04.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-01-00-04:
- EventMetaData:
  + isEndOfRun
- The default magnetic field is changed affecting all momentum measurements.
- Track:
  + getQualityIndicator
- ECLCluster:
  + getPulseShapeDiscriminationMVA
  + getClusterHadronIntensity
  + getNumberOfHadronDigits
  + hasPulseShapeDiscrimination
  + hasTriggerClusterMatching
  + isTriggerCluster
  * getCovarianceMatrixAsArray removed
- KlIds are added
- TRGSummary:
  * getTimTypeBits is replaced by getTimType
- SoftwareTriggerResult:
  * non-prescaled results added
"""

from basf2 import create_path, process, set_random_seed, find_file, LogLevel, logging
from mdst import add_mdst_dump

if __name__ == "__main__":
    logging.package("framework").log_level = LogLevel.WARNING
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v01-00-04.root"), logLevel=LogLevel.WARNING)
    main.add_module("EventInfoPrinter", logLevel=LogLevel.INFO)
    add_mdst_dump(main, True)
    process(main, 5)
