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
    EXPECT_EQ(eventLevelTrackingInfo.getNCDCHitsNotAssigned(), 0);
    EXPECT_EQ(eventLevelTrackingInfo.getNCDCHitsNotAssignedPostCleaning(), 0);

    //Let's set the total number CDC hits in the afterimage
    eventLevelTrackingInfo.setNCDCHitsNotAssigned(324);
    eventLevelTrackingInfo.setNCDCHitsNotAssignedPostCleaning(257);

    EXPECT_EQ(eventLevelTrackingInfo.getNCDCHitsNotAssigned(), 324);
    EXPECT_EQ(eventLevelTrackingInfo.getNCDCHitsNotAssignedPostCleaning(), 257);

    //Let's set and get information for the various layers:
    eventLevelTrackingInfo.setCDCLayer(9); // SL1
    EXPECT_TRUE(eventLevelTrackingInfo.hasCDCLayer(9));
    EXPECT_FALSE(eventLevelTrackingInfo.hasCDCLayer(10));
    eventLevelTrackingInfo.setCDCLayer(23);
    EXPECT_TRUE(eventLevelTrackingInfo.hasCDCLayer(23));
    EXPECT_FALSE(eventLevelTrackingInfo.hasCDCLayer(55));

    EXPECT_B2FATAL(eventLevelTrackingInfo.setCDCLayer(56));
    EXPECT_B2FATAL(eventLevelTrackingInfo.hasCDCLayer(56));

    //Let's have a look at the SuperLayers ...
    EXPECT_TRUE(eventLevelTrackingInfo.hasCDCSLayer(1));
    EXPECT_FALSE(eventLevelTrackingInfo.hasCDCSLayer(5));

    //Let's set and get the information on unused segments:
    eventLevelTrackingInfo.setNCDCSegments(21);
    EXPECT_EQ(eventLevelTrackingInfo.getNCDCSegments(), 21);
    eventLevelTrackingInfo.setNCDCSegments(300);
    EXPECT_EQ(eventLevelTrackingInfo.getNCDCSegments(), 255);
  }

  /** Test simple Setters and Getters for the VXD related part. */
  TEST_F(EventLevelTrackingInfoTest, settersNGettersVXD)
  {
    EventLevelTrackingInfo eventLevelTrackingInfo;
    EXPECT_EQ(eventLevelTrackingInfo.getNVXDClustersInLayer(3), 0);
    eventLevelTrackingInfo.setNVXDClustersInLayer(3, true, 81);
    EXPECT_EQ(eventLevelTrackingInfo.getNVXDClustersInLayer(3), 81);
    eventLevelTrackingInfo.setNVXDClustersInLayer(2, true, 3456);
    EXPECT_EQ(eventLevelTrackingInfo.getNVXDClustersInLayer(2), 3456);
    EXPECT_EQ(eventLevelTrackingInfo.getNVXDClustersInLayer(3, true), 81);
    EXPECT_EQ(eventLevelTrackingInfo.getNVXDClustersInLayer(3, false), 0);
    EXPECT_B2FATAL(eventLevelTrackingInfo.getNVXDClustersInLayer(56));

    eventLevelTrackingInfo.setNVXDClustersInLayer(6, true, 300);
    EXPECT_EQ(eventLevelTrackingInfo.getNVXDClustersInLayer(6, true), 255);

    eventLevelTrackingInfo.setSVDFirstSampleTime(-128);
    EXPECT_EQ(eventLevelTrackingInfo.getSVDFirstSampleTime(), -128);
    eventLevelTrackingInfo.setSVDFirstSampleTime(127);
    EXPECT_EQ(eventLevelTrackingInfo.getSVDFirstSampleTime(), 127);
  }

  /** Test simple Setters and Getters for the VXD related part. */
  TEST_F(EventLevelTrackingInfoTest, settersNGettersFlagBlock)
  {
    EventLevelTrackingInfo eventLevelTrackingInfo;
    EXPECT_EQ(eventLevelTrackingInfo.getHintForTrackFindingFailure(), false);
    eventLevelTrackingInfo.setHintForTrackFindingFailure();
    EXPECT_EQ(eventLevelTrackingInfo.getHintForTrackFindingFailure(), true);
  }
} // namespace Belle2
