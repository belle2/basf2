#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test backwards compatibility for an mdst file produced with release-06-00-00.

See https://confluence.desy.de/display/BI/Backward+Compatibility if this test fails.

CHANGES since release-06-00-00:

- EventLevelClusteringInfo
  + nKLMDigitsMultiStrip
  + nECLShowers
  + nECLLocalMaximums
  + nECLTriggerCells
- Track:
  + statusBitmap and getStatusBit
  + getTrackTime added
  + wasRefined
  + isFlippedAndRefitted
  + getTrackFitResultByName
  + getTrackFitResultsByName
  + getTrackFitResultWithClosestMassByName
- TrackFitResult:
  + remove the const classifier of TrackFitResults and add a function to update the contents
  + updateTrackFitResult
  + mask
"""

from basf2 import create_path, process, set_random_seed, find_file
from b2test_utils import configure_logging_for_tests
from mdst import add_mdst_dump

if __name__ == "__main__":
    configure_logging_for_tests()
    set_random_seed(1)
    main = create_path()
    main.add_module("RootInput", inputFileName=find_file("mdst/tests/mdst-v06-00-00.root"))
    main.add_module("EventInfoPrinter")
    add_mdst_dump(main, True)
    process(main, 3)
