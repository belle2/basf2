/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/utilities/TestHelpers.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  This class provides the declaration of the common test fixture to all
     *  test of the track finding in the CDC code.
     *
     *  Reminder: You can filter for specific test by running
     *
     *  $ test_tracking --gtest_filter="TrackFindingCDCTest*"
     *
     *  which runs only for track finding in the CDC (positiv match).
     *
     *  Run
     *
     *  $ test_tracking --gtest_filter="-TrackFindingCDCTest*"
     *
     *  to exclude test with this fixture. Also consider
     *  test_tracking --help for more details
     *
     *  Alternativelly it is now possible to run
     *
     *  $ test_tracking_trackFindingCDC
     *
     *  as a seperate executable.
     */
    class TrackFindingCDCTestWithTopology : public TestHelpers::TestWithGearbox {

    public:
      /**
       *  Implementation of hook method to setup the environment for all tests in this test case
       *  Unpacks the wire topology from the CDCGeometryPar.
       */
      static void SetUpTestCase();

      /**
       *  Implementation of hook method to clean up the environment for all tests in this test case.
       */
      static void TearDownTestCase();

    };

    /// Equivalent to \sa TrackFindingCDCTestWithTopology for disabled tests.
    class DISABLED_Long_TrackFindingCDCTestWithTopology : public TrackFindingCDCTestWithTopology {};
  }


  namespace TestHelpers {
    /** Predicate checking that all three components of Vector3D are close by a maximal error of tolerance. */
    template<>
    bool allNear<TrackFindingCDC::Vector3D>(const TrackFindingCDC::Vector3D& expected,
                                            const TrackFindingCDC::Vector3D& actual,
                                            double tolerance);
  }

}

