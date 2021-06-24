#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test backwards compatibility for an mdst file produced with release-02-01-00.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-02-01-00:
- EventMetaData:
  + isEndOfRun
- EventLevelTrackingInfo:
  + hasSVDCKFAbortionFlag/setSVDCKFAbortionFlag
  + hasPXDCKFAbortionFlag/setPXDCKFAbortionFlag
  + hasSVDSpacePointCreatorAbortionFlag/setSVDSpacePointCreatorAbortionFlag
- ECLCluster:
  + getPulseShapeDiscriminationMVA added
  + hasFailedFitTime
  + hasFailedTimeResolution
  - getCovarianceMatrixAsArray removed
- TRGSummary:
  + getTimQuality added
  + isPoissonInInjectionVeto
  * added getters to map bit names to numbers (getInputBitNumber, getOutputBitNumber)
    The mapping is from the database and doesn't affect mdst content and is thus not tested here.
- SoftwareTriggerResult:
  * non-prescaled results added
- TrackFitResult:
  + ndf
  + chi2
- Particle masses:
  * The particle masses have been updated to PDG 2020. As a result, the
    energies corresponding to the measured momenta became slightly different.
- EventLevelTriggerTimeInfo
  *  added new storage element for information from the Trigger Timing Distribution (TTD)
"""

from basf2 import create_path, process, set_random_seed, find_file
from b2test_utils import configure_logging_for_tests
from mdst import add_mdst_dump

if __name__ == "__main__":
    configure_logging_for_tests()
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v02-01-00.root"))
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 3)
