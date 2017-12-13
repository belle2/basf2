#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <iostream>

namespace Belle2 {
  /** Unit tests for the event level tracking information.*/
  class EventLevelTrackingInfoTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters, Getters, and Hassers for the CDC related part. */
  TEST_F(EventLevelTrackingInfoTest, settersNGettersCDC)
  {
    // Test proper initialization for an empty afterimage.
    EventLevelTrackingInfo eventLevelTrackingInfo;
    EXPECT_EQ(eventLevelTrackingInfo.getNHitsNotAssigned(), 0);
    EXPECT_EQ(eventLevelTrackingInfo.getNHitsNotAssignedPostCleaning(), 0);

    //Let's set the total number CDC hits in the afterimage
    eventLevelTrackingInfo.setNHitsNotAssigned(324);
    eventLevelTrackingInfo.setNHitsNotAssignedPostCleaning(257);

    EXPECT_EQ(eventLevelTrackingInfo.getNHitsNotAssigned(), 324);
    EXPECT_EQ(eventLevelTrackingInfo.getNHitsNotAssignedPostCleaning(), 257);

    //Let's set and get information for the various layers:
    eventLevelTrackingInfo.setLayer(9); // SL1
    EXPECT_TRUE(eventLevelTrackingInfo.hasLayer(9));
    EXPECT_FALSE(eventLevelTrackingInfo.hasLayer(10));
    eventLevelTrackingInfo.setLayer(23);
    EXPECT_TRUE(eventLevelTrackingInfo.hasLayer(23));
    EXPECT_FALSE(eventLevelTrackingInfo.hasLayer(55));

    EXPECT_B2FATAL(eventLevelTrackingInfo.setLayer(56));
    EXPECT_B2FATAL(eventLevelTrackingInfo.hasLayer(56));

    //Let's have a look at the SuperLayers ...
    EXPECT_TRUE(eventLevelTrackingInfo.hasSLayer(1));
    EXPECT_FALSE(eventLevelTrackingInfo.hasSLayer(5));

    //Let's set and get the information on unused segments:
    eventLevelTrackingInfo.setNSegments(21);
    EXPECT_EQ(eventLevelTrackingInfo.getNSegments(), 21);
    eventLevelTrackingInfo.setNSegments(300);
    EXPECT_EQ(eventLevelTrackingInfo.getNSegments(), 255);
  }
  /** Test simple Setters and Getters for the VXD related part. */
  TEST_F(EventLevelTrackingInfoTest, settersNGettersVXD)
  {
    EventLevelTrackingInfo eventLevelTrackingInfo;
    EXPECT_EQ(eventLevelTrackingInfo.getNClustersInLayer(3), 0);
    eventLevelTrackingInfo.setNClustersInLayer(3, true, 81);
    EXPECT_EQ(eventLevelTrackingInfo.getNClustersInLayer(3), 81);
    eventLevelTrackingInfo.setNClustersInLayer(2, true, 3456);
    EXPECT_EQ(eventLevelTrackingInfo.getNClustersInLayer(2), 3456);
    EXPECT_EQ(eventLevelTrackingInfo.getNClustersInLayer(3, true), 81);
    EXPECT_EQ(eventLevelTrackingInfo.getNClustersInLayer(3, false), 0);
    EXPECT_B2FATAL(eventLevelTrackingInfo.getNClustersInLayer(56));

    eventLevelTrackingInfo.setNClustersInLayer(6, true, 300);
    EXPECT_EQ(eventLevelTrackingInfo.getNClustersInLayer(6, true), 255);

    eventLevelTrackingInfo.setSVDFirstSampleTime(-128);
    EXPECT_EQ(eventLevelTrackingInfo.getSVDFirstSampleTime(), -128);
    eventLevelTrackingInfo.setSVDFirstSampleTime(127);
    EXPECT_EQ(eventLevelTrackingInfo.getSVDFirstSampleTime(), 127);
  }

} // namespace Belle2
