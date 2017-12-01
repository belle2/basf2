#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <iostream>

namespace Belle2 {
  /** Unit tests for the event level tracking information.*/
  class EventLevelTrackingInfoTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters, Getters, and Hassers. */
  TEST_F(EventLevelTrackingInfoTest, settersNGetters)
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

    // ... and stereo- vs. axial-layers
  }
} // namespace Belle2
