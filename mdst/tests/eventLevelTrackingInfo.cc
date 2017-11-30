#include <gtest/gtest.h>
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

    //Let's set the total number CDC hits in the afterimage
    eventLevelTrackingInfo.setNCDCHitsNotAssigned(324);
    EXPECT_EQ(eventLevelTrackingInfo.getNCDCHitsNotAssigned(), 324);
  }
} // namespace Belle2
