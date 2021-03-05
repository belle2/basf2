#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-04-00-03.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-04-00-03:

- EventLevelTrackingInfo:
  + hasSVDCKFAbortionFlag/setSVDCKFAbortionFlag
  + hasPXDCKFAbortionFlag/setPXDCKFAbortionFlag
- Global tags:
  * The compatibility tag "Legacy_IP_Information" is no longer added for
    release 4 input files.
- TRGSummary:
  + getTimQuality added
  + isPoissonInInjectionVeto()
  * added getters to map bit names to numbers (getInputBitNumber, getOutputBitNumber)
    The mapping is from the database and doesn't affect mdst content and is thus not tested here.
- Particle masses:
  * The particle masses have been updated to PDG 2020. As a result, the
    energies corresponding to the measured momenta became slightly different.
- TrackFitResult:
  + ndf
  + chi2
"""

from basf2 import create_path, process, set_random_seed, find_file
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
