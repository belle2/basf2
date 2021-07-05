/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbihler (jakob.lettenbichler@oeaw.ac.at)      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/filterTools/ObserverCheckMCPurity.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/ClosedRange.h>
#include <tracking/trackFindingVXD/filterMap/threeHitVariables/CircleRadius.h> // used since it is easy for testing
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>


using namespace std;

using namespace Belle2;

namespace ObserverCheckMCPurityTests {


  /** Test class for SelectionVariableFactory object. */
  class ObserverCheckMCPurityTest : public ::testing::Test {
  protected:
  };


  /** tests initialize and terminate */
  TEST_F(ObserverCheckMCPurityTest, TestInitializeAndTerminate)
  {
    // accessable instance of the observer:
    ObserverCheckMCPurity testObserver = ObserverCheckMCPurity();

/// check the initializer:
    // before initialize: all containers should be empty:
    EXPECT_EQ(0 , ObserverCheckMCPurity::s_results.size());
    EXPECT_EQ(0 , ObserverCheckMCPurity::s_wasAccepted.size());
    EXPECT_EQ(0 , ObserverCheckMCPurity::s_wasUsed.size());

    testObserver.initialize< CircleRadius<SpacePoint>, ClosedRange<double, double>>(CircleRadius<SpacePoint>(),
        ClosedRange<double, double>());

    // after initialize: now the containers are not empty any more:
    EXPECT_EQ(1 , ObserverCheckMCPurity::s_results.size());
    EXPECT_EQ(1 , ObserverCheckMCPurity::s_wasAccepted.size());
    EXPECT_EQ(1 , ObserverCheckMCPurity::s_wasUsed.size());


    testObserver.terminate();

    // after terminate: all containers should be empty:
    EXPECT_EQ(0 , ObserverCheckMCPurity::s_results.size());
    EXPECT_EQ(0 , ObserverCheckMCPurity::s_wasAccepted.size());
    EXPECT_EQ(0 , ObserverCheckMCPurity::s_wasUsed.size());
  }


  // JKL March 11th 2016: the following test is currently broken due to some missing-corner-case-treatments in the PurityCalculator (called by prepare).
// // //   /** tests notify and prepare */
// // //   TEST_F(ObserverCheckMCPurityTest, TestNotifyAndPrepare)
// // //   {
// // // /// prepare some SpacePoints for testing forming a circle-segment of a circle with radius 1:
// // //     auto position1 = B2Vector3D(0., 1., 0.);
// // //     auto position2 = B2Vector3D(1., 0., 0.);
// // //     auto position3 = B2Vector3D(0., -1., 0.);
// // //     auto posError = B2Vector3D(0.1, 0.1, 0.1);
// // //
// // //     auto sp1 = SpacePoint(position1, posError, {0., 0.}, {true, true}, VxdID(), VXD::SensorInfoBase::SensorType::VXD);
// // //     auto sp2 = SpacePoint(position2, posError, {0., 0.}, {true, true}, VxdID(), VXD::SensorInfoBase::SensorType::VXD);
// // //     auto sp3 = SpacePoint(position3, posError, {0., 0.}, {true, true}, VxdID(), VXD::SensorInfoBase::SensorType::VXD);
// // //
// // //
// // // /// prepare Filters and observer:
// // //   // testFilters used for testing (they use the same Observer!):
// // //     auto testFilterAccept((0.9 < CircleRadius<SpacePoint>() < 1.1).observe(ObserverCheckMCPurity()));
// // //     auto testFilterReject((0.5 < CircleRadius<SpacePoint>() < 0.7).observe(ObserverCheckMCPurity()));
// // //
// // //     // SelectionVariable to be used for the test (needed since the Filter has no getter for that):
// // //     CircleRadius<SpacePoint> cRadius = CircleRadius<SpacePoint>();
// // //
// // //     // accessable instance of the observer:
// // //     ObserverCheckMCPurity testObserver = ObserverCheckMCPurity();
// // //     testObserver.initialize< CircleRadius<SpacePoint>, ClosedRange<double, double>>(CircleRadius<SpacePoint>(),
// // //         ClosedRange<double, double>(), nullptr);
// // //
// // //  B2INFO("A")
// // //
// // // /// test notify of observer and if everything is stored as expected:
// // //     // before being notified- observer was not used:
// // //     EXPECT_EQ(false , *(ObserverCheckMCPurity::s_wasUsed.at(cRadius.name())));
// // //
// // //  B2INFO("B")
// // //
// // //     bool wasAccepted = testFilterAccept.accept(sp1, sp2, sp3);
// // //     EXPECT_EQ(true , wasAccepted);
// // //
// // //  B2INFO("C")
// // //
// // //     EXPECT_EQ(true , *(ObserverCheckMCPurity::s_wasUsed.at(cRadius.name())));
// // //     EXPECT_EQ(true , *(ObserverCheckMCPurity::s_wasAccepted.at(cRadius.name())));
// // //     EXPECT_DOUBLE_EQ(1. , *(ObserverCheckMCPurity::s_results.at(cRadius.name())));
// // //
// // //  B2INFO("D")
// // //
// // //     testObserver.prepare(sp1, sp2);
// // //     EXPECT_EQ(false , *(ObserverCheckMCPurity::s_wasUsed.at(cRadius.name())));
// // //  B2INFO("D2")
// // //     wasAccepted = testFilterReject.accept(sp1, sp2, sp3);
// // //     EXPECT_EQ(false , wasAccepted);
// // //
// // //  B2INFO("E")
// // //
// // //     EXPECT_EQ(true , *(ObserverCheckMCPurity::s_wasUsed.at(cRadius.name())));
// // //  B2INFO("F")
// // //     EXPECT_EQ(false , *(ObserverCheckMCPurity::s_wasAccepted.at(cRadius.name())));
// // //  B2INFO("G")
// // //     EXPECT_DOUBLE_EQ(1. , *(ObserverCheckMCPurity::s_results.at(cRadius.name())));
// // //  B2INFO("H")
// // //
// // //     testObserver.terminate();
// // //   }
}
