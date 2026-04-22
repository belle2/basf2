/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

namespace Belle2 {

  /** Unit tests for the event level trigger time information.*/
  class EventLevelTriggerTimeInfoTest : public ::testing::Test {
  protected:
  };

  /** Test setters and getters for the EventT0Source. */
  TEST_F(EventLevelTriggerTimeInfoTest, EventT0Source)
  {
    EventLevelTriggerTimeInfo tti;
    EXPECT_EQ(short(tti.getEventT0Sources()), 0);
    EXPECT_FALSE(tti.hasAnyEventT0Sources());
    EXPECT_FALSE(tti.hasEventT0SourceFromSVD());
    EXPECT_FALSE(tti.hasEventT0SourceFromCDC());
    EXPECT_FALSE(tti.hasEventT0SourceFromECL());

    tti.addEventT0SourceFromSVD(); // +1
    EXPECT_EQ(short(tti.getEventT0Sources()), 1);
    EXPECT_TRUE(tti.hasAnyEventT0Sources());
    EXPECT_TRUE(tti.hasEventT0SourceFromSVD());
    EXPECT_FALSE(tti.hasEventT0SourceFromCDC());
    EXPECT_FALSE(tti.hasEventT0SourceFromECL());

    tti.addEventT0SourceFromECL(); // +4
    EXPECT_EQ(short(tti.getEventT0Sources()), 5);
    EXPECT_TRUE(tti.hasAnyEventT0Sources());
    EXPECT_TRUE(tti.hasEventT0SourceFromSVD());
    EXPECT_FALSE(tti.hasEventT0SourceFromCDC());
    EXPECT_TRUE(tti.hasEventT0SourceFromECL());

    tti.unsetEventT0SourceFromSVD(); // -1
    EXPECT_EQ(short(tti.getEventT0Sources()), 4);
    EXPECT_TRUE(tti.hasAnyEventT0Sources());
    EXPECT_FALSE(tti.hasEventT0SourceFromSVD());
    EXPECT_FALSE(tti.hasEventT0SourceFromCDC());
    EXPECT_TRUE(tti.hasEventT0SourceFromECL());

    tti.unsetEventT0SourceFromCDC(); // no effect
    EXPECT_EQ(short(tti.getEventT0Sources()), 4);
    EXPECT_TRUE(tti.hasAnyEventT0Sources());
    EXPECT_FALSE(tti.hasEventT0SourceFromSVD());
    EXPECT_FALSE(tti.hasEventT0SourceFromCDC());
    EXPECT_TRUE(tti.hasEventT0SourceFromECL());

    tti.resetEventT0Sources(); // same as initialized
    EXPECT_EQ(short(tti.getEventT0Sources()), 0);
    EXPECT_FALSE(tti.hasAnyEventT0Sources());
    EXPECT_FALSE(tti.hasEventT0SourceFromSVD());
    EXPECT_FALSE(tti.hasEventT0SourceFromCDC());
    EXPECT_FALSE(tti.hasEventT0SourceFromECL());

    tti.addEventT0SourceFromSVD(); // +1
    tti.addEventT0SourceFromCDC(); // +2
    tti.addEventT0SourceFromECL(); // +4
    EXPECT_EQ(short(tti.getEventT0Sources()), 7);
    EXPECT_TRUE(tti.hasAnyEventT0Sources());
    EXPECT_TRUE(tti.hasEventT0SourceFromSVD());
    EXPECT_TRUE(tti.hasEventT0SourceFromCDC());
    EXPECT_TRUE(tti.hasEventT0SourceFromECL());

    tti.unsetEventT0SourceFromECL(); // -4
    EXPECT_EQ(short(tti.getEventT0Sources()), 3);
    EXPECT_TRUE(tti.hasAnyEventT0Sources());
    EXPECT_TRUE(tti.hasEventT0SourceFromSVD());
    EXPECT_TRUE(tti.hasEventT0SourceFromCDC());
    EXPECT_FALSE(tti.hasEventT0SourceFromECL());
  }
} // namespace Belle2
