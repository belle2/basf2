#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-03-01-02.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-03-01-02:

- EventMetaData:
  + isEndOfRun
- EventLevelTrackingInfo:
  + hasSVDCKFAbortionFlag/setSVDCKFAbortionFlag
  + hasPXDCKFAbortionFlag/setPXDCKFAbortionFlag
- ECLCluster:
  + hasFailedFitTime
  + hasFailedTimeResolution
- TrackFitResult:
  + ndf
  + chi2
- TRGSummary:
  + getTimQuality added
  + isPoissonInInjectionVeto()
  * added getters to map bit names to numbers (getInputBitNumber, getOutputBitNumber)
    The mapping is from the database and doesn't affect mdst content and is thus not tested here.
  + getInputBitNumber added
  + getOutputBitNumber added
- Particle masses:
  * The particle masses have been updated to PDG 2020. As a result, the
    energies corresponding to the measured momenta became slightly different.
"""

from basf2 import create_path, process, set_random_seed, find_file
from b2test_utils import configure_logging_for_tests
from mdst import add_mdst_dump

if __name__ == "__main__":
    configure_logging_for_tests()
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v03-01-02.root"))
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 3)
