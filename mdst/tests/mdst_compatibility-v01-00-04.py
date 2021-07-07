#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test backwards compatibility for an mdst file produced with release-01-00-04.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-01-00-04:
- EventMetaData:
  + isEndOfRun
- EventLevelTrackingInfo:
  + hasSVDCKFAbortionFlag/setSVDCKFAbortionFlag
  + hasPXDCKFAbortionFlag/setPXDCKFAbortionFlag
  + hasSVDSpacePointCreatorAbortionFlag/setSVDSpacePointCreatorAbortionFlag
- The default magnetic field is changed affecting all momentum measurements.
- Track:
  + getQualityIndicator
- TrackFitResult:
  + ndf
  + chi2
- TRGSummary:
  + getTimQuality added
  + isPoissonInInjectionVeto()
  * added getters to map bit names to numbers (getInputBitNumber, getOutputBitNumber)
    The mapping is from the database and doesn't affect mdst content and is thus not tested here.
- ECLCluster:
  + getMinTrkDistanceID
  + getPulseShapeDiscriminationMVA
  + getClusterHadronIntensity
  + getNumberOfHadronDigits
  + hasPulseShapeDiscrimination
  + hasTriggerClusterMatching
  + isTriggerCluster
  + hasFailedFitTime
  + hasFailedTimeResolution
  * getCovarianceMatrixAsArray removed
- KlIds are added
- TRGSummary:
  * getTimTypeBits is replaced by getTimType
  * getTimQuality added
- SoftwareTriggerResult:
  * non-prescaled results added
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
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v01-00-04.root"))
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 5)
